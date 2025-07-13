# WiXCompat.cmake - WiX 6/3 Compatibility Detection for CPack
#
# This module detects available WiX version and configures CPack accordingly.
# CPack's WIX generator currently requires WiX v3 tools (candle/light),
# but this module prepares for future WiX v6 support.
#
# Usage: include(WiXCompat) before include(CPack) in Windows builds

if(WIN32 AND "WIX" IN_LIST CPACK_GENERATOR)
    # Detect WiX installation
    find_package(WiX QUIET)
    
    if(WIX_FOUND AND NOT WIX_LEGACY_TOOLS)
        # WiX v6 found but CPack doesn't support it yet
        message(WARNING 
            "Found WiX v6 at ${WIX_ROOT_DIR}, but CPack's WIX generator currently requires WiX v3.\n"
            "Please install WiX v3.x for Windows installer generation, or wait for CMake/CPack to support WiX v6.\n"
            "WiX v3 can be downloaded from: https://github.com/wixtoolset/wix3/releases\n"
            "This project's .wxs files have been updated for WiX v6 compatibility.")
        
        # Set extension name for future WiX v6 support
        if(NOT CPACK_WIX_EXTENSIONS)
            set(CPACK_WIX_EXTENSIONS "WixToolset.Util.wixext")
        endif()
        
    elseif(WIX_FOUND AND WIX_LEGACY_TOOLS)
        # WiX v3 found - use directly with CPack
        message(STATUS "Found WiX v3 - configuring CPack for Windows installer generation")
        set(CPACK_WIX_ROOT "${WIX_ROOT_DIR}")
        
        # Use legacy extension format for WiX v3
        if(NOT CPACK_WIX_EXTENSIONS)
            set(CPACK_WIX_EXTENSIONS "WixUtilExtension.dll")
        endif()
        
    else()
        # No WiX found
        message(WARNING 
            "WiX Toolset not found. Windows installer generation will be disabled.\n"
            "Install WiX v3.x from: https://github.com/wixtoolset/wix3/releases\n"
            "Or WiX v6 from: https://wixtoolset.org/ (requires future CPack support)")
        
        # Remove WIX from generators to avoid CPack errors
        list(REMOVE_ITEM CPACK_GENERATOR "WIX")
        if(NOT CPACK_GENERATOR)
            set(CPACK_GENERATOR "ZIP")
        endif()
    endif()
    
    # Display configuration summary
    if(WIX_FOUND)
        message(STATUS "WiX Configuration:")
        message(STATUS "  Version: ${WIX_VERSION}")
        message(STATUS "  Tools: ${WIX_LEGACY_TOOLS}")
        if(WIX_LEGACY_TOOLS)
            message(STATUS "  Type: Legacy (v3)")
        else()
            message(STATUS "  Type: Modern (v6+)")
        endif()
        message(STATUS "  Extensions: ${CPACK_WIX_EXTENSIONS}")
        message(STATUS "  Templates: WiX v6 schema with v3 compatibility")
    endif()
endif()