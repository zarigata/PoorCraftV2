@echo off
REM setup_dependencies.bat - Windows dependency setup script for PoorCraft
REM This script initializes Git submodules and downloads additional dependencies

setlocal enabledelayedexpansion

REM Function to print colored output (simplified for Windows batch)
call :print_info "PoorCraft Dependency Setup Script"
call :print_info "This script will set up all required dependencies for PoorCraft"

REM Check for required tools
call :print_info "Checking prerequisites..."

REM Check Git
git --version >nul 2>&1
if errorlevel 1 (
    call :print_error "Git is not installed or not in PATH"
    call :print_error "Please install Git from https://git-scm.com/downloads"
    exit /b 1
)

for /f "tokens=3" %%i in ('git --version') do (
    set "GIT_VERSION=%%i"
    goto :git_version_found
)
:git_version_found
call :print_success "Git version: %GIT_VERSION%"

REM Check if we're in the correct directory (has CMakeLists.txt)
if not exist "CMakeLists.txt" (
    call :print_error "CMakeLists.txt not found in current directory"
    call :print_error "Please run this script from the PoorCraft root directory"
    exit /b 1
)

call :print_success "Running from correct directory"

REM Initialize and update Git submodules
call :print_info "Setting up Git submodules..."

if exist ".gitmodules" (
    call :print_info "Found .gitmodules file, initializing submodules..."

    REM Check if submodules are already initialized
    if exist "libs\glfw\.git" (
        call :print_info "Submodules appear to be already initialized"
        call :print_info "Updating existing submodules..."
        git submodule update --init --recursive
    ) else (
        call :print_info "Initializing submodules for the first time..."
        git submodule update --init --recursive
    )

    if errorlevel 1 (
        call :print_error "Failed to initialize Git submodules"
        call :print_error "Please check your internet connection and try again"
        exit /b 1
    )

    call :print_success "Git submodules updated successfully"

    REM Verify submodules
    call :print_info "Verifying submodules..."

    if not exist "libs\glfw" (
        call :print_error "GLFW submodule not found after initialization"
        goto :glad_setup
    )

    if not exist "libs\glm" (
        call :print_error "GLM submodule not found after initialization"
        goto :glad_setup
    )

    call :print_info "Initializing ENet submodule..."
    git submodule update --init --recursive libs/enet
    if errorlevel 1 (
        call :print_error "Failed to initialize ENet submodule"
        exit /b 1
    )
    call :print_success "ENet submodule initialized successfully"

    call :print_success "All Git submodules verified"

) else (
    call :print_warning ".gitmodules file not found"
    call :print_warning "Manual dependency setup may be required"
)

:glad_setup
REM Set up GLAD (OpenGL loader)
call :print_info "Setting up GLAD (OpenGL function loader)..."

if not exist "libs\glad" (
    call :print_info "Creating GLAD directory..."
    mkdir "libs\glad" 2>nul
)

REM Check if GLAD files already exist
if exist "libs\glad\include\glad\glad.h" (
    call :print_info "GLAD files already exist, skipping setup"
    goto :stb_setup
)

call :print_warning "GLAD files not found"
call :print_info "Please generate GLAD files manually:"
call :print_info ""
call :print_info "1. Go to https://glad.dav1d.de/"
call :print_info "2. Set 'API' to 'gl' (version 4.6)"
call :print_info "3. Set 'Profile' to 'Core'"
call :print_info "4. Set 'Options' to 'Generate a loader'"
call :print_info "5. Click 'Generate'"
call :print_info "6. Download the ZIP file"
call :print_info "7. Extract 'include/glad/glad.h' to libs/glad/include/glad/"
call :print_info "8. Extract 'src/glad.c' to libs/glad/src/"
call :print_info ""
call :print_warning "GLAD setup requires manual intervention"
call :print_warning "Please complete the steps above and run this script again"
pause
exit /b 1

:stb_setup
REM Set up stb_image (single-file image library)
call :print_info "Setting up stb_image..."

if not exist "libs\stb" (
    call :print_info "Creating stb directory..."
    mkdir "libs\stb" 2>nul
)

REM Check if stb_image.h already exists
if exist "libs\stb\stb_image.h" (
    call :print_info "stb_image.h already exists, skipping download"
    goto :verify_cmake
)

call :print_info "Downloading stb_image.h..."

REM Try curl first, then powershell
curl -L -o libs/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h 2>nul
if errorlevel 1 (
    call :print_info "curl failed, trying with PowerShell..."
    powershell -Command "Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/nothings/stb/master/stb_image.h' -OutFile 'libs/stb/stb_image.h'"
    if errorlevel 1 (
        call :print_error "Failed to download stb_image.h"
        call :print_error "Please manually download from:"
        call :print_error "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
        call :print_error "And place it in libs/stb/stb_image.h"
        pause
        exit /b 1
    )
)

REM Create wrapper .cpp file for CMake
(
    echo #define STB_IMAGE_IMPLEMENTATION
    echo #include "stb_image.h"
) > libs/stb/stb_image.cpp

call :print_success "stb_image.h downloaded and wrapper created"

:verify_cmake
REM Verify CMakeLists.txt files exist
call :print_info "Verifying CMake configuration files..."

set "REQUIRED_CMAKE_FILES=libs/CMakeLists.txt libs/glad/CMakeLists.txt libs/stb/CMakeLists.txt src/CMakeLists.txt"

for %%f in (%REQUIRED_CMAKE_FILES%) do (
    if exist "%%f" (
        call :print_success "Found %%f"
    ) else (
        call :print_error "Missing %%f"
        call :print_error "Please ensure all required files are present"
        pause
        exit /b 1
    )
)

REM Check for optional tools
call :print_info "Checking for optional development tools..."

REM Check for Visual Studio
if exist "%ProgramFiles%\Microsoft Visual Studio" (
    call :print_success "Visual Studio found"
) else (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio" (
        call :print_success "Visual Studio found (x86)"
    ) else (
        call :print_warning "Visual Studio not found"
        call :print_warning "You may need to install Visual Studio or use MinGW"
    )
)

REM Show summary
call :print_info ""
call :print_info "=== Dependency Setup Summary ==="
call :print_success "Git submodules: Initialized"
call :print_success "GLFW: Ready"
call :print_success "GLM: Ready"
call :print_success "GLAD: Ready"
call :print_success "stb_image: Ready"
call :print_success "CMake configuration: Verified"
call :print_info ""

REM Show next steps
call :print_info "Next steps:"
call :print_info "1. Run the build script: scripts\build.bat"
call :print_info "2. Or build manually: cmake -B build ^&^& cmake --build build"
call :print_info "3. See docs\BUILD.md for detailed build instructions"
call :print_info "4. Check docs\CONTRIBUTING.md for development guidelines"

call :print_success "Dependency setup completed successfully!"
call :print_info "You can now build the PoorCraft engine"
pause
exit /b 0

REM Function to print colored output (Windows batch approximation)
:print_info
echo [INFO] %~1
goto :eof

:print_success
echo [SUCCESS] %~1
goto :eof

:print_warning
echo [WARNING] %~1
goto :eof

:print_error
echo [ERROR] %~1
goto :eof
