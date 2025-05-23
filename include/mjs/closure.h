#pragma once

#include <mjs/reference_counter.h>
#include <mjs/object.h>

namespace mjs {

// �������ѵı���
class ClosureVar : public ReferenceCounter<ClosureVar> {
public:
	ClosureVar(Value&& value) 
		: value_(std::move(value))
	{
		assert(!value_.IsClosureVar());
	}

	~ClosureVar() = default;

	Value& value() { return value_; }
	const Value& value() const { return value_; }

public:
	// ����ѭ������
	Value value_;
};

// �հ�������¼��Value: ָ��ǰ��������ıհ�����
// Ҳ���Ըĳ�ClosureVar*���ֶ�����Reference��Dereference�����Խ�ʡһЩ�ռ�
class ClosureEnvironment {
public:
	void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
		for (auto& var : closure_var_refs_) {
			callback(context, list, var);
		}
		callback(context, list, lexical_this_);
	}

	const auto& closure_var_refs() const { return closure_var_refs_; }
	auto& closure_var_refs() { return closure_var_refs_; }

	const auto& lexical_this() const { return lexical_this_; }
	void set_lexical_this(Value&& lexical_this) { lexical_this_ = lexical_this; }

private:
	// ClosureVar*
	std::vector<Value> closure_var_refs_;

	// �ʷ������򲶻��this
	Value lexical_this_;
};


struct ClosureVarDef {
	// �ñ�����ClosureEnvironment::vars_������
	uint32_t env_var_idx;

	// �ڸ��������еı�������
	VarIndex parent_var_idx;
};

class ClosureVarTable {
public:
	void AddClosureVar(VarIndex var_idx, VarIndex parent_var_idx) {
		closure_var_defs_.emplace(var_idx,
			ClosureVarDef{
				.env_var_idx = uint32_t(closure_var_defs_.size()),
				.parent_var_idx = parent_var_idx,
			}
		);
	}

	auto& closure_var_defs() { return closure_var_defs_; }
	const auto& closure_var_defs() const { return closure_var_defs_; }

private:
	std::unordered_map<VarIndex, ClosureVarDef> closure_var_defs_;		// ������ⲿ����
};

} // namespace mjs