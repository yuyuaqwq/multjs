cmake_minimum_required(VERSION 3.10)
project(multjs)

set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(MJS_LIB_TARGET mjs)
set(MJS_INCLUDE_DIR ./include)

# 启用测试
enable_testing()

# 添加Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
# 对于Windows: 防止覆盖父项目的编译器/链接器设置
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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
        get_filename_component(INCLUDE_DIR ${RELATIVE_HEADER} DIRECTORY)
        # 替换路径分隔符为反斜杠（为了VS显示）
        if(INCLUDE_DIR)
            string(REPLACE "/" "\\" GROUP_NAME "${INCLUDE_DIR}")
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
        get_filename_component(INCLUDE_DIR ${RELATIVE_HEADER} DIRECTORY)
        # 替换路径分隔符为反斜杠（为了VS显示）
        if(INCLUDE_DIR)
            string(REPLACE "/" "\\" GROUP_NAME "${INCLUDE_DIR}")
            source_group("Public Header Files\\${GROUP_NAME}" FILES ${HEADER})
        else()
            source_group("Public Header Files" FILES ${HEADER})
        endif()
    endforeach()
endif()
file(GLOB_RECURSE SRC
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

add_library(${MJS_LIB_TARGET} STATIC ${SRC} ${PRIVATE_HEADERS} ${PUBLIC_HEADERS})
target_include_directories(${MJS_LIB_TARGET} PRIVATE ${MJS_INCLUDE_DIR} .)

# ========== C++代码生成器 ==========
# C++代码生成器库
file(GLOB CPP_GEN_SRC
    ./cpp_gen/*.cpp)

file(GLOB CPP_GEN_HEADERS
    ./cpp_gen/*.h)

add_library(mjs_cpp_gen STATIC ${CPP_GEN_SRC} ${CPP_GEN_HEADERS})
target_include_directories(mjs_cpp_gen
    PUBLIC ${MJS_INCLUDE_DIR}
    PRIVATE .
)
target_link_libraries(mjs_cpp_gen PUBLIC ${MJS_LIB_TARGET})

# 命令行工具
add_executable(js2cpp ./tools/js2cpp.cpp)
target_include_directories(js2cpp PRIVATE .)
target_link_libraries(js2cpp PRIVATE mjs_cpp_gen)

# C++代码生成器测试
# 已启用测试，包括：类型系统测试、代码发射器测试、名称修饰测试、集成测试
file(GLOB CPP_GEN_TEST_SRC
    ./tests/cpp_gen/*.cpp)

add_executable(cpp_gen_tests ${CPP_GEN_TEST_SRC})
target_include_directories(cpp_gen_tests PRIVATE ${MJS_INCLUDE_DIR} .)
target_link_libraries(cpp_gen_tests PRIVATE mjs_cpp_gen GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(cpp_gen_tests)

# 集成测试
file(GLOB INTEGRATION_TEST_SRC
    ./tests/integration/*.cpp
    ./tests/integration/**/*.cpp)

add_executable(integration_tests ${INTEGRATION_TEST_SRC})
target_include_directories(integration_tests PRIVATE ${MJS_INCLUDE_DIR} .)
target_link_libraries(integration_tests PRIVATE ${MJS_LIB_TARGET} GTest::gtest_main)

# 单元测试
file(GLOB UNIT_TEST_SRC
	./tests/unit/*.cpp
    ./tests/unit/**/*.cpp
    ./tests/unit/**/**/*.cpp
    )

add_executable(unit_tests ${UNIT_TEST_SRC})
target_include_directories(unit_tests PRIVATE ${MJS_INCLUDE_DIR} .)
target_link_libraries(unit_tests PRIVATE ${MJS_LIB_TARGET} GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(integration_tests)
gtest_discover_tests(unit_tests)