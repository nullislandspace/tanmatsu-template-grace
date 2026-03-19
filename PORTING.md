# Porting Guide: Migrating to the Graceloader Template

This template uses **Graceloader** — a minimal ELF library loader that saves appfs
space by loading app code from the SD card or internal flash.

## Architecture

- **Graceloader** (tiny binary in appfs): Mounts `/int` and `/sd` filesystems, then
  loads your `app.so` via the kbelf ELF loader.
- **app.so** (on SD card or internal flash): Contains your actual app code plus all
  its dependencies (BSP, WiFi, PAX, etc.) compiled with `-fPIC`.

## Migrating an Existing App

### Quick Migration

Run the migration script:
```bash
./tools/migrate.sh
```

### Manual Migration

1. **Move your code** from `main/main.c` to `app/main/main.c`

2. **Your `app_main()` stays the same** — all BSP, WiFi, PAX, and input calls
   are unchanged.

3. **Filesystems are pre-mounted**: `/int` and `/sd` are already available when
   your code runs. Do NOT try to mount them again.

4. **Add dependencies** to `app/main/idf_component.yml` if you use additional
   ESP-IDF managed components.

5. **Add component .a files** to the link step in `app/CMakeLists.txt` if your
   app uses components beyond the defaults.

## Building

```bash
# Build the graceloader (installed to appfs)
make build

# Build your app library
make build-app

# Flash graceloader to device
make install

# Upload app.so to SD card
make install-app
```

## Adding New Exported Symbols

If your app uses ESP-IDF core functions not currently exported by Graceloader:

1. Add the symbol name to the appropriate file in `main/symbol_export/`:
   - `c` — C standard library functions
   - `m` — Math functions
   - `freertos` — FreeRTOS primitives
   - `esp-idf` — ESP-IDF core (GPIO, NVS, heap, logging, etc.)
   - `pax-gfx` — PAX graphics functions
   - `pax-codecs` — PAX codec functions

2. Regenerate symbol tables:
   ```bash
   make regenerate-symbols
   ```

3. Rebuild both graceloader and app:
   ```bash
   make build && make build-app
   ```

## Directory Structure

```
main/           → Graceloader source (DO NOT put app code here)
app/main/       → Your app code goes here
app/fakelib/    → Generated fake .so libraries (do not edit)
components/     → Local components (kbelf ELF loader)
```
