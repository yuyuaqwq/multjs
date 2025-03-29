#include <iostream>

#include <mjs/vm.h>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/opcode_type.h>
#include <mjs/array_object.h>
#include <mjs/function_object.h>
#include <mjs/generator_object.h>

namespace mjs {

Vm::Vm(Context* context)
	: context_(context)
	, stack_frame_(&stack()) {}

Stack& Vm::stack() {
	return context_->runtime().stack();
}

void Vm::EvalFunction(const Value& func_val) {
	pc_ = 0;
	cur_func_val_ = func_val;
	stack().Resize(function_def(cur_func_val_)->var_count);

	// �����ĺ�������ͨ��CLoadFunc���أ�������Ҫ�Լ���ʼ��
	FunctionDefLoadInit(&cur_func_val_);
	FunctionEnterInit(cur_func_val_);

	Run();
}

FunctionDef* Vm::function_def(const Value& func_val) const {
	if (func_val.IsFunctionObject()) {
		return func_val.function()->func_def_;
	}
	else if  (func_val.IsFunctionDef()) {
		return func_val.function_def();
	}
	return nullptr;
}

bool Vm::FunctionDefLoadInit(Value* func_def_val) {
	auto func_def = func_def_val->function_def();
	if (func_def->closure_var_defs_.empty()) {
		return false;
	}

	*func_def_val = Value(new FunctionObject(func_def));
	
	auto func = func_def_val->function();
	// func->upvalues_.resize(func_def->closure_var_defs_.size());

	auto& arr = func->closure_value_arr_;
	arr.resize(func_def->closure_var_defs_.size());

	return true;
}

void Vm::FunctionEnterInit(const Value& func_val) {
	FunctionObject* func = nullptr;
	FunctionDef* func_def = nullptr;
	if (func_val.IsFunctionObject()) {
		func = func_val.function();
		func_def = func->func_def_;
	}
	//if (func_val.IsGeneratorObject()) {
	//	func = func_val.generator();
	//	func_def = func->func_def_;
	//}

	if (func == nullptr || func_def == nullptr) {
		return;
	}

	// ���õ��Ǻ������󣬿�����Ҫ����հ��ڵ�upvalue
	auto& arr = func->closure_value_arr_;
	for (auto& def : func_def->closure_var_defs_) {
		// ջ�ϵĶ���ͨ��upvalue�������հ�����
		stack_frame_.Set(def.first, Value(
			UpValue(&arr[def.second.arr_idx])
		));
	}
}

Value& Vm::GetVar(VarIndex idx) {
	auto* var = &stack_frame_.Get(idx);
	while (var->IsUpValue()) {
		var = var->up_value().value;
	}
	return *var;
}

void Vm::SetVar(VarIndex idx, Value&& var) {
	auto* var_ = &stack_frame_.Get(idx);
	while (var_->IsUpValue()) {
		var_ = var_->up_value().value;
	}
	*var_ = std::move(var);
}

const Value& Vm::GetGlobalConst(ConstIndex idx) {
	auto& var = context_->runtime().const_pool().Get(idx);
	return var;
}

//const Value& Vm::GetLocalConst(ConstIndex idx) {
//	auto& var = context_->const_pool().Get(idx);
//	return var;
//}

const Value& Vm::GetConst(ConstIndex idx) {
	if (IsGlobalConstIndex(idx)) {
		return GetGlobalConst(idx);
	}
	//else if (IsLocalConstIndex(idx)) {
	//	return GetLocalConst(idx);
	//}
	else {
		throw VmException("Incorrect const index.");
	}
}


void Vm::LoadConst(ConstIndex const_idx) {
	auto& value = GetGlobalConst(const_idx);

	if (value.IsFunctionDef()) {
		auto func_val = value;
		if (FunctionDefLoadInit(&func_val)) {
			// array��Ҫ��ʼ��Ϊupvalue��ָ�򸸺�����ArrayValue
			// ���û�и������Ͳ���Ҫ�����Ƕ��㺯����Ĭ�ϳ�ʼ��Ϊδ����

			auto func = func_val.function();
			auto parent_func = cur_func_val_.function();

			// ���������������ü����������ӳ��������е�ArrayValue����������
			func->parent_function_ = cur_func_val_;

			// ���õ���������ArrayValue
			auto& arr = func->closure_value_arr_;
			auto& parent_arr = parent_func->closure_value_arr_;

			for (auto& def : func->func_def_->closure_var_defs_) {
				if (!def.second.parent_var_idx) {
					// ��ǰ�Ƕ�������
					continue;
				}
				auto parent_arr_idx = parent_func->func_def_->closure_var_defs_[*def.second.parent_var_idx].arr_idx;
				arr[def.second.arr_idx] = Value(UpValue(&parent_arr[parent_arr_idx]));
			}
		}
		stack_frame_.Push(std::move(func_val));
		return;
	}

	stack_frame_.Push(value);
}


void Vm::Run() {
	auto cur_func_def = function_def(cur_func_val_);
	if (!cur_func_def) return;

	do {
		// auto pc = pc_; std::cout << cur_func_def->byte_code.Disassembly(pc) << std::endl;
		auto opcode = cur_func_def->byte_code.GetOpcode(pc_++);
		switch (opcode) {
		//case OpcodeType::kStop:
		//	return;
		case OpcodeType::kCLoad_0: {
		case OpcodeType::kCLoad_1:
		case OpcodeType::kCLoad_2:
		case OpcodeType::kCLoad_3:
		case OpcodeType::kCLoad_4:
		case OpcodeType::kCLoad_5:
			LoadConst(opcode - OpcodeType::kCLoad_0);
			break;
		}
		case OpcodeType::kCLoad: {
			auto const_idx = cur_func_def->byte_code.GetI8(pc_);
			pc_ += 1;
			LoadConst(const_idx);
			break;
		}
		case OpcodeType::kCLoadW: {
			auto const_idx = cur_func_def->byte_code.GetI16(pc_);
			pc_ += 2;
			LoadConst(const_idx);
			break;
		}
		case OpcodeType::kCLoadD: {
			auto const_idx = cur_func_def->byte_code.GetI32(pc_);
			pc_ += 4;
			LoadConst(const_idx);
			break;
		}
		case OpcodeType::kVLoad: {
			auto var_idx = cur_func_def->byte_code.GetU8(pc_);
			pc_ += 1;
			stack_frame_.Push(GetVar(var_idx));
			break;
		}
		case OpcodeType::kVLoad_0:
		case OpcodeType::kVLoad_1:
		case OpcodeType::kVLoad_2: 
		case OpcodeType::kVLoad_3: {
			auto var_idx = opcode - OpcodeType::kVLoad_0;
			stack_frame_.Push(GetVar(var_idx));
			break;
		}
		case OpcodeType::kPop: {
			stack_frame_.Pop();
			break;
		}
		case OpcodeType::kVStore: {
			auto var_idx = cur_func_def->byte_code.GetU8(pc_);
			pc_ += 1;
			SetVar(var_idx, stack_frame_.Pop());
			break;
		}
		case OpcodeType::kVStore_0:
		case OpcodeType::kVStore_1:
		case OpcodeType::kVStore_2:
		case OpcodeType::kVStore_3: {
			auto var_idx = opcode - OpcodeType::kVStore_0;
			SetVar(var_idx, stack_frame_.Pop());
			break;
		}
		case OpcodeType::kPropertyLoad: {
			auto key_val = stack_frame_.Pop();

			auto& obj_val = stack_frame_.Get(-1);

			this_val_ = obj_val;

			if (obj_val.IsObject()) {
				auto& obj = obj_val.object();
				auto prop = obj.GetProperty(key_val);
				if (!prop) {
					obj_val = Value();
				}
				else {
					obj_val = *prop;
				}
			}
			else {
				// ��Object���ͣ���������������
				// ��undefined��Ҫ����
				// number����Ҫת����ʱNumber Object

				obj_val = Value();
			}
			
			break;
		}
		case OpcodeType::kPropertyStore: {
			auto key_val = stack_frame_.Pop();

			auto obj_val = stack_frame_.Pop();
			if (obj_val.IsObject()) {
				auto& obj = obj_val.object();
				obj.SetProperty(key_val, stack_frame_.Pop());
			}
			else {
				// ��Object���ͣ���������������
				// ��undefined��Ҫ����
				// number����Ҫת����ʱNumber Object
			}
			
			break;
		}
		case OpcodeType::kIndexedLoad: {
			auto idx_val = stack_frame_.Pop();
			idx_val = idx_val.ToString();

			auto& obj_val = stack_frame_.Get(-1);
			auto& obj = obj_val.object();
			
			auto prop = obj.GetProperty(idx_val);
			if (!prop) {
				obj_val = Value();
			}
			else {
				obj_val = *prop;
			}
			break;
		}
		case OpcodeType::kIndexedStore: {
			auto idx_val = stack_frame_.Pop();
			idx_val = idx_val.ToString();

			auto obj_val = stack_frame_.Pop();
			auto& obj = obj_val.object();

			obj.SetProperty(idx_val, stack_frame_.Pop());
			break;
		}
		case OpcodeType::kAdd: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = b + a;
			break;
		}
		case OpcodeType::kSub: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = b - a;
			break;
		}
		case OpcodeType::kMul: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = b * a;
			break;
		}
		case OpcodeType::kDiv: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = b / a;
			break;
		}
		case OpcodeType::kNeg: {
			auto& a = stack_frame_.Get(-1);
			a = -a;
			break;
		}		 
		case OpcodeType::kFunctionCall: {
			//auto var_idx = cur_func_def->byte_code.GetU16(pc_);
			//pc_ += 2;

			// auto& func_val = GetVar(var_idx);

			auto func_val = stack_frame_.Pop();
			FunctionSwitch(&cur_func_def, func_val);
			
			break;
		}
		case OpcodeType::kReturn: {
			// ��Ҫ���أ���Ҫ������ǰջ֡�ϵıհ�����������
			// �õ�����֮��ĺ������������丳ֵΪ

			auto ret_value = stack_frame_.Pop();

			auto save_bottom = stack_frame_.Pop();
			auto save_pc = stack_frame_.Pop();
			auto save_func = stack_frame_.Pop();

			// �ָ�������ջ֡
			cur_func_val_ = save_func;
			cur_func_def = function_def(cur_func_val_);
			pc_ = save_pc.u64();

			// ����ջ����ջ��
			stack().Resize(stack_frame_.bottom());
			stack_frame_.set_bottom(save_bottom.u64());

			// ����λ��ԭ��λ��ջ֡ջ���ķ���ֵ
			stack_frame_.Push(ret_value);
			break;
		}
		case OpcodeType::kGeneratorReturn: {
			this_val_.generator()->SetClosed();

			auto ret_value = stack_frame_.Pop();

			auto save_bottom = stack_frame_.Pop();
			auto save_pc = stack_frame_.Pop();
			auto save_func = stack_frame_.Pop();

			// �ָ�������ջ֡
			cur_func_val_ = save_func;
			cur_func_def = function_def(cur_func_val_);
			pc_ = save_pc.u64();

			// ����ջ����ջ��
			stack().Resize(stack_frame_.bottom());
			stack_frame_.set_bottom(save_bottom.u64());

			// ����λ��ԭ��λ��ջ֡ջ���ķ���ֵ
			auto ret_obj = Value(new Object());
			ret_obj.object().SetProperty(Value("value"), std::move(ret_value));
			ret_obj.object().SetProperty(Value("done"), Value(true));
			stack_frame_.Push(ret_obj);
			break;
		}
		case OpcodeType::kYield: {
			// ����һ������
			// { value: $_, done: $boolean }

			// Ҫ�ӵ�ǰ�������ķ���

			this_val_.generator()->set_pc(pc_);

			auto ret_value = stack_frame_.Pop();

			auto save_bottom = stack_frame_.Pop();
			auto save_pc = stack_frame_.Pop();
			auto save_func = stack_frame_.Pop();

			cur_func_val_ = save_func;
			cur_func_def = function_def(cur_func_val_);
			pc_ = save_pc.u64();

			stack().Resize(stack_frame_.bottom());
			stack_frame_.set_bottom(save_bottom.u64());

			// ����λ��ԭ��λ��ջ֡ջ���ķ���ֵ
			auto ret_obj = Value(new Object());
			ret_obj.object().SetProperty(Value("value"), std::move(ret_value));
			ret_obj.object().SetProperty(Value("done"), Value(false));
			stack_frame_.Push(ret_obj);

			break;
		}
		case OpcodeType::kNe: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(!(b == a));
			break;
		}
		case OpcodeType::kEq: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(b == a);
			break;
		}
		case OpcodeType::kLt: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(b < a);
			break;
		}
		case OpcodeType::kLe: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(!(b > a));
			break;
		}
		case OpcodeType::kGt: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(b > a);
			break;
		}
		case OpcodeType::kGe: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = Value(!(b < a));
			break;
		}
		case OpcodeType::kIfEq: {
			auto boolean_val = stack_frame_.Pop().ToBoolean();
			if (boolean_val.boolean() == false) {
				pc_ = cur_func_def->byte_code.CalcPc(--pc_);
			}
			else {
				pc_ += 2;
			}
			break;
		}
		case OpcodeType::kGoto: {
			pc_ = cur_func_def->byte_code.CalcPc(--pc_);
			break;
		}
		default:
			throw VmException("Unknown instruction");
		}
	} while (pc_ >= 0 && pc_ < cur_func_def->byte_code.Size());
}

void Vm::FunctionSwitch(FunctionDef** cur_func_def, const Value& func_val) {
	auto par_count = stack_frame_.Pop().u64();

	switch (func_val.type()) {
	case ValueType::kFunctionDef:
	case ValueType::kFunctionObject: {
		auto func_def = function_def(func_val);

		if (func_def->is_generator) {
			// ������������
			// ֱ�ӷ�������������
			stack_frame_.Push(Value(new GeneratorObject(context_->runtime(), func_val)));
			return;
		}

		// printf("%s\n", func_def->Disassembly().c_str());

		if (par_count < func_def->par_count) {
			throw VmException("Wrong number of parameters passed when calling the function");
		}

		auto save_func = cur_func_val_;
		auto save_pc = pc_;
		auto save_bottom = stack_frame_.bottom();

		// ջ֡���֣�
		// ������Ϣ
		// �ֲ�����
		// ����1
		// ...
		// ����n    <- bottom
		// ��һջ֡

		// �Ѷ���Ĳ�������
		stack().Reduce(par_count - func_def->par_count);

		// �л�������ջ֡
		*cur_func_def = func_def;
		cur_func_val_ = func_val;
		pc_ = 0;
		assert(stack().Size() >= (*cur_func_def)->par_count);
		stack_frame_.set_bottom(stack().Size() - (*cur_func_def)->par_count);

		// �����Ѿ���ջ�ˣ��ٷ���ֲ���������
		assert((*cur_func_def)->var_count >= (*cur_func_def)->par_count);
		stack().Upgrade((*cur_func_def)->var_count - (*cur_func_def)->par_count);

		// ���浱ǰ����������Ret����
		stack_frame_.Push(Value(save_func));
		stack_frame_.Push(Value(save_pc));
		stack_frame_.Push(Value(save_bottom));

		if (cur_func_val_.IsFunctionObject()) {
			FunctionEnterInit(cur_func_val_);
		}

		break;
	}
	case ValueType::kCppFunction: {
		// �л�ջ֡
		auto old_bottom = stack_frame_.bottom();
		stack_frame_.set_bottom(stack().Size() - par_count);

		auto ret = func_val.cpp_function()(context_, par_count, &stack_frame_);

		// ��ԭջ֡
		stack_frame_.set_bottom(old_bottom);
		stack().Reduce(par_count);
		stack_frame_.Push(std::move(ret));
		break;
	}
	case ValueType::kGeneratorNext: {
		auto generator = this_val_.generator();
		if (generator->IsClosed()) {
			auto ret_obj = Value(new Object());
			ret_obj.object().SetProperty(Value("value"), Value());
			ret_obj.object().SetProperty(Value("done"), Value(true));
			stack_frame_.Push(ret_obj);
			break;
		}

		generator->SetExecuting();

		auto save_func = cur_func_val_;
		auto save_pc = pc_;
		auto save_bottom = stack_frame_.bottom();

		// ����ջ֡
		auto& gen_vector = generator->stack().vector();
		
		stack().vector().insert(stack().vector().end(), gen_vector.begin(), gen_vector.end());

		// �л�������ջ֡
		*cur_func_def = generator->function_def();
		cur_func_val_ = generator->function();
		pc_ = generator->pc();
		stack_frame_.set_bottom(stack().Size());

		// ���浱ǰ����������Yield����
		stack_frame_.Push(Value(save_func));
		stack_frame_.Push(Value(save_pc));
		stack_frame_.Push(Value(save_bottom));

		if (cur_func_val_.IsFunctionObject()) {
			FunctionEnterInit(cur_func_val_);
		}

		break;
	}
	default:
		throw VmException("Non callable types.");
	}
}

} // namespace mjs