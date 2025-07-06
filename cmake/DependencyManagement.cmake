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
# External Dependencies Management
# ==========================================

include(CheckCXXSourceCompiles)

# ==========================================
# Botan Cryptographic Library
# ==========================================

function(find_and_configure_botan)
    message(STATUS "Finding Botan cryptographic library...")
    
    find_package(Botan REQUIRED)
    if(BOTAN_VERSION VERSION_GREATER_EQUAL "3.0.0")
        set(WITH_XC_BOTAN3 TRUE PARENT_SCOPE)
    elseif(BOTAN_VERSION VERSION_LESS "2.11.0")
        message(FATAL_ERROR "Botan 2.11.0 or higher is required")   
    endif()
    
    include_directories(SYSTEM ${BOTAN_INCLUDE_DIR})
    message(STATUS "Found Botan: ${BOTAN_LIBRARIES} (version ${BOTAN_VERSION})")
endfunction()

# ==========================================
# Qt5 Framework
# ==========================================

function(find_and_configure_qt5)
    message(STATUS "Finding Qt5 framework...")
    
    set(QT_COMPONENTS Core Network Concurrent Gui Svg Widgets Test LinguistTools)
    
    if(UNIX AND NOT APPLE)
        if(WITH_XC_X11)
            list(APPEND QT_COMPONENTS X11Extras)
        endif()
        find_package(Qt5 COMPONENTS ${QT_COMPONENTS} DBus REQUIRED)
    elseif(APPLE)
        find_package(Qt5 COMPONENTS ${QT_COMPONENTS} REQUIRED HINTS
                /usr/local/opt/qt@5/lib/cmake
                /usr/local/Cellar/qt@5/*/lib/cmake
                /opt/homebrew/opt/qt@5/lib/cmake
                ENV PATH)
        find_package(Qt5 COMPONENTS MacExtras HINTS
                /usr/local/opt/qt@5/lib/cmake
                /usr/local/Cellar/qt@5/*/lib/cmake
                /opt/homebrew/opt/qt@5/lib/cmake
                ENV PATH)
    else()
        find_package(Qt5 COMPONENTS ${QT_COMPONENTS} REQUIRED)
    endif()

    if(Qt5Core_VERSION VERSION_LESS "5.12.0")
        message(FATAL_ERROR "Qt version 5.12.0 or higher is required")
    endif()

    get_filename_component(Qt5_PREFIX ${Qt5_DIR}/../../.. REALPATH)
    if(APPLE)
        # Add includes under Qt5 Prefix in case Qt6 is also installed
        include_directories(SYSTEM ${Qt5_PREFIX}/include)
    endif()

    # Configure Qt5 automation
    set(CMAKE_AUTOMOC ON PARENT_SCOPE)
    set(CMAKE_AUTOUIC ON PARENT_SCOPE)
    set(CMAKE_AUTORCC ON PARENT_SCOPE)
    
    # Store Qt5_PREFIX for use in other modules
    set(Qt5_PREFIX ${Qt5_PREFIX} PARENT_SCOPE)
    
    message(STATUS "Found Qt5: ${Qt5Core_VERSION}")
endfunction()

# ==========================================
# Qt5 Deployment Tools
# ==========================================

function(find_qt5_deployment_tools)
    if(APPLE)
        find_program(MACDEPLOYQT_EXE macdeployqt HINTS ${Qt5_PREFIX}/bin ${Qt5_PREFIX}/tools/qt5/bin ENV PATH)
        if(NOT MACDEPLOYQT_EXE)
            message(FATAL_ERROR "macdeployqt is required to build on macOS")
        endif()
        message(STATUS "Using macdeployqt: ${MACDEPLOYQT_EXE}")
        set(MACDEPLOYQT_EXE ${MACDEPLOYQT_EXE} PARENT_SCOPE)
        set(MACDEPLOYQT_EXTRA_BINARIES "" PARENT_SCOPE)
    elseif(WIN32)
        find_program(WINDEPLOYQT_EXE windeployqt HINTS ${Qt5_PREFIX}/bin ${Qt5_PREFIX}/tools/qt5/bin ENV PATH)
        if(NOT WINDEPLOYQT_EXE)
            message(FATAL_ERROR "windeployqt is required to build on Windows")
        endif()
        message(STATUS "Using windeployqt: ${WINDEPLOYQT_EXE}")
        set(WINDEPLOYQT_EXE ${WINDEPLOYQT_EXE} PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Compression Libraries
# ==========================================

function(find_compression_libraries)
    message(STATUS "Finding compression libraries...")
    
    # Find zlib
    find_package(ZLIB REQUIRED)
    if(ZLIB_VERSION_STRING VERSION_LESS "1.2.0")
        message(FATAL_ERROR "zlib 1.2.0 or higher is required to use the gzip format")
    endif()
    include_directories(SYSTEM ${ZLIB_INCLUDE_DIR})
    
    # Find Minizip
    find_package(Minizip REQUIRED)
    
    message(STATUS "Found ZLIB: ${ZLIB_LIBRARIES} (version ${ZLIB_VERSION_STRING})")
    message(STATUS "Found Minizip: ${MINIZIP_LIBRARIES}")
endfunction()

# ==========================================
# Argon2 Password Hashing Library
# ==========================================

function(find_argon2_library)
    message(STATUS "Finding Argon2 library...")
    
    find_library(ARGON2_LIBRARIES NAMES argon2)
    find_path(ARGON2_INCLUDE_DIR NAMES argon2.h PATH_SUFFIXES local/include)
    include_directories(SYSTEM ${ARGON2_INCLUDE_DIR})
    
    if(ARGON2_LIBRARIES)
        message(STATUS "Found Argon2: ${ARGON2_LIBRARIES}")
    else()
        message(STATUS "Argon2 library not found, will use Botan implementation")
    endif()
    
    set(ARGON2_LIBRARIES ${ARGON2_LIBRARIES} PARENT_SCOPE)
endfunction()

# ==========================================
# ZXCVBN Password Strength Library
# ==========================================

function(find_zxcvbn_library)
    message(STATUS "Finding ZXCVBN library...")
    
    find_library(ZXCVBN_LIBRARIES zxcvbn)
    if(NOT ZXCVBN_LIBRARIES)
        message(STATUS "ZXCVBN library not found, will build from source")
        add_subdirectory(src/thirdparty/zxcvbn)
        set(ZXCVBN_LIBRARIES zxcvbn PARENT_SCOPE)
    else()
        message(STATUS "Found ZXCVBN: ${ZXCVBN_LIBRARIES}")
        set(ZXCVBN_LIBRARIES ${ZXCVBN_LIBRARIES} PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# YubiKey Support Libraries
# ==========================================

function(find_yubikey_libraries)
    if(WITH_XC_YUBIKEY)
        message(STATUS "Finding YubiKey support libraries...")
        
        find_package(PCSC REQUIRED)
        include_directories(SYSTEM ${PCSC_INCLUDE_DIRS})

        if(UNIX AND NOT APPLE)
            find_library(LIBUSB_LIBRARIES NAMES usb-1.0 REQUIRED)
            find_path(LIBUSB_INCLUDE_DIR NAMES libusb.h PATH_SUFFIXES "libusb-1.0" "libusb" REQUIRED)
            include_directories(SYSTEM ${LIBUSB_INCLUDE_DIR})
            set(LIBUSB_LIBRARIES ${LIBUSB_LIBRARIES} PARENT_SCOPE)
        endif()
        
        message(STATUS "Found PCSC: ${PCSC_LIBRARIES}")
    endif()
endfunction()

# ==========================================
# Unix System Capability Checks
# ==========================================

function(check_unix_system_capabilities)
    if(UNIX)
        message(STATUS "Checking Unix system capabilities...")
        
        check_cxx_source_compiles("#include <sys/prctl.h>
        int main() { prctl(PR_SET_DUMPABLE, 0); return 0; }"
                HAVE_PR_SET_DUMPABLE)

        check_cxx_source_compiles("#include <malloc.h>
        int main() { return 0; }"
                HAVE_MALLOC_H)

        check_cxx_source_compiles("#include <malloc.h>
        int main() { malloc_usable_size(NULL); return 0; }"
                HAVE_MALLOC_USABLE_SIZE)

        check_cxx_source_compiles("#include <sys/resource.h>
        int main() {
          struct rlimit limit;
          limit.rlim_cur = 0;
          limit.rlim_max = 0;
          setrlimit(RLIMIT_CORE, &limit);
          return 0;
        }" HAVE_RLIMIT_CORE)

        if(APPLE)
            check_cxx_source_compiles("#include <sys/types.h>
          #include <sys/ptrace.h>
          int main() { ptrace(PT_DENY_ATTACH, 0, 0, 0); return 0; }"
                    HAVE_PT_DENY_ATTACH)
        endif()
        
        # Set variables in parent scope
        set(HAVE_PR_SET_DUMPABLE ${HAVE_PR_SET_DUMPABLE} PARENT_SCOPE)
        set(HAVE_MALLOC_H ${HAVE_MALLOC_H} PARENT_SCOPE)
        set(HAVE_MALLOC_USABLE_SIZE ${HAVE_MALLOC_USABLE_SIZE} PARENT_SCOPE)
        set(HAVE_RLIMIT_CORE ${HAVE_RLIMIT_CORE} PARENT_SCOPE)
        set(HAVE_PT_DENY_ATTACH ${HAVE_PT_DENY_ATTACH} PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Linux-Specific Libraries
# ==========================================

function(find_linux_libraries)
    if(UNIX AND NOT APPLE)
        find_library(KEYUTILS_LIBRARIES NAMES keyutils)
        if(NOT KEYUTILS_LIBRARIES)
            message(FATAL_ERROR "Could not find libkeyutils")
        endif()
        
        set(KEYUTILS_LIBRARIES ${KEYUTILS_LIBRARIES} PARENT_SCOPE)
        message(STATUS "Found keyutils: ${KEYUTILS_LIBRARIES}")
    endif()
endfunction()

# ==========================================
# X11 Libraries
# ==========================================

function(find_x11_libraries)
    if(UNIX AND NOT APPLE AND WITH_XC_X11)
        message(STATUS "Finding X11 libraries...")
        
        find_package(X11 REQUIRED)
        if(NOT X11_Xi_LIB OR NOT X11_Xtst_LIB)
            message(FATAL_ERROR "Xi and XTest libraries are required for Auto-Type on X11")
        endif()
        
        message(STATUS "Found X11: ${X11_LIBRARIES}")
    endif()
endfunction()

# ==========================================
# Main Dependency Configuration Function
# ==========================================

function(configure_dependencies)
    message(STATUS "Configuring external dependencies...")
    
    find_and_configure_botan()
    find_and_configure_qt5()
    find_qt5_deployment_tools()
    find_compression_libraries()
    find_argon2_library()
    find_zxcvbn_library()
    find_yubikey_libraries()
    check_unix_system_capabilities()
    find_linux_libraries()
    find_x11_libraries()
    
    message(STATUS "Dependencies configuration complete")
endfunction()