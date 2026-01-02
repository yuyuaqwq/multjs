#!/bin/bash
# 分析源文件和测试文件的对应关系

echo "=== 核心源文件测试覆盖分析 ==="
echo ""

# 定义源文件和对应的测试文件映射
declare -A source_test_map=(
    ["value.cpp"]="value_test.cpp"
    ["runtime.cpp"]="runtime_context_test.cpp"
    ["context.cpp"]="runtime_context_test.cpp"
    ["object.cpp"]="object_shape_test.cpp"
    ["shape.cpp"]="object_shape_test.cpp"
    ["shape_manager.cpp"]="object_shape_test.cpp"
    ["shape_property.cpp"]="object_shape_test.cpp"
    ["transition_table.cpp"]="object_shape_test.cpp"
    ["stack_frame.cpp"]="stack_frame_test.cpp"
    ["up_value.cpp"]="stack_frame_test.cpp"
    ["bytecode_table.cpp"]="bytecode_test.cpp"
    ["vm.cpp"]="vm_test.cpp"
    ["class_def.cpp"]="class_def_test.cpp"
    ["class_def_table.cpp"]="class_def_test.cpp"
    ["function_def.cpp"]="function_module_test.cpp"
    ["module_def.cpp"]="function_module_test.cpp"
    ["module_manager.cpp"]="function_module_test.cpp"
    ["global_const_pool.cpp"]="const_pool_test.cpp"
    ["local_const_pool.cpp"]="const_pool_test.cpp"
    ["array_object.cpp"]="objects_test.cpp"
    ["function_object.cpp"]="objects_test.cpp"
    ["generator_object.cpp"]="objects_test.cpp"
    ["module_object.cpp"]="objects_test.cpp"
    ["promise_object.cpp"]="objects_test.cpp"
    ["cpp_module_object.cpp"]="objects_test.cpp"
)

echo "源文件 -> 测试文件映射检查:"
echo "================================"
for src in "${!source_test_map[@]}"; do
    test="${source_test_map[$src]}"
    if [ -f "tests/unit/$test" ]; then
        echo "✅ $src -> $test"
    else
        echo "❌ $src -> $test (测试文件不存在)"
    fi
done

echo ""
echo "=== 可能缺少测试的源文件 ==="
echo "检查src目录下缺少测试的.cpp文件:"
for src in src/*.cpp; do
    base=$(basename "$src" .cpp)
    test="tests/unit/${base}_test.cpp"
    if [ ! -f "$test" ]; then
        echo "⚠️  $src (缺少 $test)"
    fi
done

echo ""
echo "检查src/object_impl目录下缺少测试的.cpp文件:"
for src in src/object_impl/*.cpp 2>/dev/null; do
    if [ -f "$src" ]; then
        base=$(basename "$src" .cpp)
        test="tests/unit/${base}_test.cpp"
        if [ ! -f "$test" ]; then
            echo "⚠️  $src (可能缺少独立测试,但在objects_test.cpp中)"
        fi
    fi
done
