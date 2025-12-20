cmake_minimum_required(VERSION 3.20)
project(openvim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add cpp-mcp library
add_subdirectory(external/cpp-mcp)

# Find Qt
find_package(Qt6 QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Qml Quick)
    set(QT_VERSION 5)
else()
    find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick)
    set(QT_VERSION 6)
endif()

# Set Qt to use static plugins if available
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Try to find static Qt libraries first
if(QT_VERSION EQUAL 6)
    find_library(QT6_CORE_STATIC Qt6Core PATHS /usr/lib/x86_64-linux-gnu)
    find_library(QT6_QML_STATIC Qt6Qml PATHS /usr/lib/x86_64-linux-gnu)
    find_library(QT6_QUICK_STATIC Qt6Quick PATHS /usr/lib/x86_64-linux-gnu)
    if(QT6_CORE_STATIC AND QT6_QML_STATIC AND QT6_QUICK_STATIC)
        set(QT_STATIC_LINKING ON)
        message(STATUS "Using static Qt6 libraries")
    endif()
elseif(QT_VERSION EQUAL 5)
    find_library(QT5_CORE_STATIC Qt5Core PATHS /usr/lib/x86_64-linux-gnu)
    find_library(QT5_QML_STATIC Qt5Qml PATHS /usr/lib/x86_64-linux-gnu)
    find_library(QT5_QUICK_STATIC Qt5Quick PATHS /usr/lib/x86_64-linux-gnu)
    if(QT5_CORE_STATIC AND QT5_QML_STATIC AND QT5_QUICK_STATIC)
        set(QT_STATIC_LINKING ON)
        message(STATUS "Using static Qt5 libraries")
    endif()
endif()

add_executable(openvim
  src/main.cpp
  src/config.cpp
  src/logging/logger.cpp
  src/db/db.cpp
  src/db/migrate.cpp
  src/session/session.cpp
  src/message/message.cpp
  src/llm/llm.cpp
  src/permission/permission.cpp
  src/tools/agent_tool.cpp
  src/tools/bash_tool.cpp
  src/tools/edit_tool.cpp
  src/tools/file_tool.cpp
  src/tools/glob_tool.cpp
  src/tools/grep_tool.cpp
  src/tools/ls_tool.cpp
  src/tools/mcp_tool.cpp
  src/tools/view_tool.cpp
  src/tools/write_tool.cpp
  src/gui/resources.qrc
)

target_include_directories(openvim PRIVATE src)
target_include_directories(openvim PRIVATE external/cpp-mcp/include)
target_include_directories(openvim PRIVATE external/cpp-mcp/common)

# ncurses (wide char) is preferred; fall back to non-wide.
find_package(Curses REQUIRED)
target_link_libraries(openvim PRIVATE ${CURSES_LIBRARIES})
target_include_directories(openvim PRIVATE ${CURSES_INCLUDE_DIR})

find_package(SQLite3 REQUIRED)
target_link_libraries(openvim PRIVATE SQLite::SQLite3 mcp)

# Qt libraries
if(QT_VERSION EQUAL 6)
    target_link_libraries(openvim PRIVATE Qt6::Core Qt6::Qml Qt6::Quick)
else()
    target_link_libraries(openvim PRIVATE Qt5::Core Qt5::Qml Qt5::Quick)
endif()

# Add linker flags to avoid snap library conflicts
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(openvim PRIVATE pthread)
  # Set RPATH to prefer system libraries over snap libraries
  set_target_properties(openvim PROPERTIES
    INSTALL_RPATH "/usr/lib/x86_64-linux-gnu"
    BUILD_WITH_INSTALL_RPATH TRUE
    LINK_FLAGS "-Wl,-rpath,/usr/lib/x86_64-linux-gnu"
  )
endif()
