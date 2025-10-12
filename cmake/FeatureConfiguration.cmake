#  Copyright (C) 2023 KeePassXC Team <team@keepassxc.org>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 or (at your option)
#  version 3 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

# ==========================================
# Feature Configuration and Options
# ==========================================

# ==========================================
# Build Options
# ==========================================

function(define_build_options)
    option(WITH_TESTS "Enable building of unit tests" ON)
    option(WITH_GUI_TESTS "Enable building of GUI tests" OFF)
    option(WITH_DEV_BUILD "Use only for development. Disables/warns about deprecated methods." OFF)
    option(WITH_ASAN "Enable address sanitizer checks (Linux / macOS only)" OFF)
    option(WITH_COVERAGE "Use to build with coverage tests (GCC only)." OFF)
    option(WITH_APP_BUNDLE "Enable Application Bundle for macOS" ON)
    option(WITH_CCACHE "Use ccache for build" OFF)
    
    # Feature options
    set(WITH_XC_ALL OFF CACHE BOOL "Build in all available plugins")
    
    option(WITH_XC_AUTOTYPE "Include Auto-Type." ON)
    option(WITH_XC_NETWORKING "Include networking code (e.g. for downloading website icons)." OFF)
    option(WITH_XC_BROWSER "Include browser integration with keepassxc-browser." OFF)
    option(WITH_XC_BROWSER_PASSKEYS "Passkeys support for browser integration." OFF)
    option(WITH_XC_YUBIKEY "Include YubiKey support." OFF)
    option(WITH_XC_SSHAGENT "Include SSH agent support." OFF)
    option(WITH_XC_KEESHARE "Sharing integration with KeeShare" OFF)
    option(WITH_XC_UPDATECHECK "Include automatic update checks; disable for controlled distributions" ON)
    
    if(UNIX AND NOT APPLE)
        option(WITH_XC_FDOSECRETS "Implement freedesktop.org Secret Storage Spec server side API." OFF)
    endif()
    
    option(WITH_XC_DOCS "Enable building of documentation" ON)
    set(WITH_XC_X11 ON CACHE BOOL "Enable building with X11 deps")
endfunction()

# ==========================================
# Feature Dependencies and Logic
# ==========================================

function(configure_feature_dependencies)
    # Enable all options if WITH_XC_ALL is requested
    if(WITH_XC_ALL)
        set(WITH_XC_AUTOTYPE ON PARENT_SCOPE)
        set(WITH_XC_NETWORKING ON PARENT_SCOPE)
        set(WITH_XC_BROWSER ON PARENT_SCOPE)
        set(WITH_XC_BROWSER_PASSKEYS ON PARENT_SCOPE)
        set(WITH_XC_YUBIKEY ON PARENT_SCOPE)
        set(WITH_XC_SSHAGENT ON PARENT_SCOPE)
        set(WITH_XC_KEESHARE ON PARENT_SCOPE)
        if(UNIX AND NOT APPLE)
            set(WITH_XC_FDOSECRETS ON PARENT_SCOPE)
        endif()
    endif()

    # Handle feature dependencies
    if(NOT WITH_XC_NETWORKING AND WITH_XC_UPDATECHECK)
        message(STATUS "Disabling WITH_XC_UPDATECHECK because WITH_XC_NETWORKING is disabled")
        set(WITH_XC_UPDATECHECK OFF PARENT_SCOPE)
    endif()

    if(UNIX AND NOT APPLE AND NOT WITH_XC_X11)
        message(STATUS "Disabling WITH_XC_AUTOTYPE because WITH_XC_X11 is disabled")
        set(WITH_XC_AUTOTYPE OFF PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Build Type Configuration
# ==========================================

function(configure_build_type)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
                "Choose the type of build, options are: Debug Release RelWithDebInfo Profile"
                FORCE)
    endif()
    
    string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
    if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug" OR CMAKE_BUILD_TYPE_LOWER STREQUAL "relwithdebinfo")
        set(IS_DEBUG_BUILD TRUE PARENT_SCOPE)
    endif()
    
    # Set the build type variable in parent scope
    set(CMAKE_BUILD_TYPE_LOWER ${CMAKE_BUILD_TYPE_LOWER} PARENT_SCOPE)
    
    # Debian sets the build type to None for package builds.
    # Make sure we don't enable asserts there.
    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS_NONE QT_NO_DEBUG)
endfunction()

# ==========================================
# ccache Configuration
# ==========================================

function(configure_ccache)
    if(WITH_CCACHE)
        find_program(CCACHE_FOUND ccache)
        if(NOT CCACHE_FOUND)
            message(FATAL_ERROR "ccache requested but cannot be found.")
        endif()
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_FOUND})
    endif()
endfunction()

# ==========================================
# Qt-specific Definitions
# ==========================================

function(configure_qt_definitions)
    add_definitions(-DQT_NO_EXCEPTIONS -DQT_STRICT_ITERATORS -DQT_NO_CAST_TO_ASCII)
    if(NOT IS_DEBUG_BUILD)
        add_definitions(-DQT_NO_DEBUG_OUTPUT)
    endif()

    if(WITH_APP_BUNDLE)
        add_definitions(-DWITH_APP_BUNDLE)
    endif()
    
    add_definitions(-DQT_TEST_LIB)
endfunction()

# ==========================================
# Test Configuration
# ==========================================

function(configure_testing)
    if(WITH_TESTS)
        enable_testing()
    endif()
endfunction()

# ==========================================
# Feature Summary Configuration
# ==========================================

function(configure_feature_summary)
    include(FeatureSummary)
    
    add_feature_info(Auto-Type WITH_XC_AUTOTYPE "Automatic password typing")
    add_feature_info(Networking WITH_XC_NETWORKING "Compile KeePassXC with network access code (e.g. for downloading website icons)")
    add_feature_info(KeePassXC-Browser WITH_XC_BROWSER "Browser integration with KeePassXC-Browser")
    add_feature_info(Passkeys WITH_XC_BROWSER_PASSKEYS "Passkeys support for browser integration")
    add_feature_info(SSHAgent WITH_XC_SSHAGENT "SSH agent integration compatible with KeeAgent")
    add_feature_info(KeeShare WITH_XC_KEESHARE "Sharing integration with KeeShare")
    add_feature_info(YubiKey WITH_XC_YUBIKEY "YubiKey HMAC-SHA1 challenge-response")
    add_feature_info(UpdateCheck WITH_XC_UPDATECHECK "Automatic update checking")
    if(UNIX AND NOT APPLE)
        add_feature_info(FdoSecrets WITH_XC_FDOSECRETS "Implement freedesktop.org Secret Storage Spec server side API.")
    endif()
endfunction()

# ==========================================
# Main Feature Configuration Function
# ==========================================

function(configure_features)
    message(STATUS "Configuring features and build options...")
    
    define_build_options()
    configure_build_type()
    configure_feature_dependencies()
    configure_ccache()
    configure_qt_definitions()
    configure_testing()
    configure_feature_summary()
    
    message(STATUS "Feature configuration complete")
endfunction()