# Plan: Split Template App into Graceloader + ELF Library

## Context

Currently, all Tanmatsu apps compile as monolithic binaries installed to the small `/appfs` partition, severely limiting how many apps can be installed. The goal is to split the template into:

1. **Graceloader** (installed to appfs, kept small) — a minimal loader named after Grace Hopper that mounts filesystems and loads the app
2. **The app as an ELF shared library** (`app.so` on `/sd` or `/int`) containing all app logic AND its dependencies (BSP, WiFi, PAX) compiled with `-fPIC`

The graceloader does NOT include app-specific components (BSP, WiFi, PAX, BT). It provides the core runtime (libc, libm, libgcc, FreeRTOS, basic ESP-IDF) via ROM and its own code — exported to app.so through kbelf at zero cost using weak symbol references.

---

## Current Status

### What works:
- **Graceloader builds and links**: 315KB binary (was 290KB without symbol exports)
- **app.so builds and links**: Uses ESP-IDF build with `-fPIC -D__thread=`, then a separate link step extracts .a files from `build.ninja` and re-links as `.so`
- **Weak symbol exports**: `symbol_export.py` generates `__attribute__((weak))` references so ROM-provided symbols cost zero, missing symbols resolve to NULL
- **Fakelib approach works**: app.so has `NEEDED` entries for `libc.so`, `libm.so`, etc. and libc functions like `memcpy` show as `U` (undefined/dynamic) — correctly resolved from graceloader

### Size problem (current):
- **Graceloader**: 315 KB
- **app.so**: ~2.3 MB on disk, 1.6 MB text
- **Standard template**: 968 KB
- **Total overhead**: ~1.6 MB (167% larger than monolithic)

### Root cause of bloat identified:
The app.so text section (913KB) is actually comparable to the standard template. The bloat is in **PIC metadata**:
- `.dynsym` + `.dynstr` + `.hash`: **291 KB** (dynamic symbol tables)
- `.rela.dyn`: **76 KB** (relocation entries)
- Total PIC overhead: **~367 KB** just for metadata

This is because ALL 7769 symbols from the .a files become dynamic symbols. Only `_start` and imported symbols actually need to be dynamic.

### Next step to try:
Use a **linker version script** to hide all symbols except `_start`:
```
{ global: _start; local: *; };
```
Pass via `-Wl,--version-script=app_version.script` during the `.so` link. This should eliminate most of the 367KB PIC metadata overhead, bringing app.so much closer to the template size.

---

## Architecture

```
┌──────────────────────────────┐     ┌──────────────────────────────┐
│  Graceloader (~315KB, appfs) │     │  App Library (app.so, on SD) │
│                              │     │                              │
│  1. Mount /int filesystem    │     │  Statically includes:        │
│  2. Mount /sd (if present)   │────>│  - badge-bsp (compiled -fPIC)│
│  3. appfsBootselGet()        │     │  - pax-gfx (compiled -fPIC)  │
│     → get app directory      │     │  - tanmatsu-wifi (-fPIC)     │
│  4. Load <path>/app.so       │     │  - esp_lcd, bt, etc (-fPIC)  │
│     via kbelf + fastopen()   │     │                              │
│  5. Jump to _start           │     │  app_main():                 │
│                              │     │  - GPIO ISR, NVS, BSP init   │
│  Exports via kbelf (weak):   │     │  - WiFi, display, input loop │
│  - libc/libm/libgcc (ROM!)  │     │                              │
│  - FreeRTOS                  │     │  Development is identical to │
│  - ESP-IDF core (log, etc)   │     │  current monolithic apps!    │
└──────────────────────────────┘     └──────────────────────────────┘
```

---

## Key Technical Findings

### 1. ESP32-P4 ROM functions
The ESP32-P4 has libc/libm/libgcc functions in silicon ROM at fixed addresses. ESP-IDF's `esp32p4.rom.libc.ld` etc. define symbols like `memcpy = 0x4FC0xxxx`. The standard template uses these for free (~600KB savings). The graceloader also uses them. By exporting ROM symbols via kbelf with weak references, the app.so gets ROM functions at zero cost to the graceloader.

### 2. Weak symbol references in kbelf_lib_*.c
```c
extern char const __attribute__((weak)) symbol_memcpy[] asm("memcpy");
```
- If `memcpy` is in ROM or graceloader code → resolves to its address (zero cost)
- If `memcpy` doesn't exist → resolves to NULL (kbelf skips it, app provides its own)
- No `--undefined` linker flags needed → no code pulled into graceloader

### 3. __thread / TLS incompatibility
pax-gfx has `static __thread pax_err_t last_err = PAX_OK;` (line 13 of pax_gfx.c). The `__thread` keyword generates `R_RISCV_TPREL_HI20` relocations which are incompatible with `-shared` on bare-metal RISC-V. Fix: `-D__thread=` compiler flag strips the keyword (makes it a plain static variable, losing per-task error tracking which is acceptable).

### 4. Non-PIC system libraries
The toolchain's system `libc.a`, `libm.a`, `libgcc.a` (in `~/.espressif/tools/riscv32-esp-elf/`) are NOT compiled with -fPIC. Using `-lc -lm -lgcc` in the .so link pulls these in and causes `R_RISCV_HI20` relocation errors. Solution: do NOT link system libs; use fakelibs for dynamic resolution from graceloader instead.

### 5. Assembly-only components incompatible with -shared
`libriscv.a` (interrupt vectors), `libesp_system.a` (startup code), `libesp_rom.a`, `libbootloader_support.a` contain hand-written assembly with absolute relocations (`R_RISCV_JAL`). These must be excluded from the .so link. The `-Wl,-Bsymbolic` flag resolves most JAL issues for PIC-compiled code.

### 6. ESP-IDF build produces correct .a file list in build.ninja
The `LINK_LIBRARIES` line in `build.ninja` has the exact library order ESP-IDF determined, including `--whole-archive` flags and repeated entries for circular deps. The app.so link script extracts this and replaces firmware link flags with `-shared`.

### 7. app.elf link always fails (expected)
When the ESP-IDF app project is compiled with `-fPIC`, the `app.elf` firmware link fails because PIC code is incompatible with ESP-IDF's firmware linker scripts (`.got.plt` sections, etc.). This is expected — we only need the compiled `.a` files, not the firmware binary.

---

## File Structure (Current State)

```
tanmatsu-template-grace/
├── CMakeLists.txt                  # ESP-IDF project (graceloader)
├── Makefile                        # Updated with app build targets
├── components/
│   └── kbelf/                      # LOCAL: kbelf ELF loader core + platform
│       ├── CMakeLists.txt          # Includes kbelf core + kbelfx + fastopen + symbol tables
│       ├── include/
│       │   ├── kbelf.h
│       │   └── kbelf/*.h           # kbelf sub-headers
│       └── src/
│           ├── kbelf*.c            # Core ELF loader
│           ├── port/riscv.c        # RISC-V port
│           ├── kbelfx.c            # Platform callbacks (fastopen, cache sync)
│           ├── fastopen.c/.h       # DMA-optimized file I/O
│           └── kbelf_lib_*.c       # GENERATED: weak symbol export tables
├── main/                           # GRACELOADER source
│   ├── CMakeLists.txt              # Minimal deps: fatfs, nvs_flash, kbelf, appfs
│   ├── idf_component.yml           # Only appfs dependency
│   ├── main.c                      # Mount FS, load ELF, jump to _start
│   ├── sdcard.c / sdcard.h         # SD card mount (from launcher)
│   ├── symbol_export/              # Per-library symbol lists
│   │   ├── c, m, gcc, pthread      # Standard library symbols
│   │   ├── freertos                # FreeRTOS (actual function names, not macros)
│   │   ├── esp-idf                 # ESP-IDF core (log, err)
│   │   └── pax-gfx, pax-codecs    # PAX graphics symbols
│   ├── symbol_export.py            # Generator (weak refs, from launcher)
│   └── symbol_export.sh            # Regeneration script
├── app/                            # APP (full ESP-IDF project compiled with -fPIC)
│   ├── CMakeLists.txt              # ESP-IDF project + -fPIC + -D__thread=
│   ├── app.ld                      # Linker script for .so (from plugin SDK)
│   ├── link_app_so.sh              # Extracts libs from build.ninja, links .so
│   ├── fakelib/                    # GENERATED: fake .so for dynamic linking
│   │   ├── libc.so, libm.so, libgcc.so, libpthread.so
│   │   ├── libfreertos.so, libesp-idf.so
│   │   └── libpax-gfx.so, libpax-codecs.so
│   └── main/
│       ├── CMakeLists.txt          # Same deps as original template
│       ├── idf_component.yml       # BSP, WiFi, PAX dependencies
│       ├── main.c                  # Original template app code (unchanged)
│       └── crt0.c                  # _start → app_main() glue
├── sdkconfigs/                     # Shared between graceloader and app
├── partition_tables/               # Unchanged
├── PORTING.md                      # Migration guide
└── tools/
    └── migrate.sh                  # Migration script
```

---

## Build Process

### Graceloader:
```bash
make build   # Standard ESP-IDF build, produces build/tanmatsu/application.bin (315KB)
```

### App:
```bash
# Step 1: Compile all components with -fPIC (app.elf will fail, that's expected)
cd app && idf.py -B ../build/app/tanmatsu build -DDEVICE=tanmatsu \
    -DSDKCONFIG_DEFAULTS="sdkconfigs/general;sdkconfigs/tanmatsu" \
    -DSDKCONFIG=sdkconfig_app_tanmatsu -DIDF_TARGET=esp32p4 -DFAT=0

# Step 2: Link app.so from the compiled .a files
bash app/link_app_so.sh build/app/tanmatsu "$(pwd)/app/app.ld" "$(pwd)/app/fakelib"
```

The `link_app_so.sh` script:
1. Extracts `LINK_LIBRARIES` from `build.ninja` (ESP-IDF's own library list/order)
2. Filters out incompatible system libraries (riscv, esp_system, esp_rom, etc.)
3. Preserves `-Wl,--whole-archive` flags and `-u` symbol pull-in flags
4. Links with `-shared -nostdlib -Wl,--gc-sections -Wl,-Bsymbolic`
5. Does NOT use `-lc -lm -lgcc` (system libs aren't PIC); uses fakelibs instead

---

## FreeRTOS Symbol Export Notes

Many FreeRTOS API names are macros, not real functions. The symbol export lists must use the actual function names:
- `xQueueCreate` → `xQueueGenericCreate`
- `xSemaphoreCreateMutex` → `xQueueCreateMutex`
- `xSemaphoreTake` → `xQueueSemaphoreTake`
- `xTimerStart` → `xTimerGenericCommand`
- `xQueueSend` → `xQueueGenericSend`

Use `riscv32-esp-elf-nm build/tanmatsu/esp-idf/freertos/libfreertos.a | grep " T "` to find real symbol names.

---

## Remaining Work

### Immediate (size optimization):
1. **Version script to hide symbols**: Add `-Wl,--version-script` with `{ global: _start; local: *; }` to strip 367KB of PIC metadata from app.so
2. **Verify size after version script**: app.so should drop to ~1.2-1.3MB
3. **Integrate link step into Makefile**: `make build-app` should handle both idf.py build + link_app_so.sh
4. **Handle app.elf failure gracefully**: The ESP-IDF build always fails at link because of -fPIC; need to either suppress the error or skip that step

### Testing:
5. Flash graceloader to device, copy app.so to SD card
6. Verify kbelf loads and runs app.so correctly
7. Verify BSP, display, WiFi, input all work from the dynamically loaded library

### Polish:
8. Update PORTING.md with final build instructions
9. Update Makefile targets
10. Commit and clean up generated files
11. Consider whether pax-gfx/pax-codecs symbol exports should stay in graceloader

### Open questions:
- Can we suppress the app.elf link failure without hacking ESP-IDF? (Maybe a custom CMake target that only builds .a files)
- Should the graceloader export pax-gfx symbols? (+92KB to graceloader but removes pax from app.so)
- What's the minimum viable graceloader size? (Currently 315KB, ~290KB without symbol table rodata)
