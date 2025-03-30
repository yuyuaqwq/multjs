#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <mjs/object.h>

#include "bytecode.h"

namespace mjs {

struct ClosureVarDef {
	// upvalue��closure_value_arr_������
	int32_t arr_idx;

	// upvalueָ��ı����ڸ��������еı�������
	std::optional<VarIndex> parent_var_idx;
};

class FunctionDef : public noncopyable {
public:
	struct VarInfo {
		std::string name;
	};

public:
	explicit FunctionDef(uint32_t par_count) noexcept;

	std::string Disassembly(Context* context);

	void SetGenerator() {
		type_ = Type::kGenerator;
	}

	bool IsGenerator() {
		return type_ == Type::kGenerator;
	}

	bool IsAsync() {
		return type_ == Type::kAsync;
	}

	void AddVar(std::string name) {
		++var_count_;
		var_info_.emplace_back(std::move(name));
	}

	const auto& GetVarInfo(VarIndex idx) const {
		return var_info_.at(idx);
	}

	auto par_count() const { return par_count_; }
	auto var_count() const { return var_count_; }
	const auto& byte_code() const { return byte_code_; }
	auto& byte_code() { return byte_code_; }
	const auto& closure_var_defs() const { return closure_var_defs_; }
	auto& closure_var_defs() { return closure_var_defs_; }

private:
	uint32_t par_count_;
	uint32_t var_count_ = 0;		// ����par_count
	ByteCode byte_code_;

	enum class Type {
		kNormal,
		kGenerator,
		kAsync,
	} type_ = Type::kNormal;

	// �Ż�����
	// ������м�¼��û�в����ⲿ���������Ƕ���upvalue����
	// ��closure_value_arr_����ָ��ջ֡�������ڴ����

	// upvalue������¼��upvalue�����ڵ�ǰ�����������
	// ������ڵĻ��������ʱ��Ҫ����FunctionObject

	// key: upvalue��������
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;

	std::vector<VarInfo> var_info_;
};

} // namespace mjs

