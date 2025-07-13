# FindWiX.cmake
# 
# Finds the WiX Toolset v6 installation
# This module defines:
#   WIX_FOUND          - True if WiX is found
#   WIX_EXECUTABLE     - Path to wix.exe (unified tool in WiX 6)
#   WIX_VERSION        - Version of WiX found
#   WIX_ROOT_DIR       - Root directory of WiX installation
#   WIX_HEAT_EXECUTABLE - Path to heat.exe (if available)
#
# Usage:
#   find_package(WiX)
#   if(WIX_FOUND)
#       # Use ${WIX_EXECUTABLE} for building
#   endif()

# Look for WiX v6 first, then fall back to v3 for backward compatibility
set(_WIX_SEARCH_PATHS
    "$ENV{WIX}/bin"
    "$ENV{ProgramFiles}/WiX Toolset v6.0/bin"
    "$ENV{ProgramFiles}/WiX Toolset v5.0/bin"
    "$ENV{ProgramFiles}/WiX Toolset v4.0/bin"
    "$ENV{ProgramFiles(x86)}/WiX Toolset v6.0/bin"
    "$ENV{ProgramFiles(x86)}/WiX Toolset v5.0/bin"
    "$ENV{ProgramFiles(x86)}/WiX Toolset v4.0/bin"
    # Legacy WiX v3 paths
    "$ENV{ProgramFiles}/WiX Toolset v3.11/bin"
    "$ENV{ProgramFiles(x86)}/WiX Toolset v3.11/bin"
    "$ENV{ProgramFiles}/Windows Installer XML v3.5/bin"
    "$ENV{ProgramFiles(x86)}/Windows Installer XML v3.5/bin"
)

# Find the main WiX executable
# WiX v6+ uses unified wix.exe, WiX v3 uses candle.exe + light.exe
find_program(WIX_EXECUTABLE
    NAMES wix.exe
    PATHS ${_WIX_SEARCH_PATHS}
    DOC "Path to WiX compiler executable (wix.exe)"
)

# If wix.exe not found, look for legacy candle.exe (WiX v3)
if(NOT WIX_EXECUTABLE)
    find_program(WIX_CANDLE_EXECUTABLE
        NAMES candle.exe
        PATHS ${_WIX_SEARCH_PATHS}
        DOC "Path to WiX compiler executable (candle.exe)"
    )
    
    find_program(WIX_LIGHT_EXECUTABLE
        NAMES light.exe
        PATHS ${_WIX_SEARCH_PATHS}
        DOC "Path to WiX linker executable (light.exe)"
    )
    
    if(WIX_CANDLE_EXECUTABLE AND WIX_LIGHT_EXECUTABLE)
        set(WIX_EXECUTABLE ${WIX_CANDLE_EXECUTABLE})
        set(WIX_LEGACY_TOOLS TRUE)
    endif()
endif()

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
    
    # Try to get version from wix.exe
    if(NOT WIX_LEGACY_TOOLS)
        execute_process(
            COMMAND "${WIX_EXECUTABLE}" --version
            OUTPUT_VARIABLE WIX_VERSION_OUTPUT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        if(WIX_VERSION_OUTPUT MATCHES "wix.exe version ([0-9]+\\.[0-9]+\\.[0-9]+)")
            set(WIX_VERSION "${CMAKE_MATCH_1}")
        endif()
    else()
        # Legacy WiX v3 - get version from candle.exe
        execute_process(
            COMMAND "${WIX_CANDLE_EXECUTABLE}" -nologo -help
            OUTPUT_VARIABLE WIX_VERSION_OUTPUT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        if(WIX_VERSION_OUTPUT MATCHES "version ([0-9]+\\.[0-9]+\\.[0-9]+)")
            set(WIX_VERSION "${CMAKE_MATCH_1}")
        endif()
    endif()
endif()

# Set additional variables for legacy compatibility
if(WIX_LEGACY_TOOLS)
    set(WIX_CANDLE_EXECUTABLE ${WIX_CANDLE_EXECUTABLE})
    set(WIX_LIGHT_EXECUTABLE ${WIX_LIGHT_EXECUTABLE})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WiX
    REQUIRED_VARS WIX_EXECUTABLE
    VERSION_VAR WIX_VERSION
    FAIL_MESSAGE "WiX Toolset not found. Please install WiX Toolset v6.0 or later from https://wixtoolset.org/"
)

mark_as_advanced(
    WIX_EXECUTABLE
    WIX_CANDLE_EXECUTABLE
    WIX_LIGHT_EXECUTABLE
    WIX_HEAT_EXECUTABLE
    WIX_ROOT_DIR
)

# Provide information about the found WiX version
if(WIX_FOUND)
    if(WIX_LEGACY_TOOLS)
        message(STATUS "Found WiX v3 (legacy): ${WIX_VERSION} at ${WIX_ROOT_DIR}")
        message(STATUS "  Candle: ${WIX_CANDLE_EXECUTABLE}")
        message(STATUS "  Light: ${WIX_LIGHT_EXECUTABLE}")
    else()
        message(STATUS "Found WiX v6+: ${WIX_VERSION} at ${WIX_ROOT_DIR}")
        message(STATUS "  WiX: ${WIX_EXECUTABLE}")
    endif()
    
    if(WIX_HEAT_EXECUTABLE)
        message(STATUS "  Heat: ${WIX_HEAT_EXECUTABLE}")
    endif()
endif()