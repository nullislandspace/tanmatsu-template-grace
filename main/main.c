// Graceloader — Minimal ELF library loader for Tanmatsu apps
// Named after Grace Hopper, pioneer of computer programming
//
// Mounts filesystems, determines app path from boot selection,
// loads app.so via kbelf, and jumps to its entry point.

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "hal/cache_hal.h"
#include "soc/soc.h"
#include "kbelf.h"
#include "appfs.h"
#include "sdcard.h"
#include "fastopen.h"
#include "sdkconfig.h"

static const char TAG[] = "graceloader";

static wl_handle_t wl_handle = WL_INVALID_HANDLE;

void app_main(void) {
    ESP_LOGI(TAG, "Graceloader starting...");

    // 1. Mount internal flash filesystem at /int
    esp_vfs_fat_mount_config_t fat_config = {
        .format_if_mount_failed   = false,
        .max_files                = 10,
        .allocation_unit_size     = CONFIG_WL_SECTOR_SIZE,
        .disk_status_check_enable = false,
        .use_one_fat              = false,
    };

    esp_err_t res = esp_vfs_fat_spiflash_mount_rw_wl("/int", "locfd", &fat_config, &wl_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount /int: %s", esp_err_to_name(res));
    } else {
        ESP_LOGI(TAG, "Internal filesystem mounted at /int");
    }

    // 2. Mount SD card at /sd (non-fatal if absent)
    res = sd_mount();
    if (res != ESP_OK) {
        ESP_LOGW(TAG, "SD card not available: %s", esp_err_to_name(res));
    } else {
        ESP_LOGI(TAG, "SD card mounted at /sd");
    }

    // 3. Determine app path from boot selection (set by launcher)
    bool bootsel_valid        = false;
    char const* bootsel_arg  = NULL;
    appfs_handle_t bootsel_handle = appfsBootselGet(&bootsel_valid, &bootsel_arg);

    char elf_path[256];
    if (bootsel_handle != APPFS_INVALID_FD && bootsel_valid && bootsel_arg != NULL) {
        snprintf(elf_path, sizeof(elf_path), "%s/app.so", bootsel_arg);
        ESP_LOGI(TAG, "Boot selection path: %s", bootsel_arg);
    } else {
        ESP_LOGE(TAG, "No boot selection available - cannot determine app path");
        ESP_LOGE(TAG, "App must be launched from the Tanmatsu launcher");
        return;
    }

    ESP_LOGI(TAG, "Loading app from: %s", elf_path);

    // 4. Load ELF library via kbelf
    kbelf_dyn dyn = kbelf_dyn_create(0);
    if (!dyn) {
        ESP_LOGE(TAG, "Failed to create kbelf dynamic context");
        return;
    }

    if (!kbelf_dyn_set_exec(dyn, elf_path, NULL)) {
        ESP_LOGE(TAG, "Failed to open ELF file: %s", elf_path);
        kbelf_dyn_destroy(dyn);
        return;
    }

    if (!kbelf_dyn_load(dyn)) {
        ESP_LOGE(TAG, "Failed to load ELF: %s", elf_path);
        kbelf_dyn_destroy(dyn);
        return;
    }

    // Per-segment I-cache invalidation is handled by kbelfx_cache_sync()
    // which kbelf calls automatically during kbelf_dyn_load().
    // Additionally, do a full DRAM writeback as a safety measure:
#if SOC_CACHE_WRITEBACK_SUPPORTED
    cache_hal_writeback_addr(SOC_DRAM_LOW, SOC_DRAM_HIGH - SOC_DRAM_LOW);
#endif

    // Run preinit and init arrays
    size_t preinit_count = kbelf_dyn_preinit_len(dyn);
    if (preinit_count > 0) {
        ESP_LOGI(TAG, "Running %zu preinit functions", preinit_count);
        for (size_t i = 0; i < preinit_count; i++) {
            ((void (*)(void))kbelf_dyn_preinit_get(dyn, i))();
        }
    }

    size_t init_count = kbelf_dyn_init_len(dyn);
    if (init_count > 0) {
        ESP_LOGI(TAG, "Running %zu init functions", init_count);
        for (size_t i = 0; i < init_count; i++) {
            ((void (*)(void))kbelf_dyn_init_get(dyn, i))();
        }
    }

    // 5. Jump to app entry point
    void (*entry)(int, char const**, char const**) = (void*)kbelf_dyn_entrypoint(dyn);
    ESP_LOGI(TAG, "Jumping to app @ %p", entry);
    entry(1, (char const*[]){elf_path}, (char const*[]){NULL});

    // 6. Cleanup after app returns
    ESP_LOGI(TAG, "App returned, cleaning up");

    size_t fini_count = kbelf_dyn_fini_len(dyn);
    if (fini_count > 0) {
        ESP_LOGI(TAG, "Running %zu fini functions", fini_count);
        for (size_t i = 0; i < fini_count; i++) {
            ((void (*)(void))kbelf_dyn_fini_get(dyn, i))();
        }
    }

    kbelf_dyn_unload(dyn);
    kbelf_dyn_destroy(dyn);

    ESP_LOGI(TAG, "Graceloader done");
}
