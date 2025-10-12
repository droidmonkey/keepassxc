#  Copyright (C) 2018 KeePassXC Team <team@keepassxc.org>
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

cmake_minimum_required(VERSION 3.10.0)

project(KeePassXC)
set(APP_ID "org.keepassxc.${PROJECT_NAME}")

# ==========================================
# CMake Module Path
# ==========================================

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# ==========================================
# Load Modular Configuration
# ==========================================

include(FeatureConfiguration)
include(ProjectVersion)
include(PlatformConfiguration)
include(CompilerConfiguration)
include(DependencyManagement)

# ==========================================
# Apply Configurations
# ==========================================

# Support Visual Studio Code
include(CMakeToolsHelpers OPTIONAL)
include(KPXCMacDeployHelpers)

# Configure all components
configure_features()
configure_version_and_git()
configure_platform()
configure_compiler()
configure_dependencies()

# ==========================================
# Coverage Configuration
# ==========================================

if(WITH_COVERAGE)
    # Include code coverage, use with -DCMAKE_BUILD_TYPE=Debug
    include(CodeCoverage)
    append_coverage_compiler_flags()

    set(COVERAGE_EXCLUDES
            "'^(.+/)?thirdparty/.*'"
            "'^(.+/)?main\\.cpp$$'"
            "'^(.+/)?cli/keepassxc-cli\\.cpp$$'"
            "'^(.+/)?proxy/keepassxc-proxy\\.cpp$$'")
    if(WITH_COVERAGE AND CMAKE_COMPILER_IS_CLANGXX)
        set(MAIN_BINARIES
                "$<TARGET_FILE:${PROGNAME}>"
                "$<TARGET_FILE:keepassxc-cli>"
                "$<TARGET_FILE:keepassxc-proxy>")
        setup_target_for_coverage_llvm(
                NAME coverage
                BINARY ${MAIN_BINARIES}
                SOURCES_ROOT ${CMAKE_SOURCE_DIR}/src
        )
    else()
        setup_target_for_coverage_gcovr(
                NAME coverage
                SOURCES_ROOT ${CMAKE_SOURCE_DIR}/src
        )
    endif()
endif()

# ==========================================
# Code Formatting
# ==========================================

include(CLangFormat)

# ==========================================
# Build Subdirectories
# ==========================================

add_subdirectory(src)
add_subdirectory(share)
if(WITH_TESTS)
    add_subdirectory(tests)
endif(WITH_TESTS)

if(WITH_XC_DOCS)
    add_subdirectory(docs)
endif()

# ==========================================
# Feature Summary
# ==========================================

if(PRINT_SUMMARY)
    # This will print ENABLED, REQUIRED and DISABLED
    feature_summary(WHAT ALL)
else()
    # This will only print ENABLED and DISABLED feature
    feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
    feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
endif()