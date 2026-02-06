# multjs JIT实现

## 概述

multjs的JIT编译器实现，基于asmjit库，提供分层JIT编译能力。

## 当前进度

### ✅ 已完成（第一阶段：基础设施）

1. **asmjit集成**
   - CMake构建系统配置
   - FetchContent自动下载依赖

2. **核心组件**
   - `HotnessCounter`: 热点计数器，跟踪函数执行次数
   - `JitCode`: JIT代码封装，管理编译后的机器码
   - `JITManager`: JIT管理器，协调编译流程和代码缓存
   - `BaselineCompiler`: Baseline编译器框架

3. **FunctionDef集成**
   - 添加JIT相关成员变量
   - 执行次数统计
   - JIT代码指针管理

4. **测试框架**
   - HotnessCounter单元测试
   - 测试基础结构

### ✅ 已完成（第二阶段：MVP - 最小可用版本）

1. **JIT存根函数系统**
   - `JitStubs`: JIT辅助函数集合，用于回退到VM执行
   - 支持常量加载、变量操作、算术运算、比较等基础操作
   - 新增 `IsFalsy` 辅助函数用于条件跳转判断

2. **BaselineCompiler实现**
   - 完整的x64调用约定支持（Windows/System V）
   - 函数序言/尾声生成
   - 寄存器分配和保存
   - 通过存根函数调用VM的实现
   - **控制流支持**：标签管理和跳转指令实现

3. **已实现的指令**（通过存根函数）
   - ✅ 常量加载（kCLoad variants）
   - ✅ 变量操作（kVLoad/kVStore variants）
   - ✅ 栈操作（kPop, kDump, kSwap, kUndefined）
   - ✅ 算术运算（kAdd, kSub, kMul, kDiv, kMod, kNeg, kInc）
   - ✅ 位运算（kShl, kShr, kUShr, kBitAnd, kBitOr, kBitXor, kBitNot）- 新增
   - ✅ 比较运算（kEq, kNe, kLt, kLe, kGt, kGe）
   - ✅ 逻辑运算（kLogicalAnd, kLogicalOr, kNullishCoalescing）- 新增
   - ✅ 类型操作（kTypeof, kToString）
   - ✅ 函数调用（kFunctionCall, kGetThis, kGetOuterThis）- 新增GetOuterThis
   - ✅ 属性操作（kPropertyLoad, kPropertyStore, kIndexedLoad, kIndexedStore）
   - ✅ 对象操作（kNew, kClosure）
   - ✅ 全局变量（kGetGlobal）
   - ✅ 返回（kReturn）
   - ✅ **控制流指令（kGoto, kIfEq）**

4. **控制流实现细节**
   - **标签管理系统**：使用 `std::unordered_map<Pc, asmjit::Label>` 跟踪字节码PC到机器码标签的映射
   - **两遍编译**：第一遍扫描为所有字节码位置创建标签，第二遍生成代码
   - **kGoto**：无条件跳转到目标PC
   - **kIfEq**：弹出栈顶值，如果为falsy则跳转（使用IsFalsy辅助函数）

5. **MVP特性**
   - 对于不支持的指令，自动回退到解释器执行
   - 基础的机器码生成和调用
   - 代码缓存管理（LRU策略）
   - **支持循环和条件分支**：可以编译真实的JavaScript代码

### 📋 MVP限制

以下功能在MVP中未实现，遇到这些指令会回退到解释器：

- 异步操作（kYield, kAwait, kGeneratorReturn, kAsyncReturn）
- 异常处理（kTryBegin, kThrow, kTryEnd, kFinallyReturn, kFinallyGoto）
- 模块加载（kGetModule, kGetModuleAsync）
- Super相关（kGetSuper）
- In/Instanceof

### ✅ 已完成（第三阶段：指令扩展）

1. **位运算指令支持**
   - 实现 `kShl` - 左移运算
   - 实现 `kShr` - 右移运算
   - 实现 `kUShr` - 无符号右移运算
   - 实现 `kBitAnd` - 按位与运算
   - 实现 `kBitOr` - 按位或运算
   - 实现 `kBitXor` - 按位异或运算
   - 实现 `kBitNot` - 按位取反运算

2. **逻辑运算指令支持**
   - 实现 `kLogicalAnd` - 逻辑与运算
   - 实现 `kLogicalOr` - 逻辑或运算
   - 实现 `kNullishCoalescing` - 空值合并运算

3. **GetOuterThis指令支持**
   - 实现 `kGetOuterThis` - 获取外层this值

4. **测试覆盖扩展**
   - 添加 `baseline_compiler_test.cpp` 集成测试
   - 测试位运算指令编译
   - 测试逻辑运算指令编译
   - 测试控制流指令编译
   - 测试算术运算指令编译
   - 测试比较运算指令编译
   - 测试变量操作指令编译
   - 测试函数调用指令编译
   - 测试对象和属性操作指令编译
   - 测试JITManager编译触发机制

### 🚧 下一步计划

1. **指令优化**（优先级：中）
   - 直接生成简单指令的机器码（如常量加载、简单算术）
   - 减少存根函数调用开销
   - 内联优化

2. **剩余指令支持**（优先级：低）
   - 异常处理（kTryBegin, kThrow, kTryEnd, kFinallyReturn, kFinallyGoto）
   - 异步操作（kYield, kAwait, kGeneratorReturn, kAsyncReturn）
   - Super相关（kGetSuper）
   - In/Instanceof（kIn, kInstanceof）
   - 模块加载（kGetModule, kGetModuleAsync）

3. **性能优化**（优先级：中）
   - 寄存器分配优化
   - 死代码消除
   - 常量折叠

## 编译和使用

### 编译选项

默认启用JIT支持：

```bash
cmake -B build -DENABLE_JIT=ON
cmake --build build
```

禁用JIT：

```bash
cmake -B build -DENABLE_JIT=OFF
cmake --build build
```

### 运行测试

```bash
# 运行所有测试
ctest --test-dir build

# 只运行JIT测试
./build/jit_tests
```

## 架构设计

```
JavaScript代码
    ↓
词法/语法分析
    ↓
字节码生成
    ↓
解释器执行 ← 热点检测
    ↓            ↓
[常规路径]  [达到阈值] → Baseline JIT编译 → 机器码执行
```

### 触发条件

- **Baseline JIT**: 函数执行100次后触发
- **Optimized JIT**: 函数执行10000次后触发（未实现）

### 性能目标

- **MVP（已完成）**: 基础功能可用，支持控制流，可编译真实JavaScript代码
- **第三阶段（指令扩展 - 已完成）**: 支持位运算、逻辑运算等更多指令，扩展JIT覆盖范围
- **下一步（指令优化）**: 直接生成机器码，可获得2-4倍性能提升
- **未来（Optimized JIT）**: 热点代码10-50倍性能提升（未实现）

## MVP说明

当前实现是一个**最小可用版本（MVP）**，主要特点：

### 工作原理
1. **热点检测**: 函数执行达到阈值后触发JIT编译
2. **存根函数调用**: JIT生成的机器码通过调用JitStubs函数来执行操作
3. **自动回退**: 遇到不支持的指令时回退到解释器执行
4. **控制流支持**: 通过标签管理系统支持循环和条件分支

### MVP特性
- **支持控制流**: 可以编译包含循环和条件分支的JavaScript代码
- **性能开销**: 每条指令都需要调用存根函数，有一定开销
- **适用场景**: 适用于大多数JavaScript代码，包括复杂逻辑

### 使用建议
MVP版本可用于：
- 验证JIT架构设计
- 测试基础编译流程
- 处理实际应用代码
- 为后续优化打下基础

**适合在开发和测试环境使用**，在生产环境使用前建议进行性能测试。

## 文件结构

```
multjs/
├── include/mjs/jit/
│   ├── hotness_counter.h       # 热点计数器
│   ├── jit_code.h              # JIT代码封装
│   ├── jit_manager.h           # JIT管理器
│   ├── baseline_compiler.h     # Baseline编译器
│   └── jit_stubs.h             # JIT辅助函数
│
├── src/jit/
│   ├── jit_code.cpp            # JIT代码实现
│   ├── jit_manager.cpp         # JIT管理器实现
│   ├── baseline_compiler.cpp   # Baseline编译器实现
│   └── jit_stubs.cpp           # JIT辅助函数实现
│
└── tests/jit/
    ├── hotness_counter_test.cpp  # 热点计数器测试
    └── baseline_compiler_test.cpp # Baseline编译器集成测试
```

## 开发指南

### 添加新指令编译

1. 在`BaselineCompiler`类中添加对应方法
2. 实现字节码到机器码的转换
3. 添加单元测试
4. 性能测试

### 调试JIT代码

启用详细日志：

```cpp
// 在jit_manager.cpp中设置日志级别
#define JIT_DEBUG 1
```

### 性能分析

使用基准测试工具：

```bash
./build/jit_benchmarks --benchmark_filter=JIT
```

## 参考资料

- [asmjit文档](https://asmjit.github.io/)
- [V8 Ignition解释器](https://v8.dev/blog/ignition-interpreter)
- [V8 TurboFan优化编译器](https://v8.dev/blog/turbofan)

## 贡献

欢迎贡献代码、报告问题或提出建议！

## 许可证

MIT License
