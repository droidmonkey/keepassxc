#  Copyright (C) 2023 KeePassXC Team <team@keepassxc.org>
#  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
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
# Test Utilities and Macros
# ==========================================

# ==========================================
# Argument Parsing Utility
# ==========================================

macro(parse_arguments prefix arg_names option_names)
    set(DEFAULT_ARGS)
    foreach(arg_name ${arg_names})
        set(${prefix}_${arg_name})
    endforeach(arg_name)
    foreach(option ${option_names})
        set(${prefix}_${option} FALSE)
    endforeach(option)

    set(current_arg_name DEFAULT_ARGS)
    set(current_arg_list)
    foreach(arg ${ARGN})
        set(larg_names ${arg_names})
        list(FIND larg_names "${arg}" is_arg_name)
        if(is_arg_name GREATER -1)
            set(${prefix}_${current_arg_name} ${current_arg_list})
            set(current_arg_name ${arg})
            set(current_arg_list)
        else()
            set(loption_names ${option_names})
            list(FIND loption_names "${arg}" is_option)
            if(is_option GREATER -1)
                set(${prefix}_${arg} TRUE)
            else(is_option GREATER -1)
                set(current_arg_list ${current_arg_list} ${arg})
            endif()
        endif()
    endforeach(arg)
    set(${prefix}_${current_arg_name} ${current_arg_list})
endmacro(parse_arguments)

# ==========================================
# Unit Test Creation Macro
# ==========================================

macro(add_unit_test)
    parse_arguments(TEST "NAME;SOURCES;LIBS;LAUNCHER" "" ${ARGN})
    set(_test_NAME ${TEST_NAME})
    set(_test_LAUNCHER ${TEST_LAUNCHER})
    set(_srcList ${TEST_SOURCES})
    
    add_executable(${_test_NAME} ${_srcList})
    target_link_libraries(${_test_NAME} ${TEST_LIBS})

    if(NOT TEST_OUTPUT)
        set(TEST_OUTPUT plaintext)
    endif(NOT TEST_OUTPUT)
    set(TEST_OUTPUT ${TEST_OUTPUT} CACHE STRING "The output to generate when running the QTest unit tests")

    if(KDE4_TEST_OUTPUT STREQUAL "xml")
        add_test(${_test_NAME} ${_test_LAUNCHER} ${_test_NAME} -xml -o ${_test_NAME}.tml)
    else(KDE4_TEST_OUTPUT STREQUAL "xml")
        add_test(${_test_NAME} ${_test_LAUNCHER} ${_test_NAME})
    endif(KDE4_TEST_OUTPUT STREQUAL "xml")

    set_tests_properties(${_test_NAME} PROPERTIES ENVIRONMENT "LANG=en_US.UTF-8")

    if(NOT MSVC_IDE)   #not needed for the ide
        # if the tests are EXCLUDE_FROM_ALL, add a target "buildtests" to build all tests
        if(NOT WITH_TESTS)
            get_directory_property(_buildtestsAdded BUILDTESTS_ADDED)
            if(NOT _buildtestsAdded)
                add_custom_target(buildtests)
                set_directory_properties(PROPERTIES BUILDTESTS_ADDED TRUE)
            endif()
            add_dependencies(buildtests ${_test_NAME})
        endif()
    endif()
endmacro(add_unit_test)

# ==========================================
# Test Configuration Setup
# ==========================================

function(setup_test_configuration)
    # Test data directory configuration
    set(KEEPASSX_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data PARENT_SCOPE)
    
    # Configure test header file
    configure_file(config-keepassx-tests.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-keepassx-tests.h)
    
    # Test include directories
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
    
    # Test-specific definitions
    add_definitions(-DQT_TEST_LIB)
    
    message(STATUS "Test configuration setup complete")
endfunction()

# ==========================================
# Test Support Library Creation
# ==========================================

function(create_test_support_library)
    set(testsupport_SOURCES
        modeltest.cpp
        FailDevice.cpp
        mock/MockClock.cpp
        util/TemporaryFile.cpp
    )
    
    add_library(testsupport STATIC ${testsupport_SOURCES})
    target_link_libraries(testsupport Qt5::Core Qt5::Concurrent Qt5::Widgets Qt5::Test)
    
    message(STATUS "Test support library created")
endfunction()

# ==========================================
# Standard Test Libraries Definition
# ==========================================

function(define_standard_test_libraries)
    set(TEST_LIBRARIES keepassxc_gui Qt5::Test PARENT_SCOPE)
    message(STATUS "Standard test libraries defined")
endfunction()

# ==========================================
# Feature-Based Test Creation Helpers
# ==========================================

function(add_feature_tests feature_name test_list)
    if(${feature_name})
        foreach(test_info ${test_list})
            # Parse test information - format: "name;sources;additional_libs"
            string(REPLACE ";" "\\;" test_parts "${test_info}")
            list(GET test_parts 0 test_name)
            list(GET test_parts 1 test_sources)
            if(list(LENGTH test_parts) GREATER 2)
                list(GET test_parts 2 additional_libs)
                add_unit_test(NAME ${test_name} SOURCES ${test_sources} LIBS ${additional_libs} ${TEST_LIBRARIES})
            else()
                add_unit_test(NAME ${test_name} SOURCES ${test_sources} LIBS ${TEST_LIBRARIES})
            endif()
        endforeach()
    endif()
endfunction()

# ==========================================
# Test Target Properties Helper
# ==========================================

function(set_test_target_properties test_name)
    # Add any common test target properties here
    # For example, setting specific compile definitions for tests
    if(TARGET ${test_name})
        # Enable exports if needed (for some AutoType tests)
        if(test_name MATCHES "autotype")
            set_target_properties(${test_name} PROPERTIES ENABLE_EXPORTS ON)
        endif()
        
        # Add test-specific compile definitions if needed
        if(test_name MATCHES "cli")
            target_compile_definitions(${test_name} PRIVATE KEEPASSX_CLI_PATH="$<TARGET_FILE:keepassxc-cli>")
        endif()
    endif()
endfunction()

# ==========================================
# Main Test Utilities Configuration Function
# ==========================================

function(configure_test_utilities)
    message(STATUS "Configuring test utilities...")
    
    setup_test_configuration()
    define_standard_test_libraries()
    create_test_support_library()
    
    message(STATUS "Test utilities configuration complete")
endfunction()