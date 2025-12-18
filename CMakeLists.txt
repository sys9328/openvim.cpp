cmake_minimum_required(VERSION 3.20)
project(opencode_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(opencode_cpp
  src/main.cpp
  src/logging/logger.cpp
  src/tui/app.cpp
  src/tui/pages/init_page.cpp
  src/tui/pages/repl_page.cpp
  src/tui/pages/logs_page.cpp
)

target_include_directories(opencode_cpp PRIVATE src)

# ncurses (wide char) is preferred; fall back to non-wide.
find_package(Curses REQUIRED)
target_link_libraries(opencode_cpp PRIVATE ${CURSES_LIBRARIES})
target_include_directories(opencode_cpp PRIVATE ${CURSES_INCLUDE_DIR})

# Some distros need this explicitly when linking with libncursesw.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(opencode_cpp PRIVATE pthread)
endif()
