cmake_minimum_required(VERSION 3.10)
project(multjs)

set(CMAKE_CXX_STANDARD 20)

set(MJS_LIB mjs)
set(MJS_HEADER_DIR ./include)

if(WIN32)
    file(GLOB_RECURSE PRIVATE_HEADERS 
        ./src/*.h
        ./src/*.hpp
        ./src/**/*.h
        ./src/**/*.hpp)
    source_group("Private Header Files" FILES ${PRIVATE_HEADERS})
    
    file(GLOB_RECURSE PUBLIC_HEADERS 
        ./include/*.h
        ./include/*.hpp
        ./include/**/*.h
        ./include/**/*.hpp)
    source_group("Public Header Files" FILES ${PUBLIC_HEADERS})
endif()
file(GLOB SRC
    ./src/*.cpp
    ./src/**/*.cpp)


add_library(${MJS_LIB} SHARED ${SRC} ${PRIVATE_HEADERS} ${PUBLIC_HEADERS})
target_include_directories(${MJS_LIB} PRIVATE ${MJS_HEADER_DIR})

add_executable(mjs_test test/test.cpp)
target_include_directories(mjs_test PRIVATE ${MJS_HEADER_DIR})
target_link_libraries(mjs_test PRIVATE ${MJS_LIB})