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
# Project Version and Git Information
# ==========================================

# ==========================================
# Version Constants
# ==========================================

function(set_version_constants)
    set(KEEPASSXC_VERSION_MAJOR "2" PARENT_SCOPE)
    set(KEEPASSXC_VERSION_MINOR "8" PARENT_SCOPE)
    set(KEEPASSXC_VERSION_PATCH "0" PARENT_SCOPE)
    set(KEEPASSXC_VERSION "${KEEPASSXC_VERSION_MAJOR}.${KEEPASSXC_VERSION_MINOR}.${KEEPASSXC_VERSION_PATCH}" PARENT_SCOPE)
    
    set(OVERRIDE_VERSION "" CACHE STRING "Override the KeePassXC Version for Snapshot builds")
    set(KEEPASSXC_BUILD_TYPE "Snapshot" CACHE STRING "Set KeePassXC build type to distinguish between stable releases and snapshots")
    set_property(CACHE KEEPASSXC_BUILD_TYPE PROPERTY STRINGS Snapshot Release PreRelease)
endfunction()

# ==========================================
# Git Information Retrieval
# ==========================================

function(get_git_information)
    # Retrieve git HEAD revision hash
    set(GIT_HEAD_OVERRIDE "" CACHE STRING "Manually set the Git HEAD hash when missing (eg, when no .git folder exists)")
    execute_process(COMMAND git rev-parse --short=7 HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_HEAD
            ERROR_QUIET)
    string(STRIP "${GIT_HEAD}" GIT_HEAD)
    
    if(GIT_HEAD STREQUAL "" AND NOT GIT_HEAD_OVERRIDE STREQUAL "")
        string(SUBSTRING "${GIT_HEAD_OVERRIDE}" 0 7 GIT_HEAD)
    elseif(EXISTS ${CMAKE_SOURCE_DIR}/.gitrev)
        file(READ ${CMAKE_SOURCE_DIR}/.gitrev GIT_HEAD)
    endif()
    
    message(STATUS "Found Git HEAD Revision: ${GIT_HEAD}")
    set(GIT_HEAD ${GIT_HEAD} PARENT_SCOPE)
    
    # Check if on a tag, if so build as a release
    execute_process(COMMAND git tag --points-at HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_TAG
            ERROR_QUIET)
    string(REGEX REPLACE "latest" "" GIT_TAG "${GIT_TAG}")
    
    if(GIT_TAG MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+")
        string(STRIP "${GIT_TAG}" GIT_TAG)
        set(OVERRIDE_VERSION ${GIT_TAG} PARENT_SCOPE)
    elseif(EXISTS ${CMAKE_SOURCE_DIR}/.version)
        file(READ ${CMAKE_SOURCE_DIR}/.version OVERRIDE_VERSION)
        set(OVERRIDE_VERSION ${OVERRIDE_VERSION} PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Version String Processing
# ==========================================

function(process_version_string)
    string(REGEX REPLACE "(\\r?\\n)+" "" OVERRIDE_VERSION "${OVERRIDE_VERSION}")
    
    if(OVERRIDE_VERSION)
        if(OVERRIDE_VERSION MATCHES "^[\\.0-9]+-beta[0-9]*")
            set(KEEPASSXC_BUILD_TYPE "PreRelease" PARENT_SCOPE)
            set(KEEPASSXC_VERSION ${OVERRIDE_VERSION} PARENT_SCOPE)
        elseif(OVERRIDE_VERSION MATCHES "^[\\.0-9]+$")
            set(KEEPASSXC_BUILD_TYPE "Release" PARENT_SCOPE)
            set(KEEPASSXC_VERSION ${OVERRIDE_VERSION} PARENT_SCOPE)
        else()
            set(KEEPASSXC_BUILD_TYPE "Snapshot" PARENT_SCOPE)
            set(KEEPASSXC_VERSION ${OVERRIDE_VERSION} PARENT_SCOPE)
        endif()
    else()
        if(KEEPASSXC_BUILD_TYPE STREQUAL "PreRelease")
            set(KEEPASSXC_VERSION "${KEEPASSXC_VERSION}-preview" PARENT_SCOPE)
        elseif(KEEPASSXC_BUILD_TYPE STREQUAL "Snapshot")
            set(KEEPASSXC_VERSION "${KEEPASSXC_VERSION}-snapshot" PARENT_SCOPE)
        endif()
    endif()
endfunction()

# ==========================================
# Build Type Flags
# ==========================================

function(set_build_type_flags)
    if(KEEPASSXC_BUILD_TYPE STREQUAL "Release")
        set(KEEPASSXC_BUILD_TYPE_RELEASE ON PARENT_SCOPE)
    elseif(KEEPASSXC_BUILD_TYPE STREQUAL "PreRelease")
        set(KEEPASSXC_BUILD_TYPE_PRE_RELEASE ON PARENT_SCOPE)
    else()
        set(KEEPASSXC_BUILD_TYPE_SNAPSHOT ON PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Distribution Information
# ==========================================

function(configure_distribution_info)
    set(KEEPASSXC_DIST ON PARENT_SCOPE)
    set(KEEPASSXC_DIST_TYPE "Other" CACHE STRING "KeePassXC Distribution Type")
    set_property(CACHE KEEPASSXC_DIST_TYPE PROPERTY STRINGS Snap AppImage Flatpak Other)
    
    if(KEEPASSXC_DIST_TYPE STREQUAL "Snap")
        set(KEEPASSXC_DIST_SNAP ON PARENT_SCOPE)
    elseif(KEEPASSXC_DIST_TYPE STREQUAL "AppImage")
        set(KEEPASSXC_DIST_APPIMAGE ON PARENT_SCOPE)
    elseif(KEEPASSXC_DIST_TYPE STREQUAL "Flatpak")
        set(KEEPASSXC_DIST_FLATPAK ON PARENT_SCOPE)
    elseif(KEEPASSXC_DIST_TYPE STREQUAL "Other")
        unset(KEEPASSXC_DIST PARENT_SCOPE)
    endif()
endfunction()

# ==========================================
# Main Version Configuration Function
# ==========================================

function(configure_version_and_git)
    message(STATUS "Configuring version and Git information...")
    
    set_version_constants()
    get_git_information()
    process_version_string()
    set_build_type_flags()
    configure_distribution_info()
    
    message(STATUS "Setting up build for KeePassXC v${KEEPASSXC_VERSION}")
    message(STATUS "Version configuration complete")
endfunction()