#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/object.h>

#include "bytecode.h"

namespace mjs {

class FunctionDef {
public:
	explicit FunctionDef(uint32_t par_count) noexcept;
	std::string Disassembly();

public:
	uint32_t par_count;
	uint32_t var_count = 0;
	ByteCode byte_code;

	bool is_generator = false;

	// 优化方向：
	// 如果所有记录都没有捕获外部变量，都是顶级upvalue变量
	// 则closure_value_arr_可以指向栈帧，无需内存分配

	// upvalue变量记录，upvalue变量在当前作用域的索引
	// 如果存在的话，则加载时需要创建FunctionObject
	struct ClosureVarDef {
		// upvalue在closure_value_arr_的索引
		int32_t arr_idx;

		// upvalue指向的变量在父作用域中的变量索引
		std::optional<VarIndex> parent_var_idx;
	};
	// key: upvalue变量索引
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;
};

} // namespace mjs

