# FindWiX.cmake
# 
# Finds the WiX Toolset v4+ installation (.NET Tools)
# This module defines:
#   WIX_FOUND          - True if WiX is found
#   WIX_EXECUTABLE     - Path to wix.exe (unified tool in WiX 4+)
#   WIX_VERSION        - Version of WiX found
#   WIX_ROOT_DIR       - Root directory of WiX installation
#   WIX_HEAT_EXECUTABLE - Path to heat.exe (if available)
#
# Usage:
#   find_package(WiX)
#   if(WIX_FOUND)
#       # Use ${WIX_EXECUTABLE} for building
#   endif()

# Look for WiX v4+ (.NET Tools)
set(_WIX_SEARCH_PATHS
    "$ENV{WIX}/bin"
    "$ENV{ProgramFiles}/WiX Toolset v6.0/bin"
    "$ENV{ProgramFiles}/WiX Toolset v5.0/bin"
    "$ENV{ProgramFiles}/WiX Toolset v4.0/bin"
    "$ENV{ProgramFiles(x86)}/WiX Toolset v6.0/bin"
    "$ENV{ProgramFiles(x86)}/WiX Toolset v5.0/bin"
    "$ENV{ProgramFiles(x86)}/WiX Toolset v4.0/bin"
)

# Find the main WiX executable
# WiX v4+ uses unified wix.exe
find_program(WIX_EXECUTABLE
    NAMES wix.exe
    PATHS ${_WIX_SEARCH_PATHS}
    DOC "Path to WiX compiler executable (wix.exe)"
)

# Find heat.exe (WiX harvesting tool)
find_program(WIX_HEAT_EXECUTABLE
    NAMES heat.exe
    PATHS ${_WIX_SEARCH_PATHS}
    DOC "Path to WiX harvesting tool (heat.exe)"
)

# Get WiX version
if(WIX_EXECUTABLE)
    get_filename_component(WIX_ROOT_DIR "${WIX_EXECUTABLE}" DIRECTORY)
    get_filename_component(WIX_ROOT_DIR "${WIX_ROOT_DIR}" DIRECTORY)
    
    # Get version from wix.exe
    execute_process(
        COMMAND "${WIX_EXECUTABLE}" --version
        OUTPUT_VARIABLE WIX_VERSION_OUTPUT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(WIX_VERSION_OUTPUT MATCHES "wix.exe version ([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(WIX_VERSION "${CMAKE_MATCH_1}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WiX
    REQUIRED_VARS WIX_EXECUTABLE
    VERSION_VAR WIX_VERSION
    FAIL_MESSAGE "WiX Toolset v4+ not found. Please install WiX Toolset v4.0 or later from https://wixtoolset.org/"
)

mark_as_advanced(
    WIX_EXECUTABLE
    WIX_HEAT_EXECUTABLE
    WIX_ROOT_DIR
)

# Provide information about the found WiX version
if(WIX_FOUND)
    message(STATUS "Found WiX v${WIX_VERSION} at ${WIX_ROOT_DIR}")
    message(STATUS "  WiX: ${WIX_EXECUTABLE}")
    
    if(WIX_HEAT_EXECUTABLE)
        message(STATUS "  Heat: ${WIX_HEAT_EXECUTABLE}")
    endif()
endif()