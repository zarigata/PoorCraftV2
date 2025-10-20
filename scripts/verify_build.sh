#!/bin/bash

# scripts/verify_build.sh - Post-build verification for PoorCraft

set -euo pipefail

OUTPUT_DIR="${1:-build/bin}"
EXECUTABLE_NAME="PoorCraft"
EXECUTABLE_PATH="${OUTPUT_DIR}/${EXECUTABLE_NAME}"

if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" || "$OSTYPE" == "cygwin" ]]; then
    EXECUTABLE_NAME="PoorCraft.exe"
    EXECUTABLE_PATH="${OUTPUT_DIR}/${EXECUTABLE_NAME}"
fi

print_status() {
    local status="$1"
    local message="$2"
    printf "[%s] %s\n" "$status" "$message"
}

require_file() {
    local path="$1"
    local description="$2"
    if [ ! -e "$path" ]; then
        print_status "FAIL" "$description missing: $path"
        return 1
    fi
    return 0
}

print_status "INFO" "Verifying build outputs in ${OUTPUT_DIR}"

if ! require_file "$EXECUTABLE_PATH" "Executable"; then
    print_status "HINT" "Run: git submodule update --init --recursive"
    print_status "HINT" "Then rebuild: cmake -B build && cmake --build build"
    exit 1
fi

if EXEC_SIZE=$(stat -c%s "$EXECUTABLE_PATH" 2>/dev/null); then
    :
elif EXEC_SIZE=$(stat -f%z "$EXECUTABLE_PATH" 2>/dev/null); then
    :
else
    print_status "WARN" "Unable to determine executable size"
    EXEC_SIZE=-1
fi

WARN_THRESHOLD="${POORCRAFT_EXEC_WARN_SIZE:-204800}"
if ! [[ "$WARN_THRESHOLD" =~ ^[0-9]+$ ]]; then
    print_status "WARN" "POORCRAFT_EXEC_WARN_SIZE is not numeric; using default 204800"
    WARN_THRESHOLD=204800
fi

FATAL_THRESHOLD="${POORCRAFT_EXEC_FATAL_SIZE:-102400}"
if ! [[ "$FATAL_THRESHOLD" =~ ^[0-9]+$ ]]; then
    print_status "WARN" "POORCRAFT_EXEC_FATAL_SIZE is not numeric; using default 102400"
    FATAL_THRESHOLD=102400
fi

if [ "$FATAL_THRESHOLD" -gt "$WARN_THRESHOLD" ]; then
    FATAL_THRESHOLD="$WARN_THRESHOLD"
fi

if [ "$EXEC_SIZE" -ge 0 ] && [ "$EXEC_SIZE" -lt "$FATAL_THRESHOLD" ]; then
    print_status "FAIL" "Executable size ${EXEC_SIZE} bytes is below fatal threshold ${FATAL_THRESHOLD}"
    exit 1
fi

if [ "$EXEC_SIZE" -ge 0 ] && [ "$EXEC_SIZE" -lt "$WARN_THRESHOLD" ]; then
    print_status "WARN" "Executable size ${EXEC_SIZE} bytes is below warning threshold ${WARN_THRESHOLD}"
elif [ "$EXEC_SIZE" -ge 0 ]; then
    print_status "PASS" "Executable present: ${EXECUTABLE_PATH} (${EXEC_SIZE} bytes)"
else
    print_status "INFO" "Executable present but size could not be verified"
fi

if command -v file >/dev/null 2>&1; then
    BIN_INFO=$(file "$EXECUTABLE_PATH")
    print_status "INFO" "Executable info: ${BIN_INFO}"
fi

if command -v ldd >/dev/null 2>&1 && [[ "$OSTYPE" == "linux-gnu" ]]; then
    print_status "INFO" "Checking shared library dependencies (ldd)"
    ldd "$EXECUTABLE_PATH" | sed 's/^/    /'
elif command -v otool >/dev/null 2>&1 && [[ "$OSTYPE" == "darwin"* ]]; then
    print_status "INFO" "Checking shared library dependencies (otool -L)"
    otool -L "$EXECUTABLE_PATH" | sed 's/^/    /'
fi

require_file "${OUTPUT_DIR}/assets" "Assets directory" || exit 1
require_file "${OUTPUT_DIR}/shaders" "Shaders directory" || exit 1
print_status "PASS" "Assets and shaders directories present"

CONFIG_FILE="config.ini"
if [ -f "$CONFIG_FILE" ]; then
    print_status "PASS" "Config file present in repository root (${CONFIG_FILE})"
else
    print_status "WARN" "Config file not found in repository root"
fi

print_status "PASS" "Build verification completed successfully"
