# Doxygen 注释规范指南

## 概述

本文档定义了 JavaScript 引擎项目的 Doxygen 注释规范，确保代码文档的一致性和可维护性。

## 文件头注释

每个头文件(.h)和源文件(.cpp/.c)都应该包含标准的文件头注释：

```cpp
/**
 * @file filename.h
 * @brief 文件的简要描述
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 详细的文件描述，说明文件的主要功能、设计思路和重要特性。
 * 可以包含多行描述，详细说明文件的用途和设计考虑。
 */
```

## 命名空间注释

```cpp
/**
 * @namespace mjs
 * @brief JavaScript 引擎核心命名空间
 *
 * 包含 JavaScript 引擎的所有核心类和功能模块。
 */
namespace mjs {
```

## 类注释

### 基本类注释

```cpp
/**
 * @class ClassName
 * @brief 类的简要描述
 *
 * 类的详细描述，包括：
 * - 类的职责和功能
 * - 设计模式和架构考虑
 * - 重要的使用注意事项
 * - 与其他类的关系
 *
 * @note 重要的注意事项
 * @warning 警告信息
 * @see 相关类或函数
 */
class ClassName : public BaseClass {
```

### 模板类注释

```cpp
/**
 * @class TemplateClass
 * @brief 模板类的简要描述
 *
 * @tparam T 模板参数描述
 * @tparam U 模板参数描述
 */
template<typename T, typename U>
class TemplateClass {
```

## 枚举注释

```cpp
/**
 * @enum ValueType
 * @brief JavaScript 值类型枚举
 *
 * 定义了 JavaScript 引擎支持的所有值类型，包括基本类型和对象类型。
 */
enum class ValueType : uint32_t {
    /** @brief 未定义类型 */
    kUndefined = 0,

    /** @brief 空类型 */
    kNull,

    /** @brief 布尔类型 */
    kBoolean,

    // ... 其他枚举值
};
```

## 函数注释

### 构造函数

```cpp
/**
 * @brief 构造函数
 * @param runtime 运行时环境指针
 * @throw std::invalid_argument 当参数无效时抛出
 */
Context(Runtime* runtime);
```

### 成员函数

```cpp
/**
 * @brief 编译 JavaScript 模块
 *
 * 将 JavaScript 源代码编译为内部字节码表示，并创建模块定义。
 *
 * @param module_name 模块名称
 * @param script JavaScript 源代码
 * @return 编译后的模块值
 * @throw CompileError 编译错误时抛出
 * @note 模块名称必须是唯一的
 */
Value CompileModule(std::string module_name, std::string_view script);
```

### 模板函数

```cpp
/**
 * @brief 调用 JavaScript 函数
 *
 * 使用迭代器范围作为参数调用指定的 JavaScript 函数。
 *
 * @tparam It 迭代器类型，必须满足随机访问迭代器要求
 * @param stack_frame 当前栈帧指针
 * @param func_val 要调用的函数值
 * @param this_val this 上下文值
 * @param begin 参数起始迭代器
 * @param end 参数结束迭代器
 * @return 函数执行结果
 * @throw RuntimeError 运行时错误时抛出
 */
template<typename It>
Value CallFunction(StackFrame* stack_frame, Value func_val, Value this_val, It begin, It end);
```

### 内联函数

```cpp
/**
 * @brief 获取运行时环境引用
 * @return 运行时环境常量引用
 */
auto& runtime() const { return *runtime_; }
```

## 变量注释

### 成员变量

```cpp
private:
    Runtime* runtime_;              ///< 执行上下文指针
    LocalConstPool local_const_pool_; ///< 本地常量池
    VM vm_;                         ///< 虚拟机实例
    JobQueue microtask_queue_;      ///< 微任务队列
```

### 全局变量

```cpp
/** @brief 全局符号表最大容量 */
constexpr size_t kMaxSymbolTableSize = 1024;
```

## 特殊注释标签

### @note
用于重要的实现说明或注意事项：
```cpp
/**
 * @note 新对象类型必须添加到 IsObject() 方法中，否则会导致内存泄漏
 */
```

### @warning
用于警告信息：
```cpp
/**
 * @warning 此函数不是线程安全的，调用者需要确保同步
 */
```

### @deprecated
用于标记已弃用的功能：
```cpp
/**
 * @deprecated 使用新版本的 FunctionCall 替代
 */
```

### @see
用于引用相关文档：
```cpp
/**
 * @see ValueType 枚举定义
 * @see Object 基类
 */
```

### @throw
用于异常说明：
```cpp
/**
 * @throw std::runtime_error 当值类型不匹配时抛出
 */
```

## 注释格式规范

### 缩进和对齐
- 注释使用 4 空格缩进
- 参数描述对齐
- 多行注释保持一致的缩进

### 语言风格
- 使用中文进行注释
- 技术术语保持英文原文
- 描述清晰简洁，避免歧义

### 标点符号
- 中文使用全角标点
- 英文使用半角标点
- 句子末尾使用句号

## 示例

### 完整的类定义示例

```cpp
/**
 * @class Context
 * @brief JavaScript 执行上下文管理器
 *
 * 负责管理 JavaScript 代码的执行环境，包括：
 * - 模块编译和执行
 * - 函数调用栈管理
 * - 常量池管理
 * - 垃圾回收协调
 *
 * @note 每个 Context 实例都是独立的执行环境
 * @warning Context 不是线程安全的
 * @see Runtime 运行时环境
 * @see VM 虚拟机
 */
class Context : public noncopyable {
public:
    /**
     * @brief 构造函数
     * @param runtime 运行时环境指针
     * @throw std::invalid_argument 当 runtime 为 nullptr 时抛出
     */
    Context(Runtime* runtime);

    /**
     * @brief 编译并执行 JavaScript 代码
     *
     * 将源代码编译为字节码并立即执行，适用于一次性脚本执行。
     *
     * @param module_name 模块名称
     * @param script JavaScript 源代码
     * @return 执行结果值
     * @throw CompileError 编译错误时抛出
     * @throw RuntimeError 运行时错误时抛出
     */
    Value Eval(std::string module_name, std::string_view script);

    /**
     * @brief 获取运行时环境引用
     * @return 运行时环境常量引用
     */
    auto& runtime() const { return *runtime_; }

private:
    Runtime* runtime_;              ///< 执行上下文指针
    LocalConstPool local_const_pool_; ///< 本地常量池
    VM vm_;                         ///< 虚拟机实例
    JobQueue microtask_queue_;      ///< 微任务队列
};
```

## 工具支持

### Doxygen 配置
在项目的 Doxyfile 中启用以下选项：
```
JAVADOC_AUTOBRIEF   = YES
QT_AUTOBRIEF        = YES
MULTILINE_CPP_IS_BRIEF = YES
```

### IDE 配置
在 Visual Studio 或 CLion 中启用 Doxygen 支持，配置相应的代码片段模板。

## 维护

- 每次修改代码时同步更新注释
- 定期检查注释的准确性和完整性
- 使用 Doxygen 生成文档并验证格式

---

*最后更新: 2025-10-08*