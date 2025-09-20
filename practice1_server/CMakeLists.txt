cmake_minimum_required(VERSION 3.16)
project(AuthServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PQXX_INCLUDE_DIR "/usr/include")
set(PQXX_LIBRARY "/usr/lib/x86_64-linux-gnu/libpqxx.so")

find_package(PostgreSQL REQUIRED)

add_executable(practice1_server
    main.cpp
    clientHandler.cpp
    dbAPI.cpp
)

target_include_directories(practice1_server PRIVATE
    ${PostgreSQL_INCLUDE_DIRS}
    ${PQXX_INCLUDE_DIR}
)

target_link_libraries(practice1_server PRIVATE
    ${PQXX_LIBRARY}
    ${PostgreSQL_LIBRARIES}
)