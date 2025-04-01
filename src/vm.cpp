#include <iostream>

#include <mjs/vm.h>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/opcode.h>
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
	stack().resize(function_def(cur_func_val_).var_count());

	// �����ĺ�������ͨ��CLoadFunc���أ�������Ҫ�Լ���ʼ��
	FunctionDefLoadInit(&cur_func_val_);
	FunctionEnterInit(cur_func_val_);

	Run();
}

FunctionDef& Vm::function_def(const Value& func_val) const {
	if (func_val.IsFunctionObject()) {
		return func_val.function().function_def();
	}
	else if  (func_val.IsFunctionDef()) {
		return func_val.function_def();
	}
	throw std::runtime_error("Unavailable function definition.");
}

bool Vm::FunctionDefLoadInit(Value* func_def_val) {
	auto& func_def = func_def_val->function_def();
	if (func_def.closure_var_defs().empty()) {
		return false;
	}

	*func_def_val = Value(new FunctionObject(&func_def));
	
	auto& func = func_def_val->function();
	// func->upvalues_.resize(func_def->closure_var_defs_.size());

	auto& arr = func.closure_value_arr();
	arr.resize(func_def.closure_var_defs().size());

	return true;
}

void Vm::FunctionEnterInit(const Value& func_val) {
	if (!func_val.IsFunctionObject()) {
		return;
	}

	auto& func = func_val.function();
	auto& func_def = func.function_def();

	// ���õ��Ǻ������󣬿�����Ҫ����հ��ڵ�upvalue
	auto& arr = func.closure_value_arr();
	for (auto& def : func_def.closure_var_defs()) {
		// ջ�ϵĶ���ͨ��upvalue�������հ�����
		stack_frame_.set(def.first, Value(
			UpValue(&arr[def.second.arr_idx])
		));
	}
}

Value& Vm::GetVar(VarIndex idx) {
	auto* var = &stack_frame_.get(idx);
	while (var->IsUpValue()) {
		var = var->up_value().value;
	}
	return *var;
}

void Vm::SetVar(VarIndex idx, Value&& var) {
	auto* var_ = &stack_frame_.get(idx);
	while (var_->IsUpValue()) {
		var_ = var_->up_value().value;
	}
	*var_ = std::move(var);
}

const Value& Vm::GetGlobalConst(ConstIndex idx) {
	auto& var = context_->runtime().const_pool().at(idx);
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

			auto& func = func_val.function();
			auto& parent_func = cur_func_val_.function();

			// ���������������ü����������ӳ��������е�ArrayValue����������
			func.set_parent_function(cur_func_val_);

			// ���õ���������ArrayValue
			auto& arr = func.closure_value_arr();
			auto& parent_arr = parent_func.closure_value_arr();

			for (auto& def : func.function_def().closure_var_defs()) {
				if (!def.second.parent_var_idx) {
					// ��ǰ�Ƕ�������
					continue;
				}
				auto parent_arr_idx = parent_func.function_def().closure_var_defs()[*def.second.parent_var_idx].arr_idx;
				arr[def.second.arr_idx] = Value(UpValue(&parent_arr[parent_arr_idx]));
			}
		}
		stack_frame_.push(std::move(func_val));
		return;
	}

	stack_frame_.push(value);
}

void Vm::SaveStackFrame(FunctionDef** cur_func_def, const Value& func_val, FunctionDef* func_def
	, Value&& this_val, uint32_t par_count, bool is_generator
	)
{
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

	// �л�������ջ֡
	*cur_func_def = func_def;
	cur_func_val_ = func_val;

	// �����Ѿ���ջ�ˣ��ٷ���ֲ���������
	if (!is_generator) {
		assert(stack().size() >= (*cur_func_def)->par_count());
		stack_frame_.set_bottom(stack().size() - (*cur_func_def)->par_count());

		assert((*cur_func_def)->var_count() >= (*cur_func_def)->par_count());
		stack().upgrade((*cur_func_def)->var_count() - (*cur_func_def)->par_count());
	}
	else {
		stack_frame_.set_bottom(stack().size() - (*cur_func_def)->var_count());
	}

	// ���浱ǰ����������Ret����
	stack_frame_.push(Value(save_func));
	stack_frame_.push(Value(save_pc));
	stack_frame_.push(Value(save_bottom));

	stack_frame_.set_this_val(std::move(this_val));

	if (cur_func_val_.IsFunctionObject()) {
		FunctionEnterInit(cur_func_val_);
	}
}


Value Vm::RestoreStackFrame(FunctionDef** cur_func_def) {
	// ��Ҫ���أ���Ҫ������ǰջ֡�ϵıհ�����������
	// �õ�����֮��ĺ������������丳ֵΪ

	auto ret_value = stack_frame_.pop();

	auto save_bottom = stack_frame_.pop();
	auto save_pc = stack_frame_.pop();
	auto save_func = stack_frame_.pop();

	// �ָ�������ջ֡
	cur_func_val_ = save_func;
	*cur_func_def = &function_def(cur_func_val_);
	pc_ = save_pc.u64();

	// ����ջ����ջ��
	stack().resize(stack_frame_.bottom());
	stack_frame_.set_bottom(save_bottom.u64());

	// ����λ��ԭ��λ��ջ֡ջ���ķ���ֵ
	return ret_value;
}


void Vm::Run() {
	auto cur_func_def = &function_def(cur_func_val_);
	
	do {
		OpcodeType opcode_; uint32_t par; auto pc = pc_; std::cout << cur_func_def->byte_code().Disassembly(context_, pc, opcode_, par, cur_func_def) << std::endl;
		auto opcode = cur_func_def->byte_code().GetOpcode(pc_++);
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
			auto const_idx = cur_func_def->byte_code().GetI8(pc_);
			pc_ += 1;
			LoadConst(const_idx);
			break;
		}
		case OpcodeType::kCLoadW: {
			auto const_idx = cur_func_def->byte_code().GetI16(pc_);
			pc_ += 2;
			LoadConst(const_idx);
			break;
		}
		case OpcodeType::kCLoadD: {
			auto const_idx = cur_func_def->byte_code().GetI32(pc_);
			pc_ += 4;
			LoadConst(const_idx);
			break;
		}
		case OpcodeType::kVLoad: {
			auto var_idx = cur_func_def->byte_code().GetU8(pc_);
			pc_ += 1;
			stack_frame_.push(GetVar(var_idx));
			break;
		}
		case OpcodeType::kVLoad_0:
		case OpcodeType::kVLoad_1:
		case OpcodeType::kVLoad_2: 
		case OpcodeType::kVLoad_3: {
			auto var_idx = opcode - OpcodeType::kVLoad_0;
			stack_frame_.push(GetVar(var_idx));
			break;
		}
		case OpcodeType::kPop: {
			stack_frame_.pop();
			break;
		}
		case OpcodeType::kDump: {
			stack_frame_.push(stack_frame_.get(-1));
			break;
		}
		case OpcodeType::kSwap: {
			std::swap(stack_frame_.get(-1), stack_frame_.get(-2));
			break;
		}
		case OpcodeType::kUndefined: {
			stack_frame_.push(Value());
			break;
		}
		case OpcodeType::kVStore: {
			auto var_idx = cur_func_def->byte_code().GetU8(pc_);
			pc_ += 1;
			SetVar(var_idx, stack_frame_.pop());
			break;
		}
		case OpcodeType::kVStore_0:
		case OpcodeType::kVStore_1:
		case OpcodeType::kVStore_2:
		case OpcodeType::kVStore_3: {
			auto var_idx = opcode - OpcodeType::kVStore_0;
			SetVar(var_idx, stack_frame_.pop());
			break;
		}
		case OpcodeType::kPropertyLoad: {
			auto key_val = stack_frame_.pop();
			auto& obj_val = stack_frame_.get(-1);
			if (obj_val.IsObject()) {
				auto& obj = obj_val.object();
				auto prop = obj.GetProperty(&context_->runtime(), key_val);
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
			auto key_val = stack_frame_.pop();

			auto obj_val = stack_frame_.pop();
			if (obj_val.IsObject()) {
				auto& obj = obj_val.object();
				obj.SetProperty(&context_->runtime(), key_val, stack_frame_.pop());
			}
			else {
				// ��Object���ͣ���������������
				// ��undefined��Ҫ����
				// number����Ҫת����ʱNumber Object
				// throw std::runtime_error("unrealized.");
			}
			
			break;
		}
		case OpcodeType::kIndexedLoad: {
			auto idx_val = stack_frame_.pop();
			idx_val = idx_val.ToString();

			auto& obj_val = stack_frame_.get(-1);
			auto& obj = obj_val.object();
			
			auto prop = obj.GetProperty(&context_->runtime(), idx_val);
			if (!prop) {
				obj_val = Value();
			}
			else {
				obj_val = *prop;
			}
			break;
		}
		case OpcodeType::kIndexedStore: {
			auto idx_val = stack_frame_.pop();
			idx_val = idx_val.ToString();

			auto obj_val = stack_frame_.pop();
			auto& obj = obj_val.object();

			obj.SetProperty(&context_->runtime(), idx_val, stack_frame_.pop());
			break;
		}
		case OpcodeType::kAdd: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = b + a;
			break;
		}
		case OpcodeType::kSub: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = b - a;
			break;
		}
		case OpcodeType::kMul: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = b * a;
			break;
		}
		case OpcodeType::kDiv: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = b / a;
			break;
		}
		case OpcodeType::kNeg: {
			auto& a = stack_frame_.get(-1);
			a = -a;
			break;
		}		 
		case OpcodeType::kFunctionCall: {
			//auto var_idx = cur_func_def->byte_code().GetU16(pc_);
			//pc_ += 2;

			// auto& func_val = GetVar(var_idx);

			auto this_val = stack_frame_.pop();
			auto func_val = stack_frame_.pop();
			FunctionSwitch(&cur_func_def, std::move(this_val), std::move(func_val));
			break;
		}
		case OpcodeType::kGetThis: {
			stack_frame_.push(stack_frame_.this_val());
			break;
		}
		case OpcodeType::kSetThis: {
			stack_frame_.set_this_val(stack_frame_.pop());
			break;
		}
		case OpcodeType::kReturn: {
			stack_frame_.push(RestoreStackFrame(&cur_func_def));
			break;
		}
		case OpcodeType::kGeneratorReturn: {
			auto& generator = stack_frame_.this_val().generator();

			generator.SetClosed();

			auto ret_value = RestoreStackFrame(&cur_func_def);
			auto ret_obj = generator.MakeReturnObject(&context_->runtime(), std::move(ret_value));
			stack_frame_.push(std::move(ret_obj));
			break;
		}
		case OpcodeType::kYield: {
			auto& generator = stack_frame_.this_val().generator();

			// ���浱ǰ��������pc
			generator.set_pc(pc_);

			// ���浱ǰջ֡��generator��ջ֡��
			auto& gen_vector = generator.stack().vector();
			for (int32_t i = 0; i < gen_vector.size(); ++i) {
				gen_vector[i] = std::move(stack_frame_.get(i));
			}

			auto ret_value = RestoreStackFrame(&cur_func_def);
			auto ret_obj = generator.MakeReturnObject(&context_->runtime(), std::move(ret_value));
			stack_frame_.push(std::move(ret_obj));
			break;
		}
		case OpcodeType::kNe: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = Value(!(b == a));
			break;
		}
		case OpcodeType::kEq: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = Value(b == a);
			break;
		}
		case OpcodeType::kLt: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = Value(b < a);
			break;
		}
		case OpcodeType::kLe: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = Value(!(b > a));
			break;
		}
		case OpcodeType::kGt: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = Value(b > a);
			break;
		}
		case OpcodeType::kGe: {
			auto a = stack_frame_.pop();
			auto& b = stack_frame_.get(-1);
			b = Value(!(b < a));
			break;
		}
		case OpcodeType::kIfEq: {
			auto boolean_val = stack_frame_.pop().ToBoolean();
			if (boolean_val.boolean() == false) {
				pc_ = cur_func_def->byte_code().CalcPc(--pc_);
			}
			else {
				pc_ += 2;
			}
			break;
		}
		case OpcodeType::kGoto: {
			pc_ = cur_func_def->byte_code().CalcPc(--pc_);
			break;
		}
		default:
			throw VmException("Unknown instruction");
		}
	} while (pc_ >= 0 && pc_ < cur_func_def->byte_code().Size());
}

void Vm::FunctionSwitch(FunctionDef** cur_func_def, Value&& this_val, Value&& func_val) {
	auto par_count = stack_frame_.pop().u64();

	switch (func_val.type()) {
	case ValueType::kFunctionDef:
	case ValueType::kFunctionObject: {
		auto& func_def = function_def(func_val);

		if (func_def.IsGenerator()) {
			// ������������
			// ֱ�ӷ�������������
			
			// ��ǰ��������;ֲ�����ջ�ռ�
			auto generator = new GeneratorObject(context_, func_val);
			generator->stack().upgrade(func_def.var_count());

			// �����������
			stack().reduce(par_count - func_def.par_count());

			// ���Ʋ���
			for (int32_t i = func_def.par_count() - 1; i >= 0; --i) {
				generator->stack().set(i, std::move(stack().pop()));
			}

			stack_frame_.push(Value(generator));
			return;
		}
		// printf("%s\n", func_def->Disassembly().c_str());

		if (par_count < func_def.par_count()) {
			throw VmException("Wrong number of parameters passed when calling the function");
		}

		// �Ѷ���Ĳ�������
		stack().reduce(par_count - func_def.par_count());
		SaveStackFrame(cur_func_def, func_val, &func_def, std::move(this_val), par_count, false);
		pc_ = 0;
		break;
	}
	case ValueType::kCppFunction: {
		// �л�ջ֡
		auto old_bottom = stack_frame_.bottom();
		stack_frame_.set_bottom(stack().size() - par_count);

		auto ret = func_val.cpp_function()(context_, this_val, par_count, &stack_frame_);

		// ��ԭջ֡
		stack_frame_.set_bottom(old_bottom);
		stack().reduce(par_count);
		stack_frame_.push(std::move(ret));
		break;
	}
	case ValueType::kGeneratorNext: {
		auto& generator = this_val.generator();

		auto next_val = Value();
		if (par_count > 0) {
			stack().reduce(par_count - 1);
			next_val = stack().pop();
		}
		if (generator.IsClosed()) {
			stack_frame_.push(generator.MakeReturnObject(&context_->runtime(), Value()));
			break;
		}

		bool is_first = !generator.IsExecuting();

		// ����ջ֡
		auto& vector = stack().vector();
		auto& gen_vector = generator.stack().vector();
		vector.insert(vector.end(), gen_vector.begin(), gen_vector.end());

		generator.SetExecuting();
		
		SaveStackFrame(cur_func_def, generator.function(), &generator.function_def()
			, std::move(this_val), 0, true);
		pc_ = generator.pc();

		// next������ջ
		if (!is_first) {
			stack().push(next_val);
		}

		break;
	}
	default:
		throw VmException("Non callable type.");
	}
}

} // namespace mjs