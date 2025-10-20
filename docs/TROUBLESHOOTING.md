# PoorCraft Troubleshooting Guide

## Build Succeeds But No Executable Produced

### Symptoms
- `cmake --build build` (or Visual Studio/MSBuild) completes without errors.
- `build/bin/` directory is empty or missing.
- Expected executable `build/bin/PoorCraft` (or `PoorCraft.exe`) is not generated.

### Root Cause
Git submodules for third-party libraries were not initialized. Directories like `libs/glfw/`, `libs/enet/`, `libs/imgui/`, `libs/lua/`, and `libs/sol2/` contain only placeholder files. CMake still creates targets but they have no sources, so the build pipeline finishes instantly without producing binaries.

### Resolution Steps
```bash
# 1. Confirm submodules are empty
ls libs/glfw/
ls libs/imgui/

# 2. Initialize (or reinitialize) submodules
git submodule update --init --recursive

# 3. Re-run the dependency setup script for validation
./scripts/setup_dependencies.sh          # Linux/macOS
scripts\setup_dependencies.bat          # Windows

# 4. Perform a clean rebuild
rm -rf build/
cmake -B build
cmake --build build --parallel

# 5. Verify the executable
ls -lh build/bin/PoorCraft
./scripts/verify_build.sh
```

Windows verification:
```cmd
dir build\bin\Release\PoorCraft.exe
scripts\verify_build.bat
```

### Prevention
- Always run `git submodule update --init --recursive` after cloning.
- Use `./scripts/setup_dependencies.sh` (or `.bat`) to validate dependency state.
- Run `./scripts/verify_build.sh` (or `.bat`) after every build to ensure artifacts exist.
- Continuous integration workflow `.github/workflows/build.yml` runs these checks for every commit.

## GLAD Files Missing

### Symptoms
- CMake configuration fails with messages about missing `glad.h` or `glad.c`.

### Resolution
- Generate GLAD loader from <https://glad.dav1d.de/> (OpenGL 4.6 Core, generate loader).
- Place `include/glad/glad.h` in `libs/glad/include/glad/` and `src/glad.c` in `libs/glad/src/`.
- Re-run setup: `./scripts/setup_dependencies.sh`.

## Network Failures During Submodule Fetch

### Symptoms
- `git submodule update --init --recursive` hangs or fails.
- Setup script reports empty submodule directories.

### Resolution
- Check connectivity: `ping github.com`.
- Retry with force: `git submodule update --init --recursive --force`.
- Confirm `.gitmodules` uses HTTPS URLs.
- If firewall blocks Git, download submodule ZIP archives manually and extract into `libs/`.

## Executable Present But Crashes Immediately

### Checklist
- Ensure GPU drivers support OpenGL 4.6.
- Verify assets copied beside executable (`build/bin/assets/`, `build/bin/shaders/`).
- Run with `--help` to confirm command line parsing.
- Check `config.ini` for invalid values.

For additional issues, consult `docs/BUILD.md` and project issue tracker.
