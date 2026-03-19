---
name: Graceloader Project
description: ELF library loader for Tanmatsu apps - current status, architecture decisions, and technical findings
type: project
---

The minimal ELF library loader is called "graceloader", named after Grace Hopper.

## Architecture
- Graceloader (315KB in appfs): mounts /int + /sd, loads app.so via kbelf
- app.so (on SD/int): full app with BSP/WiFi/PAX compiled -fPIC, linked as shared object
- Standard template for comparison: 968KB monolithic binary

## Key decisions made:
- Graceloader does NOT include BSP/WiFi/PAX (user requirement: keep it small)
- Graceloader exports libc/libm/libgcc via ROM at zero cost (weak symbol refs)
- App is a full ESP-IDF project compiled with -fPIC, linked as .so
- `__thread` keyword stripped via `-D__thread=` (TLS incompatible with -shared on RISC-V)
- System libc/libm/libgcc from toolchain must NOT be linked into .so (not PIC)
- Fakelibs provide dynamic references to graceloader-exported symbols

## Current blocker: app.so is 2.3MB vs 968KB template
- Actual code (text): 913KB — comparable to template
- PIC metadata bloat: 367KB (dynsym, dynstr, hash, rela.dyn)
- Next step: version script to hide internal symbols, should recover most of 367KB

## Important files:
- Plan/tracking: /home/cavac/.claude/plans/tranquil-spinning-music.md
- Graceloader main: main/main.c
- App code: app/main/main.c
- kbelf + platform: components/kbelf/
- Symbol exports: main/symbol_export/
- Link script: app/link_app_so.sh
