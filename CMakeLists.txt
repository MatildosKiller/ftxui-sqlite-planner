cmake_minimum_required(VERSION 3.16)
project(FTXUI_SQliet_planner)

set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDART_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FTXUI REQUIRED ftxui)
find_package(SQLite3 REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp src/database.cpp)

target_link_libraries(${PROJECT_NAME}
                    ${FTXUI_LIBRARIES}
                    SQLite::SQLite3
)

target_include_directories(${PROJECT_NAME} PRIVATE include ${FTXUI_INCLUDE_DIRS})

