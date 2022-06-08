# Copyright(c) 2019 spdlog authors Distributed under the MIT License (http://opensource.org/licenses/MIT)

cmake_minimum_required(VERSION 3.10)
project(spdlog_examples CXX)

 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -g  -Wno-unused-function -fvisibility=hidden   -Wno-deprecated-declarations ")
if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

# ---------------------------------------------------------------------------------------
# Example of using pre-compiled library
# ---------------------------------------------------------------------------------------
add_executable(example example.cpp)
target_link_libraries(example PRIVATE spdlog::spdlog)

# ---------------------------------------------------------------------------------------
# Example of using header-only library
# ---------------------------------------------------------------------------------------
if(SPDLOG_BUILD_EXAMPLE_HO)
    add_executable(example_header_only example.cpp)
    target_link_libraries(example_header_only PRIVATE spdlog::spdlog_header_only)
endif()

include_directories(../include)


add_executable(testshm testshm.cpp)
target_link_libraries(testshm PRIVATE spdlog::spdlog)



add_executable(testfmt testfmt.cpp)
target_link_libraries(testfmt  PRIVATE spdlog::spdlog)



add_executable(shmlog shmlog.cpp)
target_link_libraries(shmlog PRIVATE spdlog::spdlog)
