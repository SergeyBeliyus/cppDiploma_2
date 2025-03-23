cmake_minimum_required(VERSION 3.20)
project(SE_Client)

# Подключение vcpkg toolchain
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg-2025.03.19/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
endif()

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Исполняемый файл
add_executable(client main.cpp)

# Библиотеки
add_library(indexer indexer/indexer.cpp)
add_library(parser indexer/parser/parser.cpp)
add_library(utils utils/utils.cpp)
add_library(link_get indexer/link_get/link_get.cpp)
add_library(sql_database indexer/sql_database/sql_database.cpp)

# Поиск пакетов
find_package(OpenSSL REQUIRED)
find_package(Boost 1.81 REQUIRED COMPONENTS url system)
find_package(libpqxx REQUIRED)

# Отладочный вывод
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
message(STATUS "libpqxx include dirs: ${libpqxx_INCLUDE_DIRS}")
message(STATUS "libpqxx libraries: ${libpqxx_LIBRARIES}")

# Директории включаемых файлов для всех таргетов
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${libpqxx_INCLUDE_DIRS}
    "C:/vcpkg-2025.03.19/installed/x64-windows/include"  # Явный путь
)

# Применение путей ко всем таргетам
target_include_directories(client PUBLIC ${COMMON_INCLUDE_DIRS})
target_include_directories(indexer PUBLIC ${COMMON_INCLUDE_DIRS})
target_include_directories(parser PUBLIC ${COMMON_INCLUDE_DIRS})
target_include_directories(utils PUBLIC ${COMMON_INCLUDE_DIRS})
target_include_directories(link_get PUBLIC ${COMMON_INCLUDE_DIRS})
target_include_directories(sql_database PUBLIC ${COMMON_INCLUDE_DIRS})

# Принудительное добавление пути через compile options
if(MSVC)
    target_compile_options(client PRIVATE "/I C:/vcpkg-2025.03.19/installed/x64-windows/include")
    target_compile_options(indexer PRIVATE "/I C:/vcpkg-2025.03.19/installed/x64-windows/include")
    target_compile_options(link_get PRIVATE "/I C:/vcpkg-2025.03.19/installed/x64-windows/include")
    target_compile_options(parser PRIVATE "/I C:/vcpkg-2025.03.19/installed/x64-windows/include")
    target_compile_options(utils PRIVATE "/I C:/vcpkg-2025.03.19/installed/x64-windows/include")
    target_compile_options(sql_database PRIVATE "/I C:/vcpkg-2025.03.19/installed/x64-windows/include")
endif()

# Связывание библиотек
target_link_libraries(utils PUBLIC
    Boost::url
    Boost::system
)
target_link_libraries(client PUBLIC
    indexer
    parser
    utils
    link_get
    sql_database
    libpqxx::pqxx
    OpenSSL::SSL
    Boost::url
    Boost::system
)

# Опциональные флаги компиляции
if(MSVC)
    target_compile_options(client PRIVATE /W4)
    target_compile_options(indexer PRIVATE /W4)
    target_compile_options(link_get PRIVATE /W4)
else()
    target_compile_options(client PRIVATE -Wall -Wextra)
    target_compile_options(indexer PRIVATE -Wall -Wextra)
    target_compile_options(link_get PRIVATE -Wall -Wextra)
endif()