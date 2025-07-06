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
# Platform-Specific Configuration
# ==========================================

# ==========================================
# Platform Detection
# ==========================================

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
    set(IS_32BIT TRUE)
endif()

# ==========================================
# MSVC-Specific Configuration
# ==========================================

function(configure_msvc)
    if(MSVC)
        if(MSVC_TOOLSET_VERSION LESS 141)
            message(FATAL_ERROR "Only Microsoft Visual Studio 17 and newer are supported!")
        endif()
        
        add_compile_options(/permissive- /utf-8 /MP)
        
        if(IS_DEBUG_BUILD)
            add_compile_options(/Zf)
            if(MSVC_TOOLSET_VERSION GREATER 141)
                add_compile_definitions(/fsanitize=address)
            endif()
        endif()
    endif()
endfunction()

# ==========================================
# Windows-Specific Configuration
# ==========================================

function(configure_windows)
    if(WIN32)
        set(CMAKE_RC_COMPILER_INIT windres PARENT_SCOPE)
        enable_language(RC)
        
        if(MINGW)
            set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>" PARENT_SCOPE)
        endif()
        
        # Security features for release builds
        if(NOT IS_DEBUG_BUILD)
            if(MSVC)
                # By default MSVC enables NXCOMPAT
                add_compile_options(/guard:cf)
                add_link_options(/DYNAMICBASE /HIGHENTROPYVA /GUARD:CF)
            else() # MINGW
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--nxcompat -Wl,--dynamicbase" PARENT_SCOPE)
                set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--nxcompat -Wl,--dynamicbase" PARENT_SCOPE)
                # Enable high entropy ASLR for 64-bit builds
                if(NOT IS_32BIT)
                    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--high-entropy-va" PARENT_SCOPE)
                    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--high-entropy-va" PARENT_SCOPE)
                endif()
            endif()
        endif()
    endif()
endfunction()

# ==========================================
# Unix/Linux-Specific Configuration
# ==========================================

function(configure_unix_linux)
    if(UNIX AND NOT APPLE)
        check_add_gcc_compiler_flag("-Qunused-arguments")
        
        # Linker flags for security and optimization
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -Wl,--no-undefined" PARENT_SCOPE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now -pie" PARENT_SCOPE)
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--as-needed" PARENT_SCOPE)
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,relro,-z,now" PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# macOS-Specific Configuration
# ==========================================

function(configure_macos)
    if(APPLE)
        set(CMAKE_MACOSX_RPATH TRUE PARENT_SCOPE)
        
        # Perform compiler support checks for macOS features
        try_compile(XC_APPLE_COMPILER_SUPPORT_BIOMETRY
           ${CMAKE_CURRENT_BINARY_DIR}/biometry_test/
           ${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler-checks/macos/control_biometry_support.mm)
        message(STATUS "Biometry compiler support: ${XC_APPLE_COMPILER_SUPPORT_BIOMETRY}")

        try_compile(XC_APPLE_COMPILER_SUPPORT_TOUCH_ID
           ${CMAKE_CURRENT_BINARY_DIR}/touch_id_test/
           ${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler-checks/macos/control_touch_id_support.mm)
        message(STATUS "Touch ID compiler support: ${XC_APPLE_COMPILER_SUPPORT_TOUCH_ID}")

        try_compile(XC_APPLE_COMPILER_SUPPORT_WATCH
           ${CMAKE_CURRENT_BINARY_DIR}/watch_test/
           ${CMAKE_CURRENT_SOURCE_DIR}/cmake/compiler-checks/macos/control_watch_support.mm)
        message(STATUS "Apple watch compiler support: ${XC_APPLE_COMPILER_SUPPORT_WATCH}")
        
        # Set global variables to make them available to parent scope
        set(XC_APPLE_COMPILER_SUPPORT_BIOMETRY ${XC_APPLE_COMPILER_SUPPORT_BIOMETRY} PARENT_SCOPE)
        set(XC_APPLE_COMPILER_SUPPORT_TOUCH_ID ${XC_APPLE_COMPILER_SUPPORT_TOUCH_ID} PARENT_SCOPE)
        set(XC_APPLE_COMPILER_SUPPORT_WATCH ${XC_APPLE_COMPILER_SUPPORT_WATCH} PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Address Sanitizer Configuration
# ==========================================

function(configure_address_sanitizer)
    if(WITH_ASAN)
        if(NOT (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR APPLE))
            message(FATAL_ERROR "WITH_ASAN is only supported on Linux / macOS at the moment.")
        endif()

        add_gcc_compiler_flags("-fsanitize=address -DWITH_ASAN")

        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            if(NOT (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9))
                add_gcc_compiler_flags("-fsanitize=leak -DWITH_LSAN")
            endif()
        endif()
    endif()
endfunction()

# ==========================================
# Install Directory Configuration
# ==========================================

function(configure_install_directories)
    if(WIN32)
        set(CLI_INSTALL_DIR "." PARENT_SCOPE)
        set(PROXY_INSTALL_DIR "." PARENT_SCOPE)
        set(BIN_INSTALL_DIR "." PARENT_SCOPE)
        set(PLUGIN_INSTALL_DIR "." PARENT_SCOPE)
        set(DATA_INSTALL_DIR "share" PARENT_SCOPE)
    elseif(APPLE AND WITH_APP_BUNDLE)
        set(BUNDLE_INSTALL_DIR "${PROGNAME}.app/Contents" PARENT_SCOPE)
        set(CMAKE_INSTALL_MANDIR "${BUNDLE_INSTALL_DIR}/Resources/man" PARENT_SCOPE)
        set(CLI_INSTALL_DIR "${BUNDLE_INSTALL_DIR}/MacOS" PARENT_SCOPE)
        set(PROXY_INSTALL_DIR "${BUNDLE_INSTALL_DIR}/MacOS" PARENT_SCOPE)
        set(BIN_INSTALL_DIR "${BUNDLE_INSTALL_DIR}/MacOS" PARENT_SCOPE)
        set(PLUGIN_INSTALL_DIR "${BUNDLE_INSTALL_DIR}/PlugIns" PARENT_SCOPE)
        set(DATA_INSTALL_DIR "${BUNDLE_INSTALL_DIR}/Resources" PARENT_SCOPE)
    else()
        include(GNUInstallDirs)
        
        set(CLI_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}" PARENT_SCOPE)
        set(PROXY_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}" PARENT_SCOPE)
        set(BIN_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}" PARENT_SCOPE)
        set(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/keepassxc" PARENT_SCOPE)
        set(DATA_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/keepassxc" PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Program Name Configuration
# ==========================================

function(configure_program_name)
    if(APPLE AND WITH_APP_BUNDLE OR WIN32)
        set(PROGNAME KeePassXC PARENT_SCOPE)
    else()
        set(PROGNAME keepassxc PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Position Independent Code Configuration
# ==========================================

function(configure_position_independent_code)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON PARENT_SCOPE)
    
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14.0")
        cmake_policy(SET CMP0083 NEW)
        include(CheckPIESupported)
        check_pie_supported()
    endif()
endfunction()

# ==========================================
# Main Platform Configuration Function
# ==========================================

function(configure_platform)
    message(STATUS "Configuring platform-specific settings...")
    
    configure_program_name()
    configure_position_independent_code()
    configure_install_directories()
    configure_macos()
    configure_msvc()
    configure_windows()
    configure_unix_linux()
    configure_address_sanitizer()
    
    message(STATUS "Platform configuration complete")
endfunction()