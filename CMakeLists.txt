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
    # source_group("Private Header Files" FILES ${PRIVATE_HEADERS})
    
    # 为私有头文件创建保持目录结构的源组
    foreach(HEADER ${PRIVATE_HEADERS})
        # 获取相对于src目录的路径
        file(RELATIVE_PATH RELATIVE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src ${HEADER})
        # 获取目录部分
        get_filename_component(HEADER_DIR ${RELATIVE_HEADER} DIRECTORY)
        # 替换路径分隔符为反斜杠（为了VS显示）
        if(HEADER_DIR)
            string(REPLACE "/" "\\" GROUP_NAME "${HEADER_DIR}")
            source_group("Private Header Files\\${GROUP_NAME}" FILES ${HEADER})
        else()
            source_group("Private Header Files" FILES ${HEADER})
        endif()
    endforeach()

    file(GLOB_RECURSE PUBLIC_HEADERS 
        ./include/*.h
        ./include/*.hpp
        ./include/**/*.h
        ./include/**/*.hpp)
    #source_group("Public Header Files" FILES ${PUBLIC_HEADERS})

    # 为公共头文件创建保持目录结构的源组
    foreach(HEADER ${PUBLIC_HEADERS})
        # 获取相对于include目录的路径
        file(RELATIVE_PATH RELATIVE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include ${HEADER})
        # 获取目录部分
        get_filename_component(HEADER_DIR ${RELATIVE_HEADER} DIRECTORY)
        # 替换路径分隔符为反斜杠（为了VS显示）
        if(HEADER_DIR)
            string(REPLACE "/" "\\" GROUP_NAME "${HEADER_DIR}")
            source_group("Public Header Files\\${GROUP_NAME}" FILES ${HEADER})
        else()
            source_group("Public Header Files" FILES ${HEADER})
        endif()
    endforeach()
endif()
file(GLOB SRC
    ./src/*.cpp
    ./src/**/*.cpp)

foreach(SOURCE ${SRC})
    file(RELATIVE_PATH RELATIVE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src ${SOURCE})
    get_filename_component(SOURCE_DIR ${RELATIVE_SOURCE} DIRECTORY)
    if(SOURCE_DIR)
        string(REPLACE "/" "\\" GROUP_NAME "${SOURCE_DIR}")
        source_group("Source Files\\${GROUP_NAME}" FILES ${SOURCE})
    else()
        source_group("Source Files" FILES ${SOURCE})
    endif()
endforeach()


add_library(${MJS_LIB} STATIC ${SRC} ${PRIVATE_HEADERS} ${PUBLIC_HEADERS})
target_include_directories(${MJS_LIB} PRIVATE ${MJS_HEADER_DIR})

add_executable(mjs_test test/test.cpp)
target_include_directories(mjs_test PRIVATE ${MJS_HEADER_DIR})
target_link_libraries(mjs_test PRIVATE ${MJS_LIB})