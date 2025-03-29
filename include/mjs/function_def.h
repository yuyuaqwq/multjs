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
	uint32_t var_count = 0;		// ����par_count
	ByteCode byte_code;

	bool is_generator = false;

	// �Ż�����
	// ������м�¼��û�в����ⲿ���������Ƕ���upvalue����
	// ��closure_value_arr_����ָ��ջ֡�������ڴ����

	// upvalue������¼��upvalue�����ڵ�ǰ�����������
	// ������ڵĻ��������ʱ��Ҫ����FunctionObject
	struct ClosureVarDef {
		// upvalue��closure_value_arr_������
		int32_t arr_idx;

		// upvalueָ��ı����ڸ��������еı�������
		std::optional<VarIndex> parent_var_idx;
	};
	// key: upvalue��������
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;
};

} // namespace mjs

