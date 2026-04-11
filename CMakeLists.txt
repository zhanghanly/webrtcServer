cmake_minimum_required(VERSION 3.10.0)

project(webrtc CXX)

#c++11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER g++)

option(BUILD_TYPE "debug or release" ON)
if (BUILD_TYPE)
    #debug
    set(CMAKE_BUILD_TYPE Debug)
    message("This is debug edit")
else()
    set(CMAKE_BUILD_TYPE Release)
    #O2优化级别
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")
    message("This is release edit")
endif()

option(UNIT_TEST "whether build unit test" OFF)
if (UNIT_TEST)   
    add_subdirectory(test)
endif()

add_definitions(-DMS_LITTLE_ENDIAN)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/FBS)
include_directories(${PROJECT_SOURCE_DIR}/src/Utils)
include_directories(${PROJECT_SOURCE_DIR}/src/rtc)
include_directories(${PROJECT_SOURCE_DIR}/src/grpc)
include_directories(${PROJECT_SOURCE_DIR}/src/Channel)
include_directories(${PROJECT_SOURCE_DIR}/subprojects)
include_directories(${PROJECT_SOURCE_DIR}/subprojects/flatbuffers-24.3.25/include)
include_directories(${PROJECT_SOURCE_DIR}/subprojects/grpc/third_party/abseil-cpp)
include_directories(${PROJECT_SOURCE_DIR}/subprojects/libsrtp-3.0.0-beta/include)
include_directories(${PROJECT_SOURCE_DIR}/deps/libwebrtc/libwebrtc/)
include_directories(${PROJECT_SOURCE_DIR}/subprojects/openssl-3.0.8/include)
include_directories(${PROJECT_SOURCE_DIR}/deps/libwebrtc)
include_directories(${PROJECT_SOURCE_DIR}/subprojects/libuv-v1.51.0/include/)
include_directories(${PROJECT_SOURCE_DIR}/subprojects/usrsctp-d45b53f5dfa79533f5c5e7aefa5d7570405afb39/usrsctplib/)
include_directories(${PROJECT_SOURCE_DIR}/subprojects/grpc/include)
include_directories(${PROJECT_SOURCE_DIR}/subprojects/grpc/third_party/protobuf/src)

add_subdirectory(deps)

add_subdirectory(src)