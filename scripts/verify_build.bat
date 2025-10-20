@echo off
REM scripts\verify_build.bat - Post-build verification for PoorCraft (Windows)

setlocal enabledelayedexpansion

set "OUTPUT_ROOT=build\bin"
if not "%~1"=="" (
    set "OUTPUT_ROOT=%~1"
)

set "CANDIDATE_EXECUTABLES=%OUTPUT_ROOT%\PoorCraft.exe %OUTPUT_ROOT%\Release\PoorCraft.exe %OUTPUT_ROOT%\Debug\PoorCraft.exe"
set "FOUND_EXECUTABLE="

for %%E in (%CANDIDATE_EXECUTABLES%) do (
    if exist "%%~fE" (
        set "FOUND_EXECUTABLE=%%~fE"
        goto :exec_found
    )
)

echo [FAIL] Executable not found in expected locations.
echo [HINT] Verify Git submodules: git submodule update --init --recursive
echo [HINT] Rebuild: cmake -B build ^&^& cmake --build build --config Release
exit /b 1

:exec_found
echo [PASS] Executable located: %FOUND_EXECUTABLE%

for %%F in ("%FOUND_EXECUTABLE%") do set "EXEC_SIZE=%%~zF"
if not defined EXEC_SIZE set "EXEC_SIZE=0"

set "WARN_THRESHOLD=204800"
if defined POORCRAFT_EXEC_WARN_SIZE set "WARN_THRESHOLD=%POORCRAFT_EXEC_WARN_SIZE%"
call :ensure_numeric WARN_THRESHOLD 204800 "POORCRAFT_EXEC_WARN_SIZE"

set "FATAL_THRESHOLD=102400"
if defined POORCRAFT_EXEC_FATAL_SIZE set "FATAL_THRESHOLD=%POORCRAFT_EXEC_FATAL_SIZE%"
call :ensure_numeric FATAL_THRESHOLD 102400 "POORCRAFT_EXEC_FATAL_SIZE"

if %FATAL_THRESHOLD% GTR %WARN_THRESHOLD% set "FATAL_THRESHOLD=%WARN_THRESHOLD%"

if %EXEC_SIZE% LSS %FATAL_THRESHOLD% (
    echo [FAIL] Executable size %EXEC_SIZE% bytes is below fatal threshold %FATAL_THRESHOLD%
    exit /b 1
)

if %EXEC_SIZE% LSS %WARN_THRESHOLD% (
    echo [WARN] Executable size %EXEC_SIZE% bytes is below warning threshold %WARN_THRESHOLD%
) else (
    echo [PASS] Executable size OK: %EXEC_SIZE% bytes
)

echo [INFO] Collecting dependency information...
if exist "%SystemRoot%\System32\where.exe" (
    for %%T in (where) do set "WHERE_CMD=%%~fT"
) else (
    set "WHERE_CMD=where"
)
if defined WHERE_CMD (
    "%WHERE_CMD%" /Q dumpbin >nul 2>&1
    if not errorlevel 1 (
        for /f "usebackq delims=" %%L in (`dumpbin /dependents "%FOUND_EXECUTABLE%" 2^>nul ^| findstr /r /c:"^    "`) do echo [INFO] %%L
    ) else (
        echo [WARN] dumpbin not available; skipping DLL dependency listing
    )
)

for %%P in ("%FOUND_EXECUTABLE%") do set "EXEC_DIR=%%~dpP"
if exist "%EXEC_DIR%assets" (
    echo [PASS] Assets directory present: %EXEC_DIR%assets
) else (
    echo [FAIL] Assets directory missing next to executable (%EXEC_DIR%assets)
    exit /b 1
)
if exist "%EXEC_DIR%shaders" (
    echo [PASS] Shaders directory present: %EXEC_DIR%shaders
) else (
    echo [FAIL] Shaders directory missing next to executable (%EXEC_DIR%shaders)
    exit /b 1
)

if exist "config.ini" (
    echo [PASS] Config file present in repository root (config.ini)
) else (
    echo [WARN] Config file not found in repository root
)

echo [PASS] Build verification completed successfully
exit /b 0

:ensure_numeric
    set "VAR_NAME=%~1"
    set "DEFAULT_VALUE=%~2"
    set "ENV_LABEL=%~3"
    set "CURRENT=!%VAR_NAME%!"
    if not defined CURRENT (
        set "%VAR_NAME%=%DEFAULT_VALUE%"
        goto :eof
    )
    set "_NON_NUM="
    for /f "delims=0123456789" %%N in ("!CURRENT!") do set "_NON_NUM=%%N"
    if defined _NON_NUM (
        if not "%ENV_LABEL%"=="" (
            echo [WARN] %ENV_LABEL% is not numeric; using default %DEFAULT_VALUE%
        ) else (
            echo [WARN] Invalid numeric value for %VAR_NAME%; using default %DEFAULT_VALUE%
        )
        set "%VAR_NAME%=%DEFAULT_VALUE%"
    )
    goto :eof
