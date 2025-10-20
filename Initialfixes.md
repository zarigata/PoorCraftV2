# Build & Dependency Issues Summary

## Current Status
- **Status**: Resolved on 2025-10-19 23:54 (UTC-03)
- **Build outcome**: `cmake --build build --parallel` now produces `build/bin/PoorCraft` artifacts after dependency restoration.
- **Executable**: `build/bin/PoorCraft` present alongside assets and shaders directories.

## Observed Problems
- **Empty dependency submodules**: Directories like `libs/glfw/`, `libs/enet/`, `libs/imgui/`, `libs/lua/`, and `libs/sol2/` contain only placeholder files (`.gitkeep`). Their source trees were not fetched, so CMake targets defined in `libs/CMakeLists.txt` have no real source files.
- **Dependency setup script silent**: Running `./scripts/setup_dependencies.sh` exits with success and no logs. The script depends on populated submodules to validate dependencies; with empty directories it cannot report missing content.
- **Build produces no artifacts**: Because dependency targets lack sources, the build completes instantly without compiling anything. No executables or libraries are emitted, leaving `build/` nearly empty.

## Impact
- **Game untestable**: Without third-party sources, the engine binary never builds, preventing any gameplay validation.
- **False positives**: Quiet build success can mislead the team into believing the project compiled correctly.

## Suggested Fixes
- **Re-fetch submodules**: Ensure network access and run `git submodule update --init --recursive` so each `libs/` subdirectory is populated with real source files.
- **Re-run setup**: After submodules exist, rerun `./scripts/setup_dependencies.sh` to download `stb_image.h` and confirm GLAD assets. Expect the script to print progress logs.
- **Rebuild project**: Execute `cmake -S . -B build` followed by `cmake --build build --parallel`. Verify `build/bin/PoorCraft` exists and is executable.
- **Add CI check**: Consider a CI job that fails when expected outputs like `build/bin/PoorCraft` are missing despite a reported successful build.

## Resolution
- Updated Git submodules via `git submodule update --init --recursive` to repopulate `libs/` contents.
- Reran `./scripts/setup_dependencies.sh` to restore GLAD, stb, and ancillary assets with visible progress logs.
- Performed a clean rebuild using `scripts/clean_build.sh` (and mirrored changes to `scripts\clean_build.bat`) to regenerate `build/bin/PoorCraft` with required resources.

## Verification
- Confirmed submodule integrity with `git submodule status --recursive` (no leading `-` or `+`).
- Executed `scripts/clean_build.sh` followed by `scripts/verify_build.sh` to ensure the executable, assets, and shaders are present and sized above safety thresholds.
- On Windows matrix, ran `scripts\clean_build.bat --config Release` then `scripts\verify_build.bat` to validate MSVC output.

## Prevention
- Documented the end-to-end setup in `QUICKSTART.md` and cross-referenced troubleshooting guides for future onboarding.
- Added build verification thresholds (fatal and warning) across CMake and scripts to catch missing dependencies without false positives.
- Established clean build scripts (`scripts/clean_build.sh` and `scripts\clean_build.bat`) that always reinitialize submodules and rerun dependency setup, ensuring CI and developers rebuild from a consistent baseline.