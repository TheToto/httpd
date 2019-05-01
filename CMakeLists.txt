cmake_minimum_required(VERSION 3.0)
project (spider)

FIND_PACKAGE( Boost 1.55 COMPONENTS program_options REQUIRED )

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -std=c++17 -pedantic")

file(GLOB_RECURSE SOURCES "src/**/*.cc" "src/*.cc")

include_directories(lib/ src/)

add_executable(spider ${SOURCES})

target_link_libraries(spider LINK_PUBLIC ${Boost_LIBRARIES} ev ssl crypto stdc++fs)