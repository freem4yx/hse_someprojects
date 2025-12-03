cmake_minimum_required(VERSION 3.10)
project(NetworkChat)

set(CMAKE_CXX_STANDARD 11)

# Находим пакет для многопоточности
find_package(Threads REQUIRED)

add_executable(server server.cpp wrappers.cpp)
add_executable(client client.cpp wrappers.cpp)

# Линкуем с библиотекой потоков
target_link_libraries(server Threads::Threads)
target_link_libraries(client Threads::Threads)