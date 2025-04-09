#include <iostream>

#include <mjs/vm.h>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/opcode.h>
#include <mjs/array_object.h>
#include <mjs/function_object.h>
#include <mjs/generator_object.h>
#include <mjs/promise_object.h>

namespace mjs {

Vm::Vm(Context* context)
	: context_(context) {}

Stack& Vm::stack() {
	return context_->runtime().stack();
}

Value Vm::CallFunction(const StackFrame& upper_stack_frame, Value func_val, Value this_val, const std::vector<Value>& argv) {
	auto stack_frame = StackFrame(upper_stack_frame);
	
	// ����������ջ
	for (auto& v : argv) {
		stack_frame.push(v);
	}
	// par_count
	stack_frame.push(Value(argv.size()));

	// ����������һ��func_def����ô��Ҫ����Ϊfunc_obj
	if (func_val.IsFunctionDef()) {
		InitClosure(upper_stack_frame, &func_val);
	}

	CallInternal(&stack_frame, std::move(func_val), std::move(this_val));

	return stack_frame.pop();
}

FunctionDef* Vm::function_def(const Value& func_val) const {
	if (func_val.IsFunctionObject()) {
		return &func_val.function().function_def();
	}
	else if (func_val.IsFunctionDef()) {
		return &func_val.function_def();
	}
	return nullptr;
	throw std::runtime_error("Unavailable function definition.");
}

bool Vm::InitClosure(const StackFrame& upper_stack_frame, Value* func_def_val) {
	auto& func_def = func_def_val->function_def();
	if (func_def.closure_var_defs().empty()) {
		return false;
	}

	*func_def_val = Value(new FunctionObject(&func_def));
	
	auto& func = func_def_val->function();
	// func->upvalues_.resize(func_def->closure_var_defs_.size());

	auto& arr = func.closure_value_arr();
	arr.resize(func_def.closure_var_defs().size());

	// array��Ҫ��ʼ��Ϊupvalue��ָ�򸸺�����ArrayValue
	// ���û�и������Ͳ���Ҫ�����Ƕ��㺯����Ĭ�ϳ�ʼ��Ϊδ����
	if (!upper_stack_frame.func_val().IsFunctionObject()) {
		return true;
	}

	auto& parent_func = upper_stack_frame.func_val().function();

	// ���������������ü����������ӳ��������е�ArrayValue����������
	func.set_parent_function(upper_stack_frame.func_val());

	// ���õ���������ArrayValue
	auto& parent_arr = parent_func.closure_value_arr();

	for (auto& def : func.function_def().closure_var_defs()) {
		if (!def.second.parent_var_idx) {
			// ��ǰ�Ƕ�������
			continue;
		}
		auto parent_arr_idx = parent_func.function_def().closure_var_defs()[*def.second.parent_var_idx].arr_idx;
		arr[def.second.arr_idx] = Value(UpValue(&parent_arr[parent_arr_idx]));
	}

	return true;
}

void Vm::BindClosureVars(StackFrame* stack_frame, const Value& func_val) {
	if (!func_val.IsFunctionObject()) {
		return;
	}

	auto& func = func_val.function();
	auto& func_def = func.function_def();

	// ���õ��Ǻ������󣬿�����Ҫ����հ��ڵ�upvalue
	auto& arr = func.closure_value_arr();
	for (auto& def : func_def.closure_var_defs()) {
		// ջ�ϵĶ���ͨ��upvalue�������հ�����
		stack_frame->set(def.first, Value(
			UpValue(&arr[def.second.arr_idx])
		));
	}
}

Value& Vm::GetVar(StackFrame* stack_frame, VarIndex idx) {
	auto* var = &stack_frame->get(idx);
	while (var->IsUpValue()) {
		var = var->up_value().value;
	}
	return *var;
}

void Vm::SetVar(StackFrame* stack_frame, VarIndex idx, Value&& var) {
	auto* var_ = &stack_frame->get(idx);
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

const Value& Vm::GetConst(StackFrame* stack_frame, ConstIndex idx) {
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


void Vm::LoadConst(StackFrame* stack_frame, ConstIndex const_idx) {
	auto& value = GetGlobalConst(const_idx);

	if (value.IsFunctionDef()) {
		auto func_val = value;
		InitClosure(*stack_frame, &func_val);
		stack_frame->push(std::move(func_val));
		return;
	}

	stack_frame->push(value);
}

void Vm::SwitchStackFrame(const Value& func_val, FunctionDef* func_def
	, Value&& this_val, uint32_t par_count, bool is_generator
	)
{
	//auto save_func = cur_func_val_;
	//auto save_pc = pc_;
	//auto save_bottom = stack_frame_.bottom();

	// ջ֡���֣�
		//// ������Ϣ // ����
		// �ֲ�����
		// ����1
		// ...
		// ����n    <- bottom
		// ��һջ֡

	// �����Ѿ���ջ�ˣ��ٷ���ֲ���������
	if (!is_generator) {
		assert(func_def->var_count() >= func_def->par_count());
		stack().upgrade(func_def->var_count() - func_def->par_count());
	}

	// ���浱ǰ����������Ret����
	//stack_frame_.push(Value(save_func));
	//stack_frame_.push(Value(save_pc));
	//stack_frame_.push(Value(save_bottom));

	stack_frame_.set_this_val(std::move(this_val));

	//if (cur_func_val_.IsFunctionObject()) {
	//	BindClosureVars(cur_func_val_);
	//}
}


//Value Vm::RestoreStackFrame() {
//	// ��Ҫ���أ���Ҫ������ǰջ֡�ϵıհ�����������
//	// �õ�����֮��ĺ������������丳ֵΪ
//
//	auto ret_value = stack_frame_.pop();
//
//	auto save_bottom = stack_frame_.pop();
//	auto save_pc = stack_frame_.pop();
//	auto save_func = stack_frame_.pop();
//
//	// �ָ�������ջ֡
//	cur_func_val_ = save_func;
//	cur_func_def_ = function_def(cur_func_val_);
//	JumpTo(save_pc.u64());
//
//	// ����ջ����ջ��
//	stack().resize(stack_frame_.bottom());
//	stack_frame_.set_bottom(save_bottom.u64());
//
//	// ����λ��ԭ��λ��ջ֡ջ���ķ���ֵ
//	return ret_value;
//}

void Vm::CallInternal(StackFrame* stack_frame, Value func_val, Value this_val) {
	auto par_count = stack_frame->pop().u64();

	std::optional<Value> pending_return_val;

	if (!FunctionSwitch(stack_frame, &func_val, this_val, par_count)) {
		goto exit_;
	}

	//if (cur_func_def_) {
	//	std::cout << cur_func_def_->Disassembly(context_);
	//}

	{
		BindClosureVars(stack_frame, func_val);

		std::optional<Value> pending_error_val;
		Pc pending_goto_pc = kInvalidPc;


		auto* func_def = function_def(func_val);
		while (stack_frame->pc() >= 0 && func_def && pc < func_def->byte_code().Size()) {
			//OpcodeType opcode_; uint32_t par; auto pc = pc_; std::cout << exec_func_def->byte_code().Disassembly(context_, pc, opcode_, par, exec_func_def) << std::endl;
			stack_frame->set_pc(stack_frame->pc() + 1);
			auto opcode = func_def->byte_code().GetOpcode(stack_frame->pc());
			switch (opcode) {
				//case OpcodeType::kStop:
				//	return;
			case OpcodeType::kCLoad_0: {
			case OpcodeType::kCLoad_1:
			case OpcodeType::kCLoad_2:
			case OpcodeType::kCLoad_3:
			case OpcodeType::kCLoad_4:
			case OpcodeType::kCLoad_5:
				LoadConst(func_val, opcode - OpcodeType::kCLoad_0);
				break;
			}
			case OpcodeType::kCLoad: {
				auto const_idx = func_def->byte_code().GetI8(pc);
				pc += 1;
				LoadConst(func_val, const_idx);
				break;
			}
			case OpcodeType::kCLoadW: {
				auto const_idx = func_def->byte_code().GetI16(pc);
				pc += 2;
				LoadConst(func_val, const_idx);
				break;
			}
			case OpcodeType::kCLoadD: {
				auto const_idx = func_def->byte_code().GetI32(pc);
				pc += 4;
				LoadConst(func_val, const_idx);
				break;
			}
			case OpcodeType::kVLoad: {
				auto var_idx = func_def->byte_code().GetU8(pc);
				pc += 1;
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
				auto var_idx = func_def->byte_code().GetU8(pc);
				pc += 1;
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
				Value* prop = nullptr;
				if (obj_val.IsObject()) {
					auto& obj = obj_val.object();
					prop = obj.GetProperty(&context_->runtime(), key_val);
				}
				else if (obj_val.IsClassDef()) {
					auto& class_def = obj_val.class_def();
					prop = class_def.GetStaticProperty(&context_->runtime(), key_val);
				}
				else {
					// ��Object���ͣ���������������
					// ��undefined��Ҫ����
					// number����Ҫת����ʱNumber Object
				}

				if (!prop) {
					obj_val = Value();
				}
				else {
					obj_val = *prop;
				}

				break;
			}
			case OpcodeType::kPropertyStore: {
				auto key_val = stack_frame_.pop();
				auto obj_val = stack_frame_.pop();
				auto val = stack_frame_.pop();
				if (obj_val.IsObject()) {
					auto& obj = obj_val.object();
					obj.SetProperty(&context_->runtime(), key_val, std::move(val));
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
			case OpcodeType::kNew: {
				// this��Ȼ����function call����
				stack_frame_.push(Value());
			}
			case OpcodeType::kFunctionCall: {
				auto this_val = stack_frame_.pop();
				auto func_val = stack_frame_.pop();

				CallInternal(func_val, this_val);
				auto& ret = stack_frame_.get(-1);
				if (ret.IsException()) {
					pending_error_val = std::move(ret);
					if (!ThrowExecption(func_def, &pc, &pending_error_val)) {
						goto exit_;
					}
				}
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
				goto exit_;
				break;
			}
			case OpcodeType::kGeneratorReturn: {
				auto& generator = stack_frame_.this_val().generator();

				generator.SetClosed();

				auto ret_value = stack_frame_.pop();
				auto ret_obj = generator.MakeReturnObject(&context_->runtime(), std::move(ret_value));
				stack_frame_.push(std::move(ret_obj));

				goto exit_;
				break;
			}
			case OpcodeType::kAwait: {
				// ���ʽ�����һ��promise�����ж�״̬�������pending������yield����
				// �������ִ��

				auto& obj = stack_frame_.get(-1);

				if (obj.IsPromiseObject()) {
					auto& promise = obj.promise();
					if (promise.IsPending()) {
						// yield����

						// ��ô�õ��Լ���generator��


						goto exit_;
					}
				}

				//auto ret_value = stack_frame_.pop();
				//stack_frame_.push(std::move(ret_value));

				
				break;
			}
			case OpcodeType::kYield: {
				auto& generator = stack_frame_.this_val().generator();

				// ���浱ǰ��������pc
				generator.set_pc(pc);

				// ���浱ǰջ֡��generator��ջ֡��
				auto& gen_vector = generator.stack().vector();
				for (int32_t i = 0; i < gen_vector.size(); ++i) {
					gen_vector[i] = std::move(stack_frame_.get(i));
				}

				auto ret_value = stack_frame_.pop();
				auto ret_obj = generator.MakeReturnObject(&context_->runtime(), std::move(ret_value));
				stack_frame_.push(std::move(ret_obj));

				goto exit_;
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
					pc = (func_def->byte_code().CalcPc(--pc));
				}
				else {
					pc = (pc + 2);
				}
				break;
			}
			case OpcodeType::kGoto: {
				pc = (func_def->byte_code().CalcPc(--pc));
				break;
			}
			case OpcodeType::kTryBegin: {
				break;
			}
			case OpcodeType::kThrow: {
				--pc;
				pending_error_val = stack_frame_.pop();
				ThrowExecption(func_def, &pc, &pending_error_val);
				break;
			}
			case OpcodeType::kTryEnd: {
				// �Ȼص�try end
				--pc;
				if (pending_error_val) {
					// ������м�¼���쳣�������ף������������ֱ�ӵ��ϲ��׵ģ���Ϊtry end�����ڵ�ǰ��
					if (!ThrowExecption(func_def, &pc, &pending_error_val)) {
						goto exit_;
					}
				}
				else if (pending_return_val) {
					// finally����ˣ��б���ķ���ֵ
					auto& table = func_def->exception_table();
					auto* entry = table.FindEntry(pc);
					if (entry && entry->HasFinally()) {
						// �ϲ㻹����finally������ִ���ϲ�finally
						pc = (entry->finally_start_pc);
						break;
					}
					stack_frame_.push(std::move(*pending_return_val));
					stack_frame_.push(stack_frame_.pop());
					pending_return_val.reset();
					goto exit_;
				}
				else if (pending_goto_pc != kInvalidPc) {
					// finally����ˣ���δ��ɵ���ת
					auto& table = func_def->exception_table();
					auto* entry = table.FindEntry(pc);
					auto* goto_entry = table.FindEntry(pending_goto_pc);
					// Goto�Ƿ������ϲ�finally
					if (!goto_entry || entry == goto_entry) {
						pc = (pending_goto_pc);
						pending_goto_pc = kInvalidPc;
					}
					else {
						pc = (entry->finally_start_pc);
					}
				}
				else {
					++pc;
				}

				break;
			}
			case OpcodeType::kFinallyReturn: {
				--pc;
				// ����finally��return��䣬����ת��finally
				auto& table = func_def->exception_table();
				auto* entry = table.FindEntry(pc);
				if (!entry || !entry->HasFinally()) {
					throw VmException("Incorrect finally return.");
				}
				pending_return_val = stack_frame_.pop();
				if (entry->LocatedInFinally(pc)) {
					// λ��finally�ķ��أ����ǵ�ԭ�ȵķ��أ���ת��TryEnd
					pc = (entry->finally_end_pc);
				}
				else {
					pc = (entry->finally_start_pc);
				}
				break;
			}
			case OpcodeType::kFinallyGoto: {
				--pc;
				// goto������finally����ִ��finally
				auto& table = func_def->exception_table();
				auto* entry = table.FindEntry(pc);
				if (!entry || !entry->HasFinally()) {
					throw VmException("Incorrect finally return.");
				}
				pending_goto_pc = func_def->byte_code().CalcPc(pc);
				if (entry->LocatedInFinally(pc)) {
					// λ��finally��goto
					pc = (entry->finally_end_pc);
				}
				else {
					pc = (entry->finally_start_pc);
				}
				break;
			}
			default:
				throw VmException("Unknown instruction.");
			}
		}
	}

exit_:
	if (!pending_return_val) {
		pending_return_val = stack_frame_.pop();
	}
	else {
		pending_return_val->SetException();
	}

	// ��ԭջ֡
	stack().resize(stack_frame_.bottom());
	stack_frame_.set_bottom(save_bottom);
	
	stack_frame_.push(std::move(*pending_return_val));
	return;
}

bool Vm::FunctionSwitch(StackFrame* stack_frame, Value* func_val, Value this_val, uint32_t par_count) {
	switch (func_val->type()) {
	case ValueType::kFunctionDef:
	case ValueType::kFunctionObject: {
		auto func_def = function_def(*func_val);

		// printf("%s\n", func_def->Disassembly().c_str());

		if (par_count < func_def->par_count()) {
			throw VmException("Wrong number of parameters passed when calling the function");
		}

		// �����������
		stack().reduce(par_count - func_def->par_count());

		if (func_def->IsGenerator() || func_def->IsAsync()) {
			// ��ǰ��������;ֲ�����ջ�ռ�
			auto generator = new GeneratorObject(context_, *func_val);

			generator->stack().upgrade(func_def->var_count());

			// ���Ʋ���
			for (int32_t i = func_def->par_count() - 1; i >= 0; --i) {
				generator->stack().set(i, std::move(stack().pop()));
			}

			if (func_def->IsGenerator()) {
				// ������������
				// ��ֱ�ӷ�������������
				stack_frame_.push(Value(generator));
				return false;
			}
			else {
				// ���첽����
				// ��ʼִ��
				*func_val = Value(ValueType::kAsyncFunction, generator);
			}
		}
		
		SwitchStackFrame(*func_val, func_def, std::move(this_val), par_count, false);
		*pc = (0);

		return true;
	}
	case ValueType::kCppFunction: {
		// �л�ջ֡
		auto ret = func_val->cpp_function()(context_, this_val, par_count, stack_frame_);
		stack().reduce(par_count);
		stack_frame_.push(std::move(ret));
		return false;
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
			// ����ɣ�������Ҫִ��
			return false;
		}

		bool is_first = !generator.IsExecuting();

		// ����ջ֡
		auto& vector = stack().vector();
		auto& gen_vector = generator.stack().vector();
		vector.insert(vector.end(), gen_vector.begin(), gen_vector.end());

		generator.SetExecuting();
		
		SwitchStackFrame(generator.function(), &generator.function_def()
			, std::move(this_val), 0, true);
		*pc = generator.pc();

		// next������ջ
		if (!is_first) {
			stack().push(next_val);
		}
		return true;
	}
	case ValueType::kPromiseResolve:
	case ValueType::kPromiseReject: {
		auto& promise = func_val->promise();

	    Value value;
	    if (par_count > 0) {
	        value = stack_frame_.get(-1);
	    }
		stack().reduce(par_count);

		if (func_val->type() == ValueType::kPromiseResolve) {
			promise.Resolve(context_, value);
		}
		else if (func_val->type() == ValueType::kPromiseReject) {
			promise.Reject(context_, value);
		}

		stack_frame_.push(Value());

		return false;
	}
	case ValueType::kClassDef: {
		auto obj = func_val->class_def().Constructor(context_, par_count, stack_frame_);
		// ��ԭջ֡
		stack().reduce(par_count);
		stack_frame_.push(std::move(obj));
		return false;
	}
	default:
		throw VmException("Non callable type.");
	}
}

bool Vm::ThrowExecption(FunctionDef* cur_func_def, Pc* pc, std::optional<Value>* error_val) {
	auto& table = cur_func_def->exception_table();

	auto* entry = table.FindEntry(*pc);
	if (!entry) {
		return false;
	}

	if (entry->LocatedInTry(*pc)) {
		// λ��try���׳����쳣
		if (entry->HasCatch()) {
			// ����catch��catch���ܻ��쳣������쳣��Ҫ��ִ��finally��Ȼ���������
			SetVar(entry->catch_err_var_idx, std::move(**error_val));
			error_val->reset();
			*pc = entry->catch_start_pc;
		}
		else {
			// û��catch������error_val����finallyĩβ��rethrow����
			*pc = entry->finally_start_pc;
		}
	}
	else if (entry->HasCatch() && entry->LocatedInCatch(*pc)) {
		if (entry->HasFinally()) {
			*pc = entry->finally_start_pc;
		}
	}
	else if (entry->HasFinally() && entry->LocatedInFinally(*pc)) {
		// finally�׳��쳣�����Ǵ�������ת������TryEnd
		*pc = entry->finally_end_pc;
	}
	else {
		throw VmException("Incorrect execption address.");
	}
	return true;
}

//void Vm::JumpTo(Pc pc) {
//	pc_ = pc;
//}

} // namespace mjs