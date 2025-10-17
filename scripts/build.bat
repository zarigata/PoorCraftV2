@echo off
REM build.bat - Windows build script for PoorCraft engine
REM This script automates the build process for Windows using Visual Studio or MinGW

setlocal enabledelayedexpansion

REM Colors for output (Windows 10+)
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

REM Function to print colored output (simplified for Windows)
call :print_info "PoorCraft Build Script"
call :print_info "Starting Windows build process..."

REM Default values
set "BUILD_TYPE=Release"
set "BUILD_DIR=build"
set "GENERATOR=Visual Studio 16 2019"
set "CLEAN_BUILD=false"
set "VERBOSE=false"

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :end_parse_args

if "%~1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if "%~1"=="--release" (
    set "BUILD_TYPE=Release"
    shift
    goto :parse_args
)
if "%~1"=="--clean" (
    set "CLEAN_BUILD=true"
    shift
    goto :parse_args
)
if "%~1"=="--verbose" (
    set "VERBOSE=true"
    shift
    goto :parse_args
)
if "%~1"=="--mingw" (
    set "GENERATOR=MinGW Makefiles"
    shift
    goto :parse_args
)
if "%~1"=="--help" (
    call :print_help
    exit /b 0
)

REM Unknown option
call :print_error "Unknown option: %~1"
call :print_info "Use --help for usage information"
exit /b 1

:end_parse_args

call :print_info "Build Type: %BUILD_TYPE%"
call :print_info "Build Directory: %BUILD_DIR%"
call :print_info "Generator: %GENERATOR%"

REM Check for required tools
call :print_info "Checking prerequisites..."

REM Check CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    call :print_error "CMake is not installed or not in PATH"
    call :print_error "Please install CMake 3.15 or higher from https://cmake.org/download/"
    exit /b 1
)

for /f "tokens=3" %%i in ('cmake --version') do (
    set "CMAKE_VERSION=%%i"
    goto :cmake_version_found
)
:cmake_version_found
call :print_success "CMake version: %CMAKE_VERSION%"

REM Clean build directory if requested
if "%CLEAN_BUILD%"=="true" (
    call :print_info "Cleaning build directory..."
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM Create build directory
if not exist "%BUILD_DIR%" (
    call :print_info "Creating build directory: %BUILD_DIR%"
    mkdir "%BUILD_DIR%"
)

REM Change to build directory
cd "%BUILD_DIR%"

REM Configure with CMake
call :print_info "Configuring with CMake..."

set "CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%"

if "%VERBOSE%"=="true" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_VERBOSE_MAKEFILE=ON"
)

call :print_info "CMake arguments: %CMAKE_ARGS%"

cmake .. -G "%GENERATOR%" %CMAKE_ARGS%
if errorlevel 1 (
    call :print_error "CMake configuration failed"
    exit /b 1
)

call :print_success "CMake configuration completed"

REM Build the project
call :print_info "Building project..."

if "%GENERATOR%"=="MinGW Makefiles" (
    REM Use mingw32-make for MinGW
    if "%VERBOSE%"=="true" (
        mingw32-make -j%NUMBER_OF_PROCESSORS%
    ) else (
        mingw32-make -j%NUMBER_OF_PROCESSORS% >nul 2>&1
    )
) else (
    REM Use cmake --build for Visual Studio
    set "BUILD_ARGS=--build . --config %BUILD_TYPE%"

    if "%VERBOSE%"=="true" (
        set "BUILD_ARGS=--verbose %BUILD_ARGS%"
    )

    cmake %BUILD_ARGS%
)

if errorlevel 1 (
    call :print_error "Build failed"
    exit /b 1
)

call :print_success "Build completed successfully"

REM Show build results
call :print_info "Build summary:"
echo   Build Type: %BUILD_TYPE%
echo   Output Directory: %CD%\bin
if "%GENERATOR%"=="MinGW Makefiles" (
    echo   Executable: %CD%\bin\PoorCraft.exe
) else (
    echo   Solution: %CD%\PoorCraft.sln
)

REM Check if executable was created
if "%GENERATOR%"=="MinGW Makefiles" (
    if exist "bin\PoorCraft.exe" (
        for %%i in ("bin\PoorCraft.exe") do set "EXECUTABLE_SIZE=%%~zi"
        call :print_success "Executable created successfully (!EXECUTABLE_SIZE! bytes)"
    ) else (
        call :print_warning "Executable not found in expected location"
    )
) else (
    if exist "bin\%BUILD_TYPE%\PoorCraft.exe" (
        for %%i in ("bin\%BUILD_TYPE%\PoorCraft.exe") do set "EXECUTABLE_SIZE=%%~zi"
        call :print_success "Executable created successfully (!EXECUTABLE_SIZE! bytes)"
    ) else (
        call :print_warning "Executable not found in expected location"
    )
)

REM Show next steps
echo.
call :print_info "Next steps:"
if "%GENERATOR%"=="MinGW Makefiles" (
    echo   1. Run the engine: .\bin\PoorCraft.exe
) else (
    echo   1. Open PoorCraft.sln in Visual Studio and build/run
    echo   2. Or run: .\bin\%BUILD_TYPE%\PoorCraft.exe
)
echo   3. See docs\BUILD.md for detailed build documentation
echo   4. Check docs\CONTRIBUTING.md for development guidelines

call :print_success "Build script completed successfully!"
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

:print_help
echo Usage: %0 [options]
echo.
echo Options:
echo   --debug           Build in Debug mode ^(default: Release^)
echo   --release         Build in Release mode
echo   --clean           Clean build directory before building
echo   --verbose         Enable verbose output
echo   --mingw           Use MinGW Makefiles instead of Visual Studio
echo   --help            Show this help message
echo.
echo Examples:
echo   %0                    Build in Release mode with Visual Studio
echo   %0 --debug            Build in Debug mode
echo   %0 --mingw --debug    Build in Debug mode with MinGW
echo   %0 --clean --verbose  Clean build and show verbose output
goto :eof
