# PoorCraft Quick Start

## Prerequisites
- Git 2.30 or newer with submodule support
- CMake 3.20+ (`cmake --version`)
- A C++20 compiler toolchain (MSVC 2022, clang 14+, or GCC 12+)
- Python 3 for some helper scripts (optional but recommended)
- Platform SDKs and build tools
  - **Windows**: Visual Studio 2022 with "Desktop development with C++" workload
  - **Linux/macOS**: Build essentials (`build-essential`, `clang`, `ninja`, etc.)

## Quick Start in Five Steps
1. **Clone the repository**  
   ```bash
   git clone https://github.com/<your-org>/PoorCraftV2.git
   cd PoorCraftV2
   ```
2. **Initialize and verify submodules**  
   ```bash
   git submodule update --init --recursive
   git submodule status --recursive
   ```
   Confirm key folders (e.g., `libs/glfw/`, `libs/enet/`, `libs/imgui/`, `libs/lua/`, `libs/sol2/`) contain source files, not just `.gitkeep`.
3. **Run the dependency setup script**  
   ```bash
   ./scripts/setup_dependencies.sh
   ```
   On Windows PowerShell or CMD:  
   ```bat
   scripts\setup_dependencies.bat
   ```
   Pass any additional flags (e.g., `--force`) required by your environment. The script downloads assets (GLAD, stb) and validates third-party libraries.
4. **Configure and build the project**  
   ```bash
   cmake -S . -B build
   cmake --build build --parallel
   ```
   On Windows (Developer Command Prompt):  
   ```bat
   cmake -S . -B build -A x64
   cmake --build build --config Release
   ```
   For advanced configuration or generator selection, see `docs/BUILD.md`.
5. **Verify the build and run the game**  
   ```bash
   ./scripts/verify_build.sh
   ./build/bin/PoorCraft
   ```
   On Windows:  
   ```bat
   scripts\verify_build.bat
   .\build\bin\PoorCraft.exe
   ```
   The verification script checks executable presence, size, and required asset folders before launch.

## Submodule Verification Checklist
- Run `git submodule status --recursive` and ensure each entry shows a commit hash (not dashes).
- Inspect `libs/` directories for real content (`dir libs\glfw` on Windows, `ls libs/glfw` on Unix).
- Rerun `git submodule update --init --recursive` if any module displays `-` or `+` prefixes.

## Platform Notes
- **Windows**
  - Use the "x64 Native Tools Command Prompt for VS 2022" before calling CMake.
  - The quick start build defaults to `Release`; override with `cmake --build build --config Debug` if desired.
- **Linux**
  - Install GLFW, X11, and Vulkan SDK prerequisites via your package manager if system libraries are required.
  - Ensure executable permissions on scripts with `chmod +x scripts/*.sh`.
- **macOS**
  - Install Command Line Tools (`xcode-select --install`) and the latest Vulkan SDK (MoltenVK) if targeting Vulkan.
  - Use `brew install cmake ninja` for up-to-date tooling.

## Troubleshooting
- If the build succeeds without producing `build/bin/PoorCraft`, rerun submodule initialization and the setup script before rebuilding.
- For generator or toolchain issues, consult `docs/BUILD.md`.
- For runtime or verification failures, review `docs/TROUBLESHOOTING.md` and the console output from `scripts/verify_build.*`.
- Clear the build with `scripts/clean_build.sh` (Unix) or `scripts\clean_build.bat` (Windows) when switching toolchains or after dependency updates.
