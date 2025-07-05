#  Copyright (C) 2024 KeePassXC Team <team@keepassxc.org>
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

# Transactional module for KeePassXC

set(transactional_SOURCES
    Transaction.cpp
    TransactionManager.cpp
    ImmutableEntry.cpp
)

set(transactional_HEADERS
    Transaction.h
    TransactionManager.h
    ImmutableEntry.h
)

# Add library for transactional functionality
add_library(transactional STATIC ${transactional_SOURCES} ${transactional_HEADERS})

target_link_libraries(transactional
    Qt5::Core
    Qt5::Widgets
)

target_include_directories(transactional
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)

# Add test executable if testing is enabled
if(WITH_GUI_TESTS)
    add_executable(TransactionalTest TransactionalTest.cpp)
    target_link_libraries(TransactionalTest
        transactional
        Qt5::Test
        Qt5::Core
    )
    
    add_test(NAME TransactionalTest COMMAND TransactionalTest)
endif()

# Add demo executable
add_executable(TransactionalDemo TransactionalDemo.cpp)
target_link_libraries(TransactionalDemo
    transactional
    Qt5::Core
)

target_include_directories(TransactionalDemo
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)