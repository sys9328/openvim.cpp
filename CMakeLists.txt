cmake_minimum_required(VERSION 3.20)
project(openvim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(openvim
  src/main.cpp
  src/config.cpp
  src/logging/logger.cpp
  src/db/db.cpp
  src/db/migrate.cpp
  src/session/session.cpp
  src/message/message.cpp
  src/llm/llm.cpp
  src/tui/app.cpp
  src/tui/pages/init_page.cpp
  src/tui/pages/repl_page.cpp
  src/tui/pages/logs_page.cpp
)

target_include_directories(openvim PRIVATE src)

# ncurses (wide char) is preferred; fall back to non-wide.
find_package(Curses REQUIRED)
target_link_libraries(openvim PRIVATE ${CURSES_LIBRARIES})
target_include_directories(openvim PRIVATE ${CURSES_INCLUDE_DIR})

find_package(SQLite3 REQUIRED)
target_link_libraries(openvim PRIVATE SQLite::SQLite3)

# Some distros need this explicitly when linking with libncursesw.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(openvim PRIVATE pthread)
endif()
