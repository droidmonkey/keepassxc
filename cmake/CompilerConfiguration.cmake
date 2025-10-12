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
# Compiler Detection and Configuration
# ==========================================

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)

# Detect Clang compiler variants
set(CLANG_COMPILER_ID_REGEX "^(Apple)?[Cc]lang$")
if("${CMAKE_C_COMPILER}" MATCHES "clang$"
        OR "${CMAKE_EXTRA_GENERATOR_C_SYSTEM_DEFINED_MACROS}" MATCHES "__clang__"
        OR "${CMAKE_C_COMPILER_ID}" MATCHES ${CLANG_COMPILER_ID_REGEX})
    set(CMAKE_COMPILER_IS_CLANG 1)
endif()

if("${CMAKE_CXX_COMPILER}" MATCHES "clang(\\+\\+)?$"
        OR "${CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_DEFINED_MACROS}" MATCHES "__clang__"
        OR "${CMAKE_CXX_COMPILER_ID}" MATCHES ${CLANG_COMPILER_ID_REGEX})
    set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()

# ==========================================
# Compiler Flag Utility Macros
# ==========================================

macro(add_gcc_compiler_cxxflags FLAGS)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
    endif()
endmacro(add_gcc_compiler_cxxflags)

macro(add_gcc_compiler_cflags FLAGS)
    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
    endif()
endmacro(add_gcc_compiler_cflags)

macro(add_gcc_compiler_flags FLAGS)
    add_gcc_compiler_cxxflags("${FLAGS}")
    add_gcc_compiler_cflags("${FLAGS}")
endmacro(add_gcc_compiler_flags)

# Macros that check compiler flag support before adding
macro(check_add_gcc_compiler_cxxflag FLAG FLAGNAME)
    check_cxx_compiler_flag("${FLAG}" CXX_HAS${FLAGNAME})
    if(CXX_HAS${FLAGNAME})
        add_gcc_compiler_cxxflags("${FLAG}")
    endif()
endmacro(check_add_gcc_compiler_cxxflag)

macro(check_add_gcc_compiler_cflag FLAG FLAGNAME)
    check_c_compiler_flag("${FLAG}" CC_HAS${FLAGNAME})
    if(CC_HAS${FLAGNAME})
        add_gcc_compiler_cflags("${FLAG}")
    endif()
endmacro(check_add_gcc_compiler_cflag)

# Front-end macro for checking compiler flags
macro(check_add_gcc_compiler_flag FLAG)
    string(REGEX REPLACE "[-=]" "_" FLAGNAME "${FLAG}")
    set(check_lang_spec ${ARGN})
    list(LENGTH check_lang_spec num_extra_args)
    set(langs C CXX)
    if(num_extra_args GREATER 0)
        set(langs "${check_lang_spec}")
    endif()
    if("C" IN_LIST langs)
        check_add_gcc_compiler_cflag("${FLAG}" "${FLAGNAME}")
    endif()
    if("CXX" IN_LIST langs)
        check_add_gcc_compiler_cxxflag("${FLAG}" "${FLAGNAME}")
    endif()
endmacro(check_add_gcc_compiler_flag)

# ==========================================
# Language Standards Configuration
# ==========================================

function(configure_language_standards)
    set(CMAKE_C_STANDARD 99 PARENT_SCOPE)
    if(WITH_XC_BOTAN3)
        set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE)
    else()
        set(CMAKE_CXX_STANDARD 17 PARENT_SCOPE)
    endif()
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
endfunction()

# ==========================================
# Common Compiler Flags
# ==========================================

function(configure_common_compiler_flags)
    # Basic flags for all GCC-compatible compilers
    add_gcc_compiler_flags("-fno-common")
    add_gcc_compiler_flags("-Wall -Wextra -Wundef -Wpointer-arith -Wno-long-long")
    add_gcc_compiler_flags("-Wformat=2 -Wmissing-format-attribute")
    add_gcc_compiler_flags("-fvisibility=hidden")
    add_gcc_compiler_cxxflags("-fvisibility-inlines-hidden")

    # Additional C++ specific warnings
    add_gcc_compiler_cxxflags("-Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual")
    add_gcc_compiler_cflags("-Wchar-subscripts -Wwrite-strings")

    # Security-related flags
    check_add_gcc_compiler_flag("-Werror=format-security")
    check_add_gcc_compiler_flag("-Werror=implicit-function-declaration" C)
    check_add_gcc_compiler_flag("-Wcast-align")
endfunction()

# ==========================================
# Debug Build Specific Flags
# ==========================================

function(configure_debug_build_flags)
    if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
        check_add_gcc_compiler_flag("-Wshadow-compatible-local")
        check_add_gcc_compiler_flag("-Wshadow-local")
        add_gcc_compiler_flags("-Werror")
        # This is needed since compiling against Botan3 requires compiling against C++20
        if(WITH_XC_BOTAN3)
            add_gcc_compiler_cxxflags("-Wno-error=deprecated-enum-enum-conversion -Wno-error=deprecated")
        endif()
    endif()
endfunction()

# ==========================================
# Stack Protection Configuration
# ==========================================

function(configure_stack_protection)
    if(NOT HAIKU)
        if((CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8.999) OR CMAKE_COMPILER_IS_CLANGXX)
            add_gcc_compiler_flags("-fstack-protector-strong")
        else()
            add_gcc_compiler_flags("-fstack-protector --param=ssp-buffer-size=4")
        endif()
    endif()
endfunction()

# ==========================================
# Fortify Source Configuration
# ==========================================

function(configure_fortify_source)
    if(CMAKE_BUILD_TYPE_LOWER MATCHES "(release|relwithdebinfo|minsizerel)")
        add_gcc_compiler_flags("-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2")
    endif()
endfunction()

# ==========================================
# Sized Deallocation Support
# ==========================================

function(configure_sized_deallocation)
    check_cxx_compiler_flag("-fsized-deallocation" CXX_HAS_fsized_deallocation)
    if(CXX_HAS_fsized_deallocation)
        # Do additional check: the deallocation functions must be there too.
        set(CMAKE_REQUIRED_FLAGS "-fsized-deallocation")
        check_cxx_source_compiles("#include <new>
            int main() { void * ptr = nullptr; std::size_t size = 1; ::operator delete(ptr, size); }"
            HAVE_DEALLOCATION_FUNCTIONS)
        if(HAVE_DEALLOCATION_FUNCTIONS)
            check_add_gcc_compiler_flag("-fsized-deallocation" CXX)
        endif()
        unset(CMAKE_REQUIRED_FLAGS)
    endif()
endfunction()

# ==========================================
# OpenMP Configuration
# ==========================================

function(configure_openmp)
    find_package(OpenMP)
    if(OpenMP_FOUND)
        add_gcc_compiler_cflags(${OpenMP_C_FLAGS})
        add_gcc_compiler_cxxflags(${OpenMP_CXX_FLAGS})
    endif()
endfunction()

# ==========================================
# Apple-Specific Compiler Configuration
# ==========================================

function(configure_apple_compiler_flags)
    if(APPLE AND CMAKE_COMPILER_IS_CLANGXX)
        add_gcc_compiler_cxxflags("-stdlib=libc++")
    endif()
endfunction()

# ==========================================
# Development Build Configuration
# ==========================================

function(configure_development_build_flags)
    if(WITH_DEV_BUILD)
        add_definitions(-DQT_DEPRECATED_WARNINGS)
    else()
        add_definitions(-DQT_NO_DEPRECATED_WARNINGS)
        add_gcc_compiler_cxxflags("-Wno-deprecated-declarations")
    endif()
endfunction()

# ==========================================
# Main Configuration Function
# ==========================================

function(configure_compiler)
    message(STATUS "Configuring compiler settings...")
    
    configure_language_standards()
    configure_common_compiler_flags()
    configure_debug_build_flags()
    configure_stack_protection()
    configure_fortify_source()
    configure_sized_deallocation()
    configure_openmp()
    configure_apple_compiler_flags()
    configure_development_build_flags()
    
    message(STATUS "Compiler configuration complete")
endfunction()