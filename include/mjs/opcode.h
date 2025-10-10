/**
 * @file opcode.h
 * @brief JavaScript 字节码操作码定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的字节码操作码系统，包括所有
 * 操作码类型枚举、操作码信息结构和相关类型别名。
 */

#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include <mjs/constant.h>
#include <mjs/variable.h>

namespace mjs {

/**
 * @enum OpcodeType
 * @brief 操作码类型枚举
 *
 * 定义了 JavaScript 引擎支持的所有字节码操作码类型，包括：
 * - 常量加载指令
 * - 变量加载/存储指令
 * - 属性操作指令
 * - 算术运算指令
 * - 位运算指令
 * - 控制流指令
 * - 函数调用指令
 * - 异步操作指令
 * - 异常处理指令
 */
enum class OpcodeType : uint8_t {
	// 常量加载指令
	kCLoad_0 = 0x03,  ///< 加载常量0
	kCLoad_1 = 0x04,  ///< 加载常量1
	kCLoad_2 = 0x05,  ///< 加载常量2
	kCLoad_3 = 0x06,  ///< 加载常量3
	kCLoad_4 = 0x07,  ///< 加载常量4
	kCLoad_5 = 0x08,  ///< 加载常量5

	kCLoad = 0x12,    ///< 加载常量
	kCLoadW = 0x13,   ///< 加载常量（宽索引）
	kCLoadD = 0x14,   ///< 加载常量（双索引）

	// 变量加载指令
	kVLoad = 0x15,    ///< 加载变量
	kVLoad_0 = 0x1a,  ///< 加载变量0
	kVLoad_1 = 0x1b,  ///< 加载变量1
	kVLoad_2 = 0x1c,  ///< 加载变量2
	kVLoad_3 = 0x1d,  ///< 加载变量3

	kGetGlobal = 0x1e, ///< 获取全局变量

	// 模块操作指令
	kGetModule = 0x20,      ///< 获取模块
	kGetModuleAsync = 0x21, ///< 异步获取模块
	kClosure = 0x22,        ///< 创建闭包

	// 变量存储指令
	kVStore = 0x36,   ///< 存储变量
	kVStore_0 = 0x3b, ///< 存储变量0
	kVStore_1 = 0x3c, ///< 存储变量1
	kVStore_2 = 0x3d, ///< 存储变量2
	kVStore_3 = 0x3e, ///< 存储变量3

	// 属性操作指令
	kPropertyLoad = 0x40,  ///< 加载属性
	kPropertyStore = 0x41, ///< 存储属性

	// 索引操作指令
	kIndexedLoad = 0x48,   ///< 索引加载
	kIndexedStore = 0x49,  ///< 索引存储

	// 栈操作指令
	kPop = 0x50,       ///< 弹出栈顶元素
	kDump = 0x51,      ///< 转储栈内容
	kSwap = 0x52,      ///< 交换栈顶元素
	kUndefined = 0x53, ///< 压入 undefined

	kToString = 0x58,  ///< 转换为字符串

	// 算术运算指令
	kAdd = 0x60, ///< 加法运算
	kInc = 0x61, ///< 递增运算
	kSub = 0x64, ///< 减法运算
	kMul = 0x68, ///< 乘法运算
	kDiv = 0x6c, ///< 除法运算

	// 位运算和移位指令
	kShl = 0x71,    ///< 左移位运算
	kShr = 0x72,    ///< 右移位运算
	kUShr = 0x73,   ///< 无符号右移位运算
	kBitAnd = 0x74, ///< 按位与运算
	kBitOr = 0x75,  ///< 按位或运算
	kBitXor = 0x76, ///< 按位异或运算
	kBitNot = 0x77, ///< 按位取反运算

	// 取反指令
	kNeg = 0x70, ///< 取反运算

	// 比较指令
	kEq = 0x99, ///< 等于比较
	kNe = 0x9a, ///< 不等于比较
	kLt = 0x9b, ///< 小于比较
	kGe = 0x9c, ///< 大于等于比较
	kGt = 0x9d, ///< 大于比较
	kLe = 0x9e, ///< 小于等于比较

	kIfEq = 0xa0, ///< 条件跳转（等于）

	// 控制流指令
	kGoto = 0xa7, ///< 无条件跳转

	// 返回指令
	kReturn = 0xb1, ///< 函数返回

	// 方法调用指令
	kFunctionCall = 0xb8,   ///< 函数调用
	kGetThis = 0xb9,       ///< 获取 this
	kGetOuterThis = 0xba,  ///< 获取外部 this

	// 异步操作指令
	kYield = 0xc0,           ///< 生成器 yield
	kGeneratorReturn = 0xc1, ///< 生成器返回
	kAwait = 0xc2,           ///< 异步等待
	kAsyncReturn = 0xc3,     ///< 异步返回

	// 对象创建指令
	kNew = 0xc8, ///< 创建新对象

	// 异常处理指令
	kTryBegin = 0xd0,      ///< 异常处理开始
	kThrow = 0xd3,         ///< 抛出异常
	kTryEnd = 0xd4,        ///< 异常处理结束
	kFinallyReturn = 0xd5, ///< finally 块返回
	kFinallyGoto = 0xd6,   ///< finally 块跳转

	// 保留操作码范围
	// 0xf0 ~ 0xff 保留
};

/**
 * @struct OpcodeInfo
 * @brief 操作码信息结构体
 *
 * 存储操作码的相关信息，包括操作码字符串表示和参数大小列表。
 */
struct OpcodeInfo {
	std::string str;                ///< 操作码字符串表示
	std::vector<char> par_size_list; ///< 参数大小列表
};

using Pc = uint32_t;        ///< 程序计数器类型
using PcOffset = uint16_t;  ///< 程序计数器偏移量类型

constexpr Pc kInvalidPc = 0xffffffff; ///< 无效程序计数器值

} // namespace mjs