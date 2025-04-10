#include <iostream>

#include <mjs/vm.h>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/opcode.h>
#include <mjs/array_object.h>
#include <mjs/function_object.h>
#include <mjs/generator_object.h>
#include <mjs/async_object.h>
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

	// ����������һ��func_def����ô��Ҫ����Ϊfunc_obj
	if (func_val.IsFunctionDef()) {
		InitClosure(upper_stack_frame, &func_val);
	}

	CallInternal(&stack_frame, std::move(func_val), std::move(this_val), argv.size());

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

void Vm::BindClosureVars(StackFrame* stack_frame) {
	if (!stack_frame->func_val().IsFunctionObject()) {
		return;
	}

	auto& func = stack_frame->func_val().function();
	auto* func_def = stack_frame->func_def();

	// ���õ��Ǻ������󣬿�����Ҫ����հ��ڵ�upvalue
	auto& arr = func.closure_value_arr();
	for (auto& def : stack_frame->func_def()->closure_var_defs()) {
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


void Vm::CallInternal(StackFrame* stack_frame, Value func_val, Value this_val, uint32_t param_count) {
	stack_frame->set_func_val(std::move(func_val));
	stack_frame->set_this_val(std::move(this_val));
	
	std::optional<Value> pending_return_val;
	
	if (!FunctionScheduling(stack_frame, param_count)) {
		goto exit_;
	}

	//if (cur_func_def_) {
	//	std::cout << cur_func_def_->Disassembly(context_);
	//}

	{
		std::optional<Value> pending_error_val;
		Pc pending_goto_pc = kInvalidPc;

		auto* func_def = stack_frame->func_def();
		while (stack_frame->pc() >= 0 && func_def && stack_frame->pc() < func_def->byte_code().Size()) {
			//OpcodeType opcode_; uint32_t par; auto pc = pc_; std::cout << exec_func_def->byte_code().Disassembly(context_, pc, opcode_, par, exec_func_def) << std::endl;
			auto opcode = func_def->byte_code().GetOpcode(stack_frame->pc());
			stack_frame->set_pc(stack_frame->pc() + 1);
			switch (opcode) {
				//case OpcodeType::kStop:
				//	return;
			case OpcodeType::kCLoad_0: {
			case OpcodeType::kCLoad_1:
			case OpcodeType::kCLoad_2:
			case OpcodeType::kCLoad_3:
			case OpcodeType::kCLoad_4:
			case OpcodeType::kCLoad_5:
				LoadConst(stack_frame, opcode - OpcodeType::kCLoad_0);
				break;
			}
			case OpcodeType::kCLoad: {
				auto const_idx = func_def->byte_code().GetI8(stack_frame->pc());
				stack_frame->set_pc(stack_frame->pc() + 1);
				LoadConst(stack_frame, const_idx);
				break;
			}
			case OpcodeType::kCLoadW: {
				auto const_idx = func_def->byte_code().GetI16(stack_frame->pc());
				stack_frame->set_pc(stack_frame->pc() + 2);
				LoadConst(stack_frame, const_idx);
				break;
			}
			case OpcodeType::kCLoadD: {
				auto const_idx = func_def->byte_code().GetI32(stack_frame->pc());
				stack_frame->set_pc(stack_frame->pc() + 4);
				LoadConst(stack_frame, const_idx);
				break;
			}
			case OpcodeType::kVLoad: {
				auto var_idx = func_def->byte_code().GetU8(stack_frame->pc());
				stack_frame->set_pc(stack_frame->pc() + 1);
				stack_frame->push(GetVar(stack_frame, var_idx));
				break;
			}
			case OpcodeType::kVLoad_0:
			case OpcodeType::kVLoad_1:
			case OpcodeType::kVLoad_2:
			case OpcodeType::kVLoad_3: {
				auto var_idx = opcode - OpcodeType::kVLoad_0;
				stack_frame->push(GetVar(stack_frame, var_idx));
				break;
			}
			case OpcodeType::kPop: {
				stack_frame->pop();
				break;
			}
			case OpcodeType::kDump: {
				stack_frame->push(stack_frame->get(-1));
				break;
			}
			case OpcodeType::kSwap: {
				std::swap(stack_frame->get(-1), stack_frame->get(-2));
				break;
			}
			case OpcodeType::kUndefined: {
				stack_frame->push(Value());
				break;
			}
			case OpcodeType::kVStore: {
				auto var_idx = func_def->byte_code().GetU8(stack_frame->pc());
				stack_frame->set_pc(stack_frame->pc() + 1);
				SetVar(stack_frame, var_idx, stack_frame->pop());
				break;
			}
			case OpcodeType::kVStore_0:
			case OpcodeType::kVStore_1:
			case OpcodeType::kVStore_2:
			case OpcodeType::kVStore_3: {
				auto var_idx = opcode - OpcodeType::kVStore_0;
				SetVar(stack_frame, var_idx, stack_frame->pop());
				break;
			}
			case OpcodeType::kPropertyLoad: {
				auto key_val = stack_frame->pop();
				auto& obj_val = stack_frame->get(-1);
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
				auto key_val = stack_frame->pop();
				auto obj_val = stack_frame->pop();
				auto val = stack_frame->pop();
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
				auto idx_val = stack_frame->pop();
				idx_val = idx_val.ToString();

				auto& obj_val = stack_frame->get(-1);
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
				auto idx_val = stack_frame->pop();
				idx_val = idx_val.ToString();

				auto obj_val = stack_frame->pop();
				auto& obj = obj_val.object();

				obj.SetProperty(&context_->runtime(), idx_val, stack_frame->pop());
				break;
			}
			case OpcodeType::kAdd: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = b + a;
				break;
			}
			case OpcodeType::kSub: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = b - a;
				break;
			}
			case OpcodeType::kMul: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = b * a;
				break;
			}
			case OpcodeType::kDiv: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = b / a;
				break;
			}
			case OpcodeType::kNeg: {
				auto& a = stack_frame->get(-1);
				a = -a;
				break;
			}
			case OpcodeType::kNew: {
				// this��Ȼ����function call����
				stack_frame->push(Value());
			}
			case OpcodeType::kFunctionCall: {
				auto this_val = stack_frame->pop();
				auto func_val = stack_frame->pop();
				auto param_count = stack_frame->pop().u64();
				
				auto new_stack_frame = StackFrame(*stack_frame);

				// �����Ѿ���ջ���ˣ�����bottom
				new_stack_frame.set_bottom(
					new_stack_frame.bottom() - param_count
				);

				CallInternal(&new_stack_frame, func_val, this_val, param_count);
				auto& ret = new_stack_frame.get(-1);
				if (ret.IsException()) {
					pending_error_val = std::move(ret);
					if (!ThrowExecption(stack_frame, &pending_error_val)) {
						goto exit_;
					}
				}
				break;
			}
			case OpcodeType::kGetThis: {
				stack_frame->push(stack_frame->this_val());
				break;
			}
			case OpcodeType::kSetThis: {
				stack_frame->set_this_val(stack_frame->pop());
				break;
			}
			case OpcodeType::kReturn: {
				goto exit_;
				break;
			}
			case OpcodeType::kGeneratorReturn: {
				auto& generator = stack_frame->this_val().generator();

				generator.SetClosed();

				auto ret_value = stack_frame->pop();
				auto ret_obj = generator.MakeReturnObject(&context_->runtime(), std::move(ret_value));
				stack_frame->push(std::move(ret_obj));

				goto exit_;
				break;
			}
			case OpcodeType::kAwait: {
				// ���ʽ�����һ��promise�����ж�״̬
				// �������ִ��

				auto val = stack_frame->get(-1);

				if (!val.IsPromiseObject()) {
					// ����Promise������Promise��װ
					val = Value(new PromiseObject(context_, Value()));
				}

				auto& promise = val.promise();
				auto& async_obj = stack_frame->func_val().async();
				GeneratorSaveContext(stack_frame, &async_obj);
				promise.Then(context_, Value(&async_obj), Value());
				goto exit_;
				break;
			}
			case OpcodeType::kYield: {
				auto& generator = stack_frame->this_val().generator();

				GeneratorSaveContext(stack_frame, &generator);

				auto ret_value = stack_frame->pop();
				auto ret_obj = generator.MakeReturnObject(&context_->runtime(), std::move(ret_value));
				stack_frame->push(std::move(ret_obj));

				goto exit_;
				break;
			}
			case OpcodeType::kNe: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = Value(!(b == a));
				break;
			}
			case OpcodeType::kEq: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = Value(b == a);
				break;
			}
			case OpcodeType::kLt: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = Value(b < a);
				break;
			}
			case OpcodeType::kLe: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = Value(!(b > a));
				break;
			}
			case OpcodeType::kGt: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = Value(b > a);
				break;
			}
			case OpcodeType::kGe: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = Value(!(b < a));
				break;
			}
			case OpcodeType::kIfEq: {
				auto boolean_val = stack_frame->pop().ToBoolean();
				if (boolean_val.boolean() == false) {
					stack_frame->set_pc(func_def->byte_code().CalcPc(stack_frame->pc() - 1));
				}
				else {
					stack_frame->set_pc(stack_frame->pc() + 2);
				}
				break;
			}
			case OpcodeType::kGoto: {
				stack_frame->set_pc(func_def->byte_code().CalcPc(stack_frame->pc() - 1));
				break;
			}
			case OpcodeType::kTryBegin: {
				break;
			}
			case OpcodeType::kThrow: {
				stack_frame->set_pc(stack_frame->pc() - 1);
				pending_error_val = stack_frame->pop();
				ThrowExecption(stack_frame, &pending_error_val);
				break;
			}
			case OpcodeType::kTryEnd: {
				// �Ȼص�try end
				stack_frame->set_pc(stack_frame->pc() - 1);
				if (pending_error_val) {
					// ������м�¼���쳣�������ף������������ֱ�ӵ��ϲ��׵ģ���Ϊtry end�����ڵ�ǰ��
					if (!ThrowExecption(stack_frame, &pending_error_val)) {
						goto exit_;
					}
				}
				else if (pending_return_val) {
					// finally����ˣ��б���ķ���ֵ
					auto& table = func_def->exception_table();
					auto* entry = table.FindEntry(stack_frame->pc());
					if (entry && entry->HasFinally()) {
						// �ϲ㻹����finally������ִ���ϲ�finally
						stack_frame->set_pc(entry->finally_start_pc);
						break;
					}
					stack_frame->push(std::move(*pending_return_val));
					stack_frame->push(stack_frame->pop());
					pending_return_val.reset();
					goto exit_;
				}
				else if (pending_goto_pc != kInvalidPc) {
					// finally����ˣ���δ��ɵ���ת
					auto& table = func_def->exception_table();
					auto* entry = table.FindEntry(stack_frame->pc());
					auto* goto_entry = table.FindEntry(pending_goto_pc);
					// Goto�Ƿ������ϲ�finally
					if (!goto_entry || entry == goto_entry) {
						stack_frame->set_pc(pending_goto_pc);
						pending_goto_pc = kInvalidPc;
					}
					else {
						stack_frame->set_pc(entry->finally_start_pc);
					}
				}
				else {
					stack_frame->set_pc(stack_frame->pc() + 1);
				}

				break;
			}
			case OpcodeType::kFinallyReturn: {
				stack_frame->set_pc(stack_frame->pc() - 1);
				// ����finally��return��䣬����ת��finally
				auto& table = func_def->exception_table();
				auto* entry = table.FindEntry(stack_frame->pc());
				if (!entry || !entry->HasFinally()) {
					throw VmException("Incorrect finally return.");
				}
				pending_return_val = stack_frame->pop();
				if (entry->LocatedInFinally(stack_frame->pc())) {
					// λ��finally�ķ��أ����ǵ�ԭ�ȵķ��أ���ת��TryEnd
					stack_frame->set_pc(entry->finally_end_pc);
				}
				else {
					stack_frame->set_pc(entry->finally_start_pc);
				}
				break;
			}
			case OpcodeType::kFinallyGoto: {
				stack_frame->set_pc(stack_frame->pc() - 1);
				// goto������finally����ִ��finally
				auto& table = func_def->exception_table();
				auto* entry = table.FindEntry(stack_frame->pc());
				if (!entry || !entry->HasFinally()) {
					throw VmException("Incorrect finally return.");
				}
				pending_goto_pc = func_def->byte_code().CalcPc(stack_frame->pc());
				if (entry->LocatedInFinally(stack_frame->pc())) {
					// λ��finally��goto
					stack_frame->set_pc(entry->finally_end_pc);
				}
				else {
					stack_frame->set_pc(entry->finally_start_pc);
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
		pending_return_val = stack_frame->pop();
	}
	else {
		pending_return_val->SetException();
	}

	// ��ԭջ֡
	stack().resize(stack_frame->bottom());
	stack_frame->push(std::move(*pending_return_val));
	return;
}

bool Vm::FunctionScheduling(StackFrame* stack_frame, uint32_t par_count) {
	switch (stack_frame->func_val().type()) {
	case ValueType::kFunctionDef:
	case ValueType::kFunctionObject: {
		stack_frame->set_func_def(function_def(stack_frame->func_val()));
		auto* func_def = stack_frame->func_def();

		// printf("%s\n", func_def->Disassembly().c_str());

		if (par_count < func_def->par_count()) {
			throw VmException("Wrong number of parameters passed when calling the function");
		}

		// �����������
		stack().reduce(par_count - func_def->par_count());

		assert(func_def->var_count() >= func_def->par_count());
		stack_frame->upgrade(func_def->var_count() - func_def->par_count());
		BindClosureVars(stack_frame);

		if (func_def->IsGenerator() || func_def->IsAsync()) {
			GeneratorObject* generator;
			if (func_def->IsGenerator()) {
				generator = new GeneratorObject(context_, stack_frame->func_val());
			}
			else {
				generator = new AsyncObject(context_, stack_frame->func_val());
			}

			// ��ǰ��������;ֲ�����ջ�ռ�
			generator->stack().upgrade(func_def->var_count());

			// ���Ʋ����ͱ���(��Ϊ��������ͨ��BindClosureVars����)
			for (int32_t i = 0; i < func_def->var_count(); ++i) {
				generator->stack().set(i, stack_frame->get(i));
			}

			if (func_def->IsGenerator()) {
				// ������������
				// ��ֱ�ӷ�������������
				stack_frame->push(Value(generator));
				return false;
			}
			else {
				// ���첽����
				// ��ʼִ��
				stack_frame->set_func_val(Value(static_cast<AsyncObject*>(generator)));
			}
		}
		return true;
	}
	case ValueType::kClassDef: {
		auto obj = stack_frame->func_val().class_def().Constructor(context_, par_count, *stack_frame);
		stack_frame->push(std::move(obj));
		return false;
	}
	case ValueType::kCppFunction: {
		// �л�ջ֡
		auto ret = stack_frame->func_val().cpp_function()(context_, stack_frame->this_val(), par_count, *stack_frame);
		stack_frame->push(std::move(ret));
		return false;
	}
	case ValueType::kGeneratorNext: {
		auto& generator = stack_frame->this_val().generator();

		auto next_val = Value();
		if (par_count > 0) {
			stack_frame->reduce(par_count - 1);
			next_val = stack_frame->pop();
		}
		if (generator.IsClosed()) {
			stack_frame->push(generator.MakeReturnObject(&context_->runtime(), Value()));
			// ����ɣ�������Ҫִ��
			return false;
		}

		bool is_first = !generator.IsExecuting();

		GeneratorRestoreContext(stack_frame, &generator);

		// next������ջ
		if (!is_first) {
			stack_frame->push(next_val);
		}
		return true;
	}
	case ValueType::kPromiseResolve:
	case ValueType::kPromiseReject: {
		auto& promise = stack_frame->func_val().promise();

	    Value value;
	    if (par_count > 0) {
	        value = stack_frame->get(-1);
	    }
		stack_frame->reduce(par_count);

		if (stack_frame->func_val().type() == ValueType::kPromiseResolve) {
			promise.Resolve(context_, value);
		}
		else if (stack_frame->func_val().type() == ValueType::kPromiseReject) {
			promise.Reject(context_, value);
		}

		stack_frame->push(Value());

		return false;
	}
	case ValueType::kAsyncObject: {
		auto& async = stack_frame->func_val().async();
		auto ret = stack_frame->pop();
		GeneratorRestoreContext(stack_frame, &async);
		stack_frame->push(std::move(ret));
		return true;
	}
	default:
		throw VmException("Non callable type.");
	}
}

bool Vm::ThrowExecption(StackFrame* stack_frame, std::optional<Value>* error_val) {
	auto& table = stack_frame->func_def()->exception_table();

	auto* entry = table.FindEntry(stack_frame->pc());
	if (!entry) {
		return false;
	}

	if (entry->LocatedInTry(stack_frame->pc())) {
		// λ��try���׳����쳣
		if (entry->HasCatch()) {
			// ����catch��catch���ܻ��쳣������쳣��Ҫ��ִ��finally��Ȼ���������
			SetVar(stack_frame, entry->catch_err_var_idx, std::move(**error_val));
			error_val->reset();
			stack_frame->set_pc(entry->catch_start_pc);
		}
		else {
			// û��catch������error_val����finallyĩβ��rethrow����
			stack_frame->set_pc(entry->finally_start_pc);
		}
	}
	else if (entry->HasCatch() && entry->LocatedInCatch(stack_frame->pc())) {
		if (entry->HasFinally()) {
			stack_frame->set_pc(entry->finally_start_pc);
		}
	}
	else if (entry->HasFinally() && entry->LocatedInFinally(stack_frame->pc())) {
		// finally�׳��쳣�����Ǵ�������ת������TryEnd
		stack_frame->set_pc(entry->finally_end_pc);
	}
	else {
		throw VmException("Incorrect execption address.");
	}
	return true;
}

void Vm::GeneratorSaveContext(StackFrame* stack_frame, GeneratorObject* generator) {
	// ���浱ǰ��������pc
	generator->set_pc(stack_frame->pc());

	// ���浱ǰջ֡��generator��ջ֡��
	auto& gen_vector = generator->stack().vector();
	for (int32_t i = 0; i < gen_vector.size(); ++i) {
		gen_vector[i] = std::move(stack_frame->get(i));
	}
}

void Vm::GeneratorRestoreContext(StackFrame* stack_frame, GeneratorObject* generator) {
	// ����ջ֡
	auto& vector = stack().vector();
	auto& gen_vector = generator->stack().vector();
	vector.insert(vector.end(), gen_vector.begin(), gen_vector.end());

	generator->SetExecuting();

	stack_frame->set_func_def(&generator->function_def());
	stack_frame->set_pc(generator->pc());
}

} // namespace mjs