#pragma once

#include "stack_frame.h"

namespace mjs {

class ObjectHeader {

};

// 关于函数的设计
// 函数体就是指令流的封装，只会存放在常量里，定义函数会在常量池创建函数
// 并且会在局部变量表中创建并函数原型，指向函数体，类似语法糖的想法
class FunctionBodyObject : public ObjectHeader {
public:
	explicit FunctionBodyObject(uint32_t t_parCount) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	ByteCode byte_code;
	StackFrame stack_frame;
};


typedef Value(*FunctionBridgeCall)(uint32_t par_count, StackFrame* stack);
class FunctionBridgeObject : public ObjectHeader {
public:
	explicit FunctionBridgeObject(FunctionBridgeCall func_addr) noexcept;

	FunctionBridgeCall func_addr;
};

class FunctionProtoObject : public ObjectHeader {
public:
	explicit FunctionProtoObject(FunctionBodyObject* value) noexcept;
	explicit FunctionProtoObject(FunctionBridgeObject* value) noexcept;

	union {
		FunctionBodyObject* body_val;
		FunctionBridgeObject* bridge_val;
	};
};

class UpObject : public ObjectHeader {
public:
	UpObject(uint32_t t_index, FunctionBodyObject* func_proto) noexcept;

public:
	uint32_t index;
	FunctionBodyObject* func_proto;
};

} // namespace mjs