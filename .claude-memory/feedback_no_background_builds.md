---
name: No background builds
description: Run build commands in foreground, don't use background + sleep polling
type: feedback
---

Run build commands in foreground, not background with sleep polling.

**Why:** Running builds in background then sleeping a fixed time wastes time — either sleeping too long or checking too early. The user finds this frustrating.

**How to apply:** Always run build commands (make, idf.py, cmake, etc.) directly in foreground Bash calls with appropriate timeout. Never use run_in_background for builds.
