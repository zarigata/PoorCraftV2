@echo off
REM scripts\clean_build.bat - Clean rebuild helper for PoorCraft (Windows)

setlocal enabledelayedexpansion

set "ARCH=x64"
if not "%CMAKE_ARCH%"=="" set "ARCH=%CMAKE_ARCH%"

set "CONFIG=Release"
if not "%BUILD_CONFIG%"=="" set "CONFIG=%BUILD_CONFIG%"

set "NEXT_IS_CONFIG=0"
for %%A in (%*) do (
    if !NEXT_IS_CONFIG! EQU 1 (
        set "CONFIG=%%~A"
        set "NEXT_IS_CONFIG=0"
    ) else (
        if /I "%%~A"=="--config" (
            set "NEXT_IS_CONFIG=1"
        )
    )
)

call :print_step "Removing previous build artifacts"
if exist build (
    rmdir /s /q build
    if errorlevel 1 goto :error
)
if exist CMakeCache.txt del /f /q CMakeCache.txt
if exist CMakeFiles rmdir /s /q CMakeFiles

call :print_step "Verifying dependency submodules"
call scripts\setup_dependencies.bat %*
if errorlevel 1 goto :error

call :print_step "Configuring project (arch=%ARCH%)"
cmake -S . -B build -A %ARCH%
if errorlevel 1 goto :error

call :print_step "Building project (config=%CONFIG%)"
cmake --build build --config %CONFIG%
if errorlevel 1 goto :error

call :print_step "Running build verification"
call scripts\verify_build.bat
if errorlevel 1 goto :error

call :print_step "Clean build completed successfully"
echo [SUMMARY] Configuration=%CONFIG%, Architecture=%ARCH%
exit /b 0

:print_step
    echo [STEP] %~1
    goto :eof

:error
    echo [ERROR] Clean build failed with errorlevel %errorlevel%
    exit /b %errorlevel%
