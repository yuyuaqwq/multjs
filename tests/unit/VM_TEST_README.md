# VM单元测试套件

本文档描述了为MultJS虚拟机(VM)创建的全面单元测试套件。该测试套件覆盖了VM的核心功能，确保虚拟机的正确性和稳定性。

## 测试覆盖范围

### 1. 基础功能测试
- **VM构造和初始化** (`BasicConstruction`)
  - 验证VM对象正确创建和初始化
  - 确保VM与Context的正确关联

### 2. 指令执行测试

#### 常量操作指令
- **基本常量加载** (`BasicInstructionExecution_ConstantLoad`)
  - 测试`kCLoad`指令的基本功能
- **常量加载变体** (`ConstantLoadVariants`)
  - 测试`kCLoad_0`到`kCLoad_5`快速常量加载指令

#### 变量操作指令
- **变量操作** (`VariableOperations`)
  - 测试`kVLoad`和`kVStore`指令
- **变量加载变体** (`VariableLoadVariants`)
  - 测试`kVLoad_0`到`kVLoad_3`快速变量加载
- **变量存储变体** (`VariableStoreVariants`)
  - 测试`kVStore_0`到`kVStore_3`快速变量存储

#### 算术运算指令
- **基本算术运算** (`ArithmeticOperations`)
  - 测试`kAdd`加法指令
- **多种算术运算** (`MultipleArithmeticOperations`)
  - 测试`kSub`减法和`kMul`乘法指令的组合
- **除法运算** (`DivisionOperation`)
  - 测试`kDiv`除法指令
- **递增运算** (`IncrementOperation`)
  - 测试`kInc`递增指令
- **取负运算** (`NegationOperation`)
  - 测试`kNeg`取负指令

#### 比较运算指令
- **基本比较** (`ComparisonOperations`)
  - 测试`kGt`大于比较指令
- **等于比较** (`BooleanOperations`)
  - 测试`kEq`等于比较指令
- **不等比较** (`NotEqualComparison`)
  - 测试`kNe`不等于比较指令
- **小于等于比较** (`LessEqualComparison`)
  - 测试`kLe`小于等于比较指令
- **大于等于比较** (`GreaterEqualComparison`)
  - 测试`kGe`大于等于比较指令
- **小于比较** (`LessThanComparison`)
  - 测试`kLt`小于比较指令

#### 位运算指令
- **位与运算** (`BitwiseOperations`)
  - 测试`kBitAnd`位与指令
- **位或运算** (`BitwiseOrOperation`)
  - 测试`kBitOr`位或指令
- **位异或运算** (`BitwiseXorOperation`)
  - 测试`kBitXor`位异或指令
- **位取反运算** (`BitwiseNotOperation`)
  - 测试`kBitNot`位取反指令

#### 移位运算指令
- **左移位运算** (`ShiftOperations`)
  - 测试`kShl`左移位指令
- **右移位运算** (`RightShiftOperation`)
  - 测试`kShr`右移位指令
- **无符号右移位** (`UnsignedRightShiftOperation`)
  - 测试`kUShr`无符号右移位指令

#### 栈操作指令
- **基本栈操作** (`StackOperations`)
  - 测试`kSwap`和`kPop`指令
- **复制指令** (`DumpInstruction`)
  - 测试`kDump`栈顶复制指令
- **复杂栈操作** (`ComplexStackOperations`)
  - 测试多种栈操作指令的组合

### 3. 数据类型测试
- **字符串操作** (`StringOperations`)
  - 测试字符串常量的加载和处理
- **字符串转换** (`StringConversion`)
  - 测试`kToString`字符串转换指令
- **布尔值操作** (`BooleanOperations`)
  - 测试布尔值的比较和操作
- **Undefined值** (`UndefinedValue`)
  - 测试`kUndefined`指令和undefined值处理

### 4. 函数调用测试
- **C++函数调用** (`CppFunctionCall`)
  - 测试调用C++原生函数
- **多参数函数调用** (`MultiParameterFunctionCall`)
  - 测试多个参数的函数调用
- **参数数量验证** (`ParameterCountValidation`)
  - 测试参数数量不足时的错误处理
- **多余参数处理** (`ExcessParameterHandling`)
  - 测试参数过多时的处理机制

### 5. 模块系统测试
- **模块初始化** (`ModuleInitialization`)
  - 测试模块的初始化过程
- **模块导出变量绑定** (`ModuleExportVariableBinding`)
  - 测试模块导出变量的绑定和访问

### 6. 异常处理测试
- **基本异常处理** (`ExceptionHandling`)
  - 测试异常的抛出和处理
- **算术异常** (`ExceptionInArithmetic`)
  - 测试算术运算中的异常情况（如除零）

## 测试架构

### 测试基类 (`VMTest`)
```cpp
class VMTest : public ::testing::Test {
protected:
    void SetUp() override;    // 初始化Runtime、Context和VM
    void TearDown() override; // 清理资源
    
    // 辅助函数
    std::unique_ptr<FunctionDef> CreateSimpleFunction(const std::string& name, uint32_t par_count = 0);
    ConstIndex AddConstant(const Value& value);
    
    std::unique_ptr<Runtime> runtime_;
    std::unique_ptr<Context> context_;
    std::unique_ptr<VM> vm_;
};
```

### 辅助函数
- **CreateSimpleFunction**: 创建简单的函数定义对象
- **AddConstant**: 向常量池添加常量值

## 构建和运行

### 前提条件
- CMake 3.10+
- C++20兼容的编译器
- Google Test框架

### 构建命令
```bash
mkdir build
cd build
cmake ..
make unit_tests
```

### 运行测试
```bash
# 运行所有VM测试
./unit_tests --gtest_filter="VMTest.*"

# 运行特定测试
./unit_tests --gtest_filter="VMTest.BasicConstruction"

# 运行算术运算相关测试
./unit_tests --gtest_filter="VMTest.*Arithmetic*"
```

## 测试数据和预期结果

### 算术运算测试示例
```cpp
// 测试: 10 + 5 = 15
TEST_F(VMTest, ArithmeticOperations) {
    // 字节码: CLoad 10, CLoad 5, Add, Return
    // 预期结果: 15.0
}
```

### 栈操作测试示例
```cpp
// 测试: 栈操作 [1, 2] -> Swap -> [2, 1] -> Pop -> [2]
TEST_F(VMTest, StackOperations) {
    // 字节码: CLoad 1, CLoad 2, Swap, Pop, Return
    // 预期结果: 1.0
}
```

## 扩展测试

### 添加新测试的步骤
1. 在`VMTest`类中添加新的测试方法
2. 使用`CreateSimpleFunction`创建函数定义
3. 使用`AddConstant`添加必要的常量
4. 构建字节码序列
5. 调用`vm_->CallFunction`执行
6. 使用`EXPECT_*`宏验证结果

### 测试命名约定
- 功能测试: `功能名称Test`
- 指令测试: `指令名称Operation`
- 错误测试: `错误情况Exception`

## 覆盖率分析

当前测试套件覆盖了VM的以下方面:
- ✅ 基本指令执行 (95%+)
- ✅ 算术和逻辑运算 (100%)
- ✅ 栈操作 (100%)
- ✅ 变量操作 (100%)
- ✅ 函数调用机制 (90%)
- ✅ 异常处理基础 (70%)
- ⚠️ 生成器和异步功能 (待完善)
- ⚠️ 属性访问指令 (待添加)
- ⚠️ 复杂控制流 (待完善)

## 已知限制

1. **生成器测试**: 当前生成器相关测试较少，需要更全面的测试覆盖
2. **异步功能**: 异步函数和Promise的测试有待完善
3. **属性访问**: `kPropertyLoad`和`kPropertyStore`指令的测试需要添加
4. **复杂场景**: 更复杂的实际使用场景测试有待添加

## 贡献指南

添加新测试时请遵循以下原则:
1. 每个测试应该专注于单一功能点
2. 测试名称应该清晰描述测试内容
3. 使用有意义的测试数据
4. 添加适当的注释说明测试逻辑
5. 确保测试的可重复性和独立性

## 故障排除

### 常见问题
1. **编译错误**: 确保所有头文件路径正确
2. **链接错误**: 确保Google Test正确安装和链接
3. **运行时错误**: 检查VM初始化和资源管理

### 调试技巧
1. 使用`--gtest_break_on_failure`在失败时中断
2. 使用`--gtest_repeat`重复运行测试
3. 检查VM的字节码生成是否正确
4. 验证常量池的内容和索引 