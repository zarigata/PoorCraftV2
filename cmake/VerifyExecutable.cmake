# cmake/VerifyExecutable.cmake - Ensures the PoorCraft executable was produced

if(NOT DEFINED EXPECTED_EXECUTABLE)
    message(FATAL_ERROR "EXPECTED_EXECUTABLE variable is not defined for verification")
endif()

get_filename_component(_exec_path "${EXPECTED_EXECUTABLE}" ABSOLUTE)
message(STATUS "Checking for executable at ${_exec_path}")

if(NOT EXISTS "${_exec_path}")
    message(FATAL_ERROR "Build succeeded but executable not found at ${_exec_path}.\nEnsure Git submodules are initialized and rerun the build.")
endif()

file(SIZE "${_exec_path}" _exec_size)
set(_warn_threshold 204800)
set(_fatal_threshold 102400)

if(DEFINED ENV{POORCRAFT_EXEC_WARN_SIZE})
    string(REGEX MATCH "^[0-9]+$" _warn_match "$ENV{POORCRAFT_EXEC_WARN_SIZE}")
    if(_warn_match)
        set(_warn_threshold $ENV{POORCRAFT_EXEC_WARN_SIZE})
    else()
        message(WARNING "Environment variable POORCRAFT_EXEC_WARN_SIZE is not an integer; using default ${_warn_threshold} bytes.")
    endif()
endif()

if(DEFINED ENV{POORCRAFT_EXEC_FATAL_SIZE})
    string(REGEX MATCH "^[0-9]+$" _fatal_match "$ENV{POORCRAFT_EXEC_FATAL_SIZE}")
    if(_fatal_match)
        set(_fatal_threshold $ENV{POORCRAFT_EXEC_FATAL_SIZE})
    else()
        message(WARNING "Environment variable POORCRAFT_EXEC_FATAL_SIZE is not an integer; using default ${_fatal_threshold} bytes.")
    endif()
endif()

if(_fatal_threshold GREATER _warn_threshold)
    set(_fatal_threshold ${_warn_threshold})
endif()

if(_exec_size LESS _fatal_threshold)
    message(FATAL_ERROR "Executable at ${_exec_path} is unexpectedly small (${_exec_size} bytes).\nThis usually indicates missing dependencies or empty targets.")
endif()

if(_exec_size LESS _warn_threshold)
    message(WARNING "Executable at ${_exec_path} (${_exec_size} bytes) is below the warning threshold of ${_warn_threshold} bytes. Set POORCRAFT_EXEC_WARN_SIZE to adjust or investigate missing assets.")
else()
    message(STATUS "Executable verification passed: ${_exec_path} (${_exec_size} bytes)")
endif()
