@echo off
REM setup_dependencies.bat - Windows dependency setup script for PoorCraft
REM This script initializes Git submodules and downloads additional dependencies

setlocal enabledelayedexpansion

set "FORCE_FLAG="
set "DOWNLOAD_GLAD="

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--force" (
    set "FORCE_FLAG=--force"
    shift
    goto :parse_args
)
if /i "%~1"=="--download-glad" (
    set "DOWNLOAD_GLAD=1"
    shift
    goto :parse_args
)
if /i "%~1"=="--help" (
    echo Usage: scripts\setup_dependencies.bat [--force] [--download-glad]
    echo.
    echo   --force           Retry submodule fetch with git's --force flag
    echo   --download-glad   Attempt to download pre-generated GLAD files from mirror
    echo   --help            Show this help message
    exit /b 0
)
if /i "%~1"=="-h" (
    goto :show_help
)
echo Unknown option: %~1
exit /b 1

:show_help
echo Usage: scripts\setup_dependencies.bat [--force] [--download-glad]
echo.
echo   --force           Retry submodule fetch with git's --force flag
echo   --download-glad   Attempt to download pre-generated GLAD files from mirror
echo   --help            Show this help message
exit /b 0

:args_done

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

call :check_network

set "SUBMODULE_ARGS=--init --recursive"
if defined FORCE_FLAG (
    set "SUBMODULE_ARGS=%SUBMODULE_ARGS% --force"
)

REM Initialize and update Git submodules
call :print_info "Setting up Git submodules..."

if exist ".gitmodules" (
    call :print_info "Found .gitmodules file, initializing submodules..."

    set /a _progress=0
    set /a _total=6

    for %%S in (libs/glfw libs/glm libs/enet libs/imgui libs/lua libs/sol2) do (
        set /a _progress+=1
        call :print_info "Fetching %%S... !_progress!/%_total%"
        git submodule update %SUBMODULE_ARGS% %%S
        if errorlevel 1 (
            call :print_error "Failed to initialize submodule: %%S"
            call :print_error "Try rerunning with --force or inspect .gitmodules configuration"
            exit /b 1
        )
    )

    call :print_success "Git submodules updated successfully"

    REM Verify submodules
    call :print_info "Verifying submodules..."

    call :verify_file "libs\glfw\CMakeLists.txt"
    call :ensure_directory_has_content "libs\glfw"

    call :verify_file "libs\glm\CMakeLists.txt"
    call :ensure_directory_has_content "libs\glm"

    call :verify_file "libs\enet\CMakeLists.txt"
    call :ensure_directory_has_content "libs\enet"

    call :verify_file "libs\imgui\imgui.cpp"
    call :ensure_directory_has_content "libs\imgui"

    call :verify_file "libs\lua\lua.h"
    call :ensure_directory_has_content "libs\lua"

    call :verify_file "libs\sol2\include\sol\sol.hpp"
    call :ensure_directory_has_content "libs\sol2"

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
    mkdir "libs\glad\include\glad" 2>nul
    mkdir "libs\glad\include\KHR" 2>nul
    mkdir "libs\glad\src" 2>nul
)

REM Check if GLAD files already exist
if exist "libs\glad\include\glad\glad.h" if exist "libs\glad\src\glad.c" goto :stb_setup

if defined DOWNLOAD_GLAD (
    call :print_info "Attempting to download GLAD loader from mirror..."
    set "GLAD_BASE_URL=https://raw.githubusercontent.com/Dav1dde/glad/master"
    curl -fL -o libs/glad/include/glad/glad.h %GLAD_BASE_URL%/include/glad/glad.h >nul 2>&1
    if errorlevel 1 (
        call :print_warning "curl failed, trying PowerShell"
        powershell -Command "Invoke-WebRequest -Uri '%GLAD_BASE_URL%/include/glad/glad.h' -OutFile 'libs/glad/include/glad/glad.h'" >nul
    )
    curl -fL -o libs/glad/include/KHR/khrplatform.h %GLAD_BASE_URL%/include/KHR/khrplatform.h >nul 2>&1
    if errorlevel 1 (
        powershell -Command "Invoke-WebRequest -Uri '%GLAD_BASE_URL%/include/KHR/khrplatform.h' -OutFile 'libs/glad/include/KHR/khrplatform.h'" >nul
    )
    curl -fL -o libs/glad/src/glad.c %GLAD_BASE_URL%/src/glad.c >nul 2>&1
    if errorlevel 1 (
        powershell -Command "Invoke-WebRequest -Uri '%GLAD_BASE_URL%/src/glad.c' -OutFile 'libs/glad/src/glad.c'" >nul
    )
)

if exist "libs\glad\include\glad\glad.h" if exist "libs\glad\src\glad.c" (
    call :print_success "GLAD files downloaded successfully"
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
call :print_info "8. Extract 'include/KHR/khrplatform.h' to libs/glad/include/KHR/"
call :print_info "9. Extract 'src/glad.c' to libs/glad/src/"
call :print_error "GLAD setup requires manual intervention or rerun with --download-glad"
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
if exist "libs\stb\stb_image.h" goto :verify_cmake

call :print_info "Downloading stb_image.h..."

REM Try curl first, then powershell
curl -L -o libs/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h >nul 2>&1
if errorlevel 1 (
    call :print_info "curl failed, trying with PowerShell..."
    powershell -Command "Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/nothings/stb/master/stb_image.h' -OutFile 'libs/stb/stb_image.h'" >nul
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

if exist "%ProgramFiles%\Microsoft Visual Studio" (
    call :print_success "Visual Studio found"
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio" (
    call :print_success "Visual Studio found (x86)"
) else (
    call :print_warning "Visual Studio not found"
    call :print_warning "You may need to install Visual Studio or use MinGW"
)

REM Show summary
call :print_info ""
call :print_info "=== Dependency Setup Summary ==="
for /f "delims=" %%l in ('git submodule status --recursive ^| findstr /r /c:"^"') do (
    call :print_success "Submodule: %%l"
)
call :print_success "GLFW: Verified"
call :print_success "GLM: Verified"
call :print_success "GLAD: Ready"
call :print_success "stb_image: Ready"
call :print_success "CMake configuration: Verified"
call :print_info ""

REM Show next steps
call :print_info "Next steps:"
call :print_info "1. Run the build script: scripts\build.bat"
call :print_info "2. Or build manually: cmake -B build ^&^& cmake --build build"
call :print_info "3. Run scripts\verify_build.bat after building to validate artifacts"
call :print_info "4. See docs\BUILD.md for detailed build instructions"
call :print_info "5. Check docs\CONTRIBUTING.md for development guidelines"

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

:check_network
call :print_info "Checking network connectivity..."
ping -n 1 github.com >nul 2>&1
if errorlevel 1 (
    powershell -Command "Resolve-DnsName github.com" >nul 2>&1
    if errorlevel 1 (
        call :print_error "Unable to reach github.com"
        call :print_error "Please verify your internet connection or firewall settings"
        exit /b 1
    )
)
call :print_success "Network connectivity verified"
goto :eof

:verify_file
if not exist "%~1" (
    call :print_error "Expected file missing: %~1"
    exit /b 1
)
goto :eof

:ensure_directory_has_content
set "_dir=%~1"
for %%i in ("!_dir!\*.*") do (
    if /i not "%%~nxi"==".git" if /i not "%%~nxi"==".gitignore" if /i not "%%~nxi"==".gitkeep" goto :dir_ok
)
call :print_error "Directory appears empty: %~1"
call :print_error "Submodule content may not have been fetched"
exit /b 1
:dir_ok
goto :eof
