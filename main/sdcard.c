#include "sdcard.h"
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/gpio_types.h"
#include "sd_pwr_ctrl.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#include "sdmmc_cmd.h"

static sd_status_t status = SD_STATUS_NOT_PRESENT;

#if defined(CONFIG_BSP_TARGET_TANMATSU) || defined(CONFIG_BSP_TARGET_KONSOOL)
static sdmmc_card_t*        card          = NULL;
static const char           mount_point[] = "/sd";
static sd_pwr_ctrl_handle_t sd_pwr_handle = NULL;
static char const TAG[] = "sdcard";

static sd_pwr_ctrl_handle_t initialize_sd_ldo(void) {
    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = 4,
    };
    sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;
    esp_err_t            res             = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create a new on-chip LDO power control driver");
        return NULL;
    }
    return pwr_ctrl_handle;
}

static esp_err_t reset_sd_card(void) {
    if (sd_pwr_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGI(TAG, "Power cycling SD card...");

    gpio_config_t gpio_cfg = {
        .pin_bit_mask = BIT64(GPIO_NUM_39) | BIT64(GPIO_NUM_40) | BIT64(GPIO_NUM_41) | BIT64(GPIO_NUM_42) |
                        BIT64(GPIO_NUM_43) | BIT64(GPIO_NUM_44),
        .mode         = GPIO_MODE_OUTPUT_OD,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&gpio_cfg);
    gpio_set_level(GPIO_NUM_39, 0);
    gpio_set_level(GPIO_NUM_40, 0);
    gpio_set_level(GPIO_NUM_41, 0);
    gpio_set_level(GPIO_NUM_42, 0);
    gpio_set_level(GPIO_NUM_43, 0);
    gpio_set_level(GPIO_NUM_44, 0);

    sd_pwr_ctrl_set_io_voltage(sd_pwr_handle, 0);
    vTaskDelay(pdMS_TO_TICKS(150));

    gpio_cfg.mode = GPIO_MODE_INPUT;
    gpio_config(&gpio_cfg);
    sd_pwr_ctrl_set_io_voltage(sd_pwr_handle, 3300);
    vTaskDelay(pdMS_TO_TICKS(150));
    return ESP_OK;
}

esp_err_t sd_mount(void) {
    if (card != NULL) {
        ESP_LOGI(TAG, "SD card already mounted");
        return ESP_OK;
    }

    if (sd_pwr_handle == NULL) {
        ESP_LOGI(TAG, "Acquiring SD LDO power control handle");
        sd_pwr_handle = initialize_sd_ldo();
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files              = 10,
        .allocation_unit_size   = 16 * 1024,
    };

    ESP_LOGI(TAG, "Initializing SD card");
    reset_sd_card();

    sdmmc_host_t host    = SDMMC_HOST_DEFAULT();
    host.slot            = SDMMC_HOST_SLOT_0;
    host.max_freq_khz    = SDMMC_FREQ_HIGHSPEED;
    host.pwr_ctrl_handle = sd_pwr_handle;

    static DRAM_DMA_ALIGNED_ATTR uint8_t dma_buf[512 * 4];
    host.dma_aligned_buffer = dma_buf;

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.clk                 = GPIO_NUM_43;
    slot_config.cmd                 = GPIO_NUM_44;
    slot_config.d0                  = GPIO_NUM_39;
    slot_config.d1                  = GPIO_NUM_40;
    slot_config.d2                  = GPIO_NUM_41;
    slot_config.d3                  = GPIO_NUM_42;
    slot_config.width               = 4;

    ESP_LOGI(TAG, "Mounting filesystem");
    esp_err_t res = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (res != ESP_OK) {
        if (res == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount SD card filesystem.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the SD card (%s).", esp_err_to_name(res));
        }
        status = SD_STATUS_ERROR;
        return res;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    sdmmc_card_print_info(stdout, card);
    status = SD_STATUS_OK;
    return ESP_OK;
}

#else

esp_err_t sd_mount(void) {
    return ESP_ERR_NOT_SUPPORTED;
}

#endif

sd_status_t sd_status(void) {
    return status;
}
