cmake_minimum_required(VERSION 3.30)

project(Utilities C)

add_executable(lock lock.c)
add_executable(mutex mutex.c)
add_executable(unlock unlock.c)

find_package(ZLIB REQUIRED)
add_executable(compress compress.c ${CMAKE_CURRENT_SOURCE_DIR}/../src/coding/zlib.c)
target_link_libraries(compress PRIVATE ZLIB::ZLIB)
