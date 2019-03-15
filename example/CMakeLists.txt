cmake_minimum_required(VERSION 3.4.0)

project(example CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/.
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

file (GLOB CPP_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Models/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Ctrls/*.cpp
)
file (GLOB H_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Models/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Ctrls/*.h
)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

add_executable(example ${SOURCE_FILES})
