# Release changelog

## 4.1.1 - 2026-02-05

* API changed: Modified `jfofs_add()` function signature.
* API changed: Added `jfofs_version()` function to retrieve the library version string.
* Build system: Updated version macros in `config.h` to include a full version string.
* Bugfix: Fixed potential clipping issue in `test_manager.c` by reducing per-fof amplitude.
* Code cleanup: Removed unnecessary preprocessor directives in `test_mix_client.c`.
