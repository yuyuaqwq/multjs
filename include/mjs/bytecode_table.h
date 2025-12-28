/**
 * @file bytecode.h
 * @brief JavaScript 字节码表系统
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的字节码表系统，包括字节码表的
 * 管理、编码、解码和反汇编等功能，支持 JavaScript 代码的编译和执行。
 */

#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include <mjs/noncopyable.h>
#include <mjs/constant.h>
#include <mjs/variable.h>
#include <mjs/opcode.h>

namespace mjs {

/**
 * @brief 操作码类型加法运算符重载
 * @param a 操作码类型
 * @param b 偏移量
 * @return 偏移后的操作码类型
 */
inline static OpcodeType operator+(OpcodeType a, size_t b) {
	return static_cast<OpcodeType>(static_cast<size_t>(a) + b);
}

/**
 * @brief 操作码类型减法运算符重载
 * @param a 操作码类型
 * @param b 操作码类型
 * @return 操作码类型差值
 */
inline static size_t operator-(OpcodeType a, OpcodeType b) {
	return static_cast<size_t>(a) - static_cast<size_t>(b);
}

class Context;
class FunctionDefBase;

/**
 * @class BytecodeTable
 * @brief 字节码表类
 *
 * 管理 JavaScript 字节码的存储、编码、解码和反汇编功能。
 * 继承自 noncopyable 确保单例特性。
 *
 * @note 提供完整的字节码操作接口，支持各种字节码指令的生成和解析
 * @see OpcodeType 操作码类型枚举
 * @see FunctionDefBase 函数定义基类
 */
class BytecodeTable : public noncopyable {
public:
	/**
	 * @brief 获取指定位置的字节码操作码
	 * @param pc 程序计数器位置
	 * @return 操作码类型
	 */
	OpcodeType GetOpcode(Pc pc) const;

	/**
	 * @brief 获取程序计数器位置
	 * @param pc 程序计数器指针
	 * @return 程序计数器位置
	 */
	Pc GetPc(Pc* pc) const;

	/**
	 * @brief 获取变量索引
	 * @param pc 程序计数器指针
	 * @return 变量索引
	 */
	VarIndex GetVarIndex(Pc* pc);

	/**
	 * @brief 获取常量索引
	 * @param pc 程序计数器指针
	 * @return 常量索引
	 */
	ConstIndex GetConstIndex(Pc* pc);

	/**
	 * @brief 发射操作码
	 * @param opcode 操作码类型
	 */
	void EmitOpcode(OpcodeType opcode);

	/**
	 * @brief 发射程序计数器偏移量
	 * @param offset 程序计数器偏移量
	 */
	void EmitPcOffset(PcOffset offset);

	/**
	 * @brief 发射变量索引
	 * @param idx 变量索引
	 */
	void EmitVarIndex(VarIndex idx);

	/**
	 * @brief 发射常量索引
	 * @param idx 常量索引
	 */
	void EmitConstIndex(ConstIndex idx);

	/**
	 * @brief 发射常量加载指令
	 *
	 * 根据常量索引的范围选择最优的常量加载指令：
	 * - 索引 0-5：使用专用指令 kCLoad_0 到 kCLoad_5
	 * - 索引 -128 到 127：使用单字节指令 kCLoad
	 * - 索引 -32768 到 32767：使用双字节指令 kCLoadW
	 * - 其他情况：使用四字节指令 kCLoadD
	 *
	 * @param idx 常量索引
	 */
	void EmitConstLoad(ConstIndex idx);

	/**
	 * @brief 发射变量存储指令
	 * @param idx 变量索引
	 */
	void EmitVarStore(VarIndex idx);

	/**
	 * @brief 发射变量加载指令
	 * @param idx 变量索引
	 */
	void EmitVarLoad(VarIndex idx);

	/**
	 * @brief 发射跳转指令
	 */
	void EmitGoto();

	/**
	 * @brief 发射属性加载指令
	 * @param const_idx 常量索引
	 */
	void EmitPropertyLoad(ConstIndex const_idx);

	/**
	 * @brief 发射属性存储指令
	 * @param const_idx 常量索引
	 */
	void EmitPropertyStore(ConstIndex const_idx);

	/**
	 * @brief 发射索引加载指令
	 */
	void EmitIndexedLoad();

	/**
	 * @brief 发射索引存储指令
	 */
	void EmitIndexedStore();

	/**
	 * @brief 发射返回指令
	 * @param function_def 函数定义指针
	 */
	void EmitReturn(FunctionDefBase* function_def);

	/**
	 * @brief 修复操作码
	 * @param opcode_pc 操作码位置
	 * @param op 新的操作码类型
	 */
	void RepairOpcode(Pc opcode_pc, OpcodeType op);

	/**
	 * @brief 修复程序计数器
	 * @param pc_from 源程序计数器位置
	 * @param pc_to 目标程序计数器位置
	 */
	void RepairPc(Pc pc_from, Pc pc_to);

	/**
	 * @brief 计算程序计数器位置
	 * @param cur_pc 当前程序计数器位置
	 * @return 计算后的程序计数器位置
	 */
	Pc CalcPc(Pc cur_pc) const;

	/**
	 * @brief 反汇编字节码
	 *
	 * 将字节码转换为可读的汇编指令格式，用于调试和开发目的。
	 * 输出格式包括：程序计数器位置、操作码名称、参数值和符号信息。
	 *
	 * @param context 执行上下文指针
	 * @param pc 程序计数器引用（输入当前PC，输出下一指令PC）
	 * @param opcode 操作码引用（输出当前操作码）
	 * @param param 参数引用（输出当前参数）
	 * @param func_def 函数定义指针
	 * @return 反汇编后的字符串表示
	 */
	std::string Disassembly(Context* context, Pc& pc, OpcodeType& opcode, uint32_t& param, const FunctionDefBase* func_def) const;

	/**
	 * @brief 获取字节码表大小
	 * @return 字节码表大小
	 */
	Pc Size() const { return bytes_.size(); }

	/**
	 * @brief 获取8位有符号整数
	 * @param pc 程序计数器位置
	 * @return 8位有符号整数
	 */
	int8_t GetI8(Pc pc) const;

	/**
	 * @brief 获取8位无符号整数
	 * @param pc 程序计数器位置
	 * @return 8位无符号整数
	 */
	uint8_t GetU8(Pc pc) const;

	/**
	 * @brief 获取16位有符号整数
	 * @param pc 程序计数器位置
	 * @return 16位有符号整数
	 */
	int16_t GetI16(Pc pc) const;

	/**
	 * @brief 获取16位无符号整数
	 * @param pc 程序计数器位置
	 * @return 16位无符号整数
	 */
	uint16_t GetU16(Pc pc) const;

	/**
	 * @brief 获取32位有符号整数
	 * @param pc 程序计数器位置
	 * @return 32位有符号整数
	 */
	int32_t GetI32(Pc pc) const;

	/**
	 * @brief 获取32位无符号整数
	 * @param pc 程序计数器位置
	 * @return 32位无符号整数
	 */
	uint32_t GetU32(Pc pc) const;

	/**
	 * @brief 发射8位有符号整数
	 * @param val 8位有符号整数
	 */
	void EmitI8(int8_t val);

	/**
	 * @brief 发射8位无符号整数
	 * @param val 8位无符号整数
	 */
	void EmitU8(uint8_t val);

	/**
	 * @brief 发射16位有符号整数
	 * @param val 16位有符号整数
	 */
	void EmitI16(int16_t val);

	/**
	 * @brief 发射16位无符号整数
	 * @param val 16位无符号整数
	 */
	void EmitU16(uint16_t val);

	/**
	 * @brief 发射32位有符号整数
	 * @param val 32位有符号整数
	 */
	void EmitI32(int32_t val);

	/**
	 * @brief 发射32位无符号整数
	 * @param val 32位无符号整数
	 */
	void EmitU32(uint32_t val);

	/**
	 * @brief 获取操作码类型映射表
	 * @return 操作码类型映射表常量引用
	 */
	static const std::unordered_map<OpcodeType, OpcodeInfo>& opcode_type_map();

private:
	/**
	 * @brief 获取字节码指针
	 * @param pc 程序计数器位置
	 * @return 字节码指针
	 */
	uint8_t* GetPtr(Pc pc);

	/**
	 * @brief 获取字节码常量指针
	 * @param pc 程序计数器位置
	 * @return 字节码常量指针
	 */
	const uint8_t* GetPtr(Pc pc) const;

private:
	std::vector<uint8_t> bytes_; ///< 字节码存储向量
};

} // namespace mjs