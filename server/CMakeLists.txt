cmake_minimum_required(VERSION 3.20)
project(SE_Server)

# Подключение vcpkg toolchain
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg-2025.03.19/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
endif()

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)  # Изменено с 20 на 17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Исполняемый файл
add_executable(server 
    main.cpp
    connection.cpp
)

# Библиотека
add_library(engine
    engine/engine.cpp
)

# Поиск пакетов
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS url system)
find_package(libpqxx REQUIRED)

# Отладочный вывод
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
message(STATUS "libpqxx include dirs: ${libpqxx_INCLUDE_DIRS}")
message(STATUS "libpqxx libraries: ${libpqxx_LIBRARIES}")

# Директории включаемых файлов
set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${libpqxx_INCLUDE_DIRS}
    "C:/vcpkg-2025.03.19/installed/x64-windows/include"
)

target_include_directories(server PUBLIC ${COMMON_INCLUDE_DIRS})
target_include_directories(engine PUBLIC ${COMMON_INCLUDE_DIRS})

# Связывание библиотек
target_link_libraries(server PUBLIC 
    engine
    libpqxx::pqxx
    OpenSSL::SSL
    Boost::url
    Boost::system
)

# Опциональные флаги компиляции
if(MSVC)
    target_compile_options(server PRIVATE /W4)
    target_compile_options(engine PRIVATE /W4)
else()
    target_compile_options(server PRIVATE -Wall -Wextra)
    target_compile_options(engine PRIVATE -Wall -Wextra)
endif()