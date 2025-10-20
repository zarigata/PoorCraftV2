#!/bin/bash

# scripts/clean_build.sh - Clean rebuild helper for PoorCraft

set -euo pipefail

print_step() {
    echo "[STEP] $1"
}

print_step "Removing previous build artifacts"
rm -rf build/
rm -f CMakeCache.txt
rm -rf CMakeFiles/

print_step "Verifying dependency submodules"
./scripts/setup_dependencies.sh "$@"

print_step "Configuring project"
cmake -B build

print_step "Building project"
cmake --build build --parallel

print_step "Running build verification"
./scripts/verify_build.sh

print_step "Clean build completed successfully"
