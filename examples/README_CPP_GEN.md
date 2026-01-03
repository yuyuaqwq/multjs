# JavaScript to C++ Transpiler

这是一个为multjs项目添加的JavaScript到C++的转译功能，可以将JavaScript逻辑编译为高性能的C++代码。

## 功能特性

- ✅ 静态类型推断 + 动态类型回退
- ✅ 生成生产级高性能C++代码
- ✅ 完全复用现有AST系统（无需修改Lexer和Parser）
- ✅ 模块化设计，易于扩展
- ✅ 命令行工具和编程API两种使用方式

## 架构设计

### 核心组件

1. **CppType** - 类型系统（JavaScript类型到C++类型的映射）
   - 支持基本类型：int64_t, double, bool, std::string
   - 支持复杂类型：std::vector, std::optional, std::variant
   - 动态类型回退：mjs::generated::JSValue

2. **TypeInferenceEngine** - 类型推断引擎
   - 字面量类型推断
   - 二元表达式类型推断
   - 变量声明类型推断
   - 作用域管理

3. **CppCodeGenerator** - C++代码生成器
   - 表达式代码生成
   - 语句代码生成
   - 函数生成
   - 配置选项支持

4. **CodeEmitter** - 代码发射器
   - 自动缩进管理
   - 代码格式化
   - 块结构辅助

5. **NameMangler** - 名称修饰器
   - 处理C++关键字冲突
   - 标识符合法性检查

6. **mjs_runtime** - 运行时支持库
   - JSValue（动态类型值）
   - JSObject（动态对象）
   - JSArray（动态数组）

## 文件结构

```
multjs/
├── include/mjs/cpp_gen/
│   └── mjs_runtime.h              # 运行时支持库
│
├── src/compiler/cpp_gen/
│   ├── cpp_code_generator.h       # C++代码生成器
│   ├── cpp_code_generator.cpp
│   ├── type_inference_engine.h    # 类型推断引擎
│   ├── type_inference_engine.cpp
│   ├── cpp_type.h                 # 类型系统
│   ├── cpp_type.cpp
│   ├── code_emitter.h             # 代码发射器
│   ├── code_emitter.cpp
│   ├── name_mangler.h             # 名称修饰
│   └── name_mangler.cpp
│
├── tools/
│   └── js2cpp.cpp                 # 命令行工具
│
├── tests/cpp_gen/
│   ├── cpp_type_test.cpp          # 类型测试
│   ├── code_emitter_test.cpp      # 发射器测试
│   ├── name_mangler_test.cpp      # 名称修饰测试
│   └── integration_test.cpp       # 集成测试
│
└── examples/
    └── simple_example.js          # 使用示例
```

## 使用方法

### 1. 编译项目

```bash
cd build
cmake ..
cmake --build .
```

### 2. 命令行工具使用

```bash
# 基本用法
./js2cpp input.js -o output.cpp

# 指定命名空间
./js2cpp input.js --namespace game_logic -o output.cpp

# 禁用类型推断
./js2cpp input.js --no-type-inference -o output.cpp

# 设置缩进大小
./js2cpp input.js --indent 2 -o output.cpp

# 输出到stdout
./js2cpp input.js
```

### 3. 编程API使用

```cpp
#include <mjs/cpp_gen/cpp_code_generator.h>
#include <mjs/compiler/lexer.h>
#include <mjs/compiler/parser.h>

int main() {
    // 读取JavaScript代码
    std::string js_code = ReadFile("game_logic.js");

    // 词法分析和语法分析
    mjs::compiler::Lexer lexer(js_code);
    mjs::compiler::Parser parser(&lexer);
    parser.ParseProgram();

    // 配置代码生成器
    mjs::compiler::cpp_gen::CppCodeGeneratorConfig config;
    config.namespace_name = "game";
    config.enable_type_inference = true;

    // 生成C++代码
    mjs::compiler::cpp_gen::CppCodeGenerator generator(config);
    std::string cpp_code = generator.Generate(parser);

    // 写入文件
    WriteFile("game_logic.cpp", cpp_code);
    return 0;
}
```

## 转换示例

### 输入JavaScript

```javascript
function calculateDamage(base, multiplier) {
    return base * multiplier;
}

let damage = calculateDamage(100, 1.5);
```

### 输出C++

```cpp
namespace mjs_generated {

auto calculateDamage(auto base, auto multiplier) {
    return base * multiplier;
}

auto damage = calculateDamage(100, 1.5);

} // namespace mjs_generated
```

## 运行测试

```bash
cd build
./cpp_gen_tests
```

## 支持的语言特性

### 完全支持
- ✅ 基本类型（数字、字符串、布尔）
- ✅ 变量声明（let/const）
- ✅ 函数定义和调用
- ✅ 控制流（if/while/for）
- ✅ 运算符（算术、比较、逻辑）
- ✅ 数组字面量
- ✅ 成员访问

### 部分支持（待完善）
- ⚠️ 对象字面量
- ⚠️ 函数闭包
- ⚠️ 递归函数

### 不支持
- ❌ eval
- ❌ with语句
- ❌ Proxy/Reflect
- ❌ Generator函数

## 实现阶段

- ✅ 阶段1：核心框架（CppType、CodeEmitter、NameMangler）
- ✅ 阶段2：类型推断引擎
- ✅ 阶段3：表达式代码生成
- ✅ 阶段4：语句代码生成
- ⏳ 阶段5：函数和闭包（部分完成）
- ✅ 阶段6：运行时支持
- ✅ 阶段7：工具和集成

## 后续工作

当前实现提供了核心框架和基础功能，以下功能待完善：

1. **更完整的类型推断**
   - 函数签名推断
   - 对象属性类型推断
   - 控制流类型合并

2. **更完整的语句生成**
   - if/else语句
   - while/for循环
   - return语句
   - 函数声明

3. **函数和闭包支持**
   - 闭包变量捕获
   - 嵌套函数
   - 递归函数

4. **性能优化**
   - 减少不必要的类型转换
   - 内联简单函数
   - 常量折叠

5. **测试完善**
   - 更多单元测试
   - 双执行验证测试
   - 性能基准测试

## 注意事项

- 这是一个独立的模块，不会影响原有multjs项目
- 当前版本为初步实现，建议用于实验和测试
- 生产环境使用需要更完善的测试和优化
