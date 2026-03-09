cmake_minimum_required(VERSION 3.16)
project(FTXUI_SQliet_planner)

# Исправление опечатки в названии стандарта
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим библиотеки через vcpkg (не через pkg-config)
find_package(ftxui REQUIRED)
find_package(SQLite3 REQUIRED)

# Создаём исполняемый файл
add_executable(${PROJECT_NAME}
  src/main.cpp
  src/database.cpp
)

# Указываем директорию для поиска заголовков проекта
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Подключаем библиотеки
target_link_libraries(${PROJECT_NAME}
  ftxui::ftxui      # FTXUI из vcpkg
  SQLite::SQLite3    # SQLite из vcpkg
)
