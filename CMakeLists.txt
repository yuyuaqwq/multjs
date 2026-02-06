cmake_minimum_required(VERSION 3.10)
project(multjs)

set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    add_compile_options(/utf-8 /FS)
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

# 过滤掉 JIT 相关的源文件（JIT 由 ENABLE_JIT 选项单独控制）
list(FILTER SRC EXCLUDE REGEX ".*src/jit/.*")

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

# ========== JIT编译器支持 ==========

#[[
option(ENABLE_JIT "Enable JIT compilation" OFF)

if(ENABLE_JIT)
    message(STATUS "JIT compilation enabled")

    # 使用FetchContent获取asmjit
    include(FetchContent)
    FetchContent_Declare(
        asmjit
        GIT_REPOSITORY https://github.com/asmjit/asmjit.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(asmjit)

    # 定义JIT编译宏
    target_compile_definitions(${MJS_LIB_TARGET} PRIVATE ENABLE_JIT)

    # JIT源文件
    file(GLOB JIT_SRC
        ./src/jit/*.cpp
    )

    file(GLOB JIT_HEADERS
        ./include/mjs/jit/*.h
    )

    # 创建JIT库
    add_library(mjs_jit STATIC ${JIT_SRC} ${JIT_HEADERS})
    target_include_directories(mjs_jit
        PRIVATE ${MJS_INCLUDE_DIR}
        PRIVATE .
        PUBLIC ./include/mjs/jit
    )

    # 链接依赖
    target_link_libraries(mjs_jit
        PUBLIC ${MJS_LIB_TARGET}
        PUBLIC asmjit::asmjit
    )

    # 定义JIT编译宏（JIT库也需要）
    target_compile_definitions(mjs_jit PUBLIC ENABLE_JIT)

    # 将JIT库链接到主库
    target_link_libraries(${MJS_LIB_TARGET} PRIVATE mjs_jit)

    # JIT单元测试
    file(GLOB JIT_TEST_SRC ./tests/jit/*.cpp)

    if(JIT_TEST_SRC)
        add_executable(jit_tests ${JIT_TEST_SRC})
        target_include_directories(jit_tests PRIVATE ${MJS_INCLUDE_DIR} . ./tests/unit)
        target_link_libraries(jit_tests PRIVATE mjs_jit GTest::gtest_main)
        gtest_discover_tests(jit_tests)
    else()
        message(STATUS "No JIT tests found, skipping jit_tests target")
    endif()
else()
    message(STATUS "JIT compilation disabled")
endif()
]]