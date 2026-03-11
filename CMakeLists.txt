cmake_minimum_required(VERSION 3.16)
project(FTXUI_SQLite_planner)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Используем vcpkg для зависимостей
find_package(SQLite3 REQUIRED)

# Явно указываем пути к FTXUI через vcpkg
set(VCPKG_ROOT "C:/vcpkg")
set(FTXUI_INCLUDE_DIR "${VCPKG_ROOT}/installed/x64-windows/include")
set(FTXUI_LIB_DIR "${VCPKG_ROOT}/installed/x64-windows/lib")

# Проверяем существование путей
if(NOT EXISTS "${FTXUI_INCLUDE_DIR}")
  message(FATAL_ERROR "FTXUI include directory not found: ${FTXUI_INCLUDE_DIR}")
endif()

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/database.cpp
)

# Добавляем пути к заголовочным файлам FTXUI
target_include_directories(${PROJECT_NAME} PRIVATE
  include
  ${FTXUI_INCLUDE_DIR}
)

# Связываем с библиотеками FTXUI (явные пути)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  "${FTXUI_LIB_DIR}/ftxui-component.lib"
  "${FTXUI_LIB_DIR}/ftxui-screen.lib"
  "${FTXUI_LIB_DIR}/ftxui-dom.lib"
  SQLite::SQLite3
)
