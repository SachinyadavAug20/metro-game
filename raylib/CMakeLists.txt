cmake_minimum_required(VERSION 3.15)
project(CoasterGrid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib 4.0 QUIET)
if (NOT raylib_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RAYLIB REQUIRED raylib)
endif()

file(GLOB SOURCES "src/*.cpp")

add_executable(coaster_grid ${SOURCES})
target_include_directories(coaster_grid PRIVATE src)

if (TARGET raylib)
    target_link_libraries(coaster_grid PRIVATE raylib)
else()
    target_link_libraries(coaster_grid PRIVATE ${RAYLIB_LIBRARIES})
    target_include_directories(coaster_grid PRIVATE ${RAYLIB_INCLUDE_DIRS})
endif()
