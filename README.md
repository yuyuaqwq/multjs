# MultJS

MultJS是一个轻量级的JavaScript引擎实现。

## 重构进展

### 已完成
- 词法分析器(Lexer)重构
- 语法分析器(Parser)重构
- 代码生成器(CodeGenerator)重构

### 代码生成器重构
代码生成器负责将抽象语法树(AST)转换为字节码，以便虚拟机执行。重构的主要内容包括：

1. 改进命名规范：
   - 将类名从`CodeGener`改为更规范的`CodeGenerator`
   - 将方法名从`EntryScope`改为`EnterScope`
   - 将方法名从`AllocConst`改为`AllocateConst`
   - 将方法名从`AllocVar`改为`AllocateVar`
   - 将方法名从`FindVarIndexByName`改为`FindVarInfoByName`
   - 将方法名从`GetVarByExpression`改为`GetVarInfoByExpression`

2. 改进代码结构：
   - 添加了详细的文档注释
   - 添加了参数验证和异常处理
   - 使用现代C++特性，如`std::optional`和`nullptr`
   - 将成员变量名改为更清晰的命名，如`cur_module_def_`改为`current_module_def_`

3. 添加单元测试：
   - 添加了`code_generator_test.cpp`文件，包含多个测试用例
   - 测试用例涵盖了表达式、变量声明、条件语句、循环语句、函数声明和调用、数组表达式、对象表达式和异常处理

## 待完成
- 虚拟机(VM)重构
- 运行时(Runtime)重构
- 对象系统重构
- 内存管理优化
- 更多单元测试

## 构建与运行

### 依赖项
- CMake 3.10+
- C++20兼容的编译器

### 构建步骤
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 运行测试
```bash
cd build
ctest
```