#include <iostream>

#include <mjs/vm.h>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/opcode.h>
#include <mjs/object/array_object.h>
#include <mjs/object/function_object.h>
#include <mjs/object/generator_object.h>
#include <mjs/object/async_object.h>
#include <mjs/object/promise_object.h>
#include <mjs/object/module_object.h>
#include <mjs/class_def/promise_object_class_def.h>

namespace mjs {

Vm::Vm(Context* context)
	: context_(context) 
	, stack_frame_(&context->runtime().stack()) {}


bool Vm::InitClosure(const StackFrame& upper_stack_frame, Value* func_def_val) {
	auto& func_def = func_def_val->function_def();
	if (func_def.closure_var_defs().empty()) {
		return false;
	}

	FunctionObject* func_obj;
	if (func_def.IsModule()) {
		*func_def_val = Value(new ModuleObject(context_, &func_def));
		func_obj = &func_def_val->module();
	}
	else {
		*func_def_val = Value(new FunctionObject(context_, &func_def));
		func_obj = &func_def_val->function();
	}
	
	auto& arr = func_obj->closure_value_arr();
	arr.resize(func_def.closure_var_defs().size());

	// array需要初始化为upvalue，指向父函数的ArrayValue
	// 如果没有父函数就不需要，即是顶层模块，默认初始化为未定义
	auto& parent_func_val = upper_stack_frame.function_val();
	if (parent_func_val.IsUndefined()) {
		assert(func_def.IsModule());
		return true;
	}

	FunctionObject* parent_func_obj;
	if (upper_stack_frame.function_def()->IsModule()) {
		parent_func_obj = &parent_func_val.module();
	}
	else {
		parent_func_obj = &parent_func_val.function();
	}

	// 递增父函数的引用计数，用于延长父函数中的closure_value_arr_的生命周期
	func_obj->set_parent_function(parent_func_val);

	// 引用到父函数的closure_value_arr_
	auto& parent_arr = parent_func_obj->closure_value_arr();

	for (auto& def : func_obj->function_def().closure_var_defs()) {
		if (!def.second.parent_var_idx) {
			// 当前是顶级变量
			continue;
		}
		auto& parent_closure_var_defs = parent_func_obj->function_def().closure_var_defs();
		auto parent_arr_idx = parent_closure_var_defs[*def.second.parent_var_idx].arr_idx;
		arr[def.second.arr_idx] = Value(UpValue(&parent_arr[parent_arr_idx]));
	}

	return true;
}


void Vm::BindClosureVars(StackFrame* stack_frame) {
	auto& func_val = stack_frame->function_val();
	if (func_val.IsUndefined() || func_val.IsFunctionDef()) {
		return;
	}

	auto* func_def = stack_frame->function_def();
	FunctionObject* func_obj;
	if (func_def->IsModule()) {
		auto module_obj = &stack_frame->function_val().module();

		// 额外绑定可能存在的导出变量到export_map
		auto& arr = module_obj->closure_value_arr();
		for (auto& pair : func_def->export_var_defs()) {
			auto var_idx = pair.second;
			auto iter = func_def->closure_var_defs().find(var_idx);
			assert(iter != func_def->closure_var_defs().end());
			module_obj->export_map().set(&context_->runtime(), pair.first, Value(
				UpValue(&arr[iter->second.arr_idx])
			));
		}

		func_obj = module_obj;
	}
	else {
		func_obj = &stack_frame->function_val().function();
	}

	// 调用的是函数对象，可能需要处理闭包内的upvalue
	auto& arr = func_obj->closure_value_arr();
	for (auto& def : stack_frame->function_def()->closure_var_defs()) {
		// 栈上的对象通过upvalue关联到闭包变量
		stack_frame->set(def.first, Value(
			UpValue(&arr[def.second.arr_idx])
		));
	}
}

Value& Vm::GetVar(StackFrame* stack_frame, VarIndex idx) {
	auto* var = &stack_frame->get(idx);
	if (var->IsUpValue()) {
		var = &var->up_value().Up();
	}
	return *var;
}

void Vm::SetVar(StackFrame* stack_frame, VarIndex idx, Value&& var) {
	auto* var_ = &stack_frame->get(idx);
	if (var_->IsUpValue()) {
		var_ = &var_->up_value().Up();
	}
	*var_ = std::move(var);
}

const Value& Vm::GetGlobalConst(ConstIndex idx) {
	auto& var = context_->runtime().const_pool().at(idx);
	return var;
}

void Vm::LoadConst(StackFrame* stack_frame, ConstIndex const_idx) {
	auto& value = GetGlobalConst(const_idx);
	// 未来改掉，生成单独的加载函数指令
	if (value.IsFunctionDef()) {
		auto func_val = value;
		InitClosure(*stack_frame, &func_val);
		stack_frame->push(std::move(func_val));
		return;
	}
	stack_frame->push(value);
}

bool Vm::FunctionScheduling(StackFrame* stack_frame, uint32_t par_count) {
	switch (stack_frame->function_val().type()) {
	case ValueType::kFunctionDef: {
		// 这里可能需要初始化，因为可能由C++处调用一个需要提升的FunctionDef
		auto func_val = stack_frame->function_val();
		if (InitClosure(stack_frame->upper_stack_frame(), &func_val)) {
			stack_frame->set_function_val(std::move(func_val));
		}
	}
	case ValueType::kModuleObject:
	case ValueType::kFunctionObject: {
		stack_frame->set_function_def(function_def(stack_frame->function_val()));
		auto* function_def = stack_frame->function_def();

		// printf("%s\n", function_def->Disassembly().c_str());

		if (par_count < function_def->par_count()) {
			throw VmException("Wrong number of parameters passed when calling the function");
		}

		// 弹出多余参数
		stack().reduce(par_count - function_def->par_count());

		assert(function_def->var_count() >= function_def->par_count());
		stack_frame->upgrade(function_def->var_count() - function_def->par_count());
		BindClosureVars(stack_frame);

		if (function_def->IsGenerator() || function_def->IsAsync()) {
			GeneratorObject* generator;
			if (function_def->IsGenerator()) {
				generator = new GeneratorObject(context_, stack_frame->function_val());
			}
			else {
				generator = new AsyncObject(context_, stack_frame->function_val());
			}

			// 提前分配参数和局部变量栈空间
			generator->stack().upgrade(function_def->var_count());

			// 复制参数和变量(因为变量可能通过BindClosureVars绑定了)
			for (int32_t i = 0; i < function_def->var_count(); ++i) {
				generator->stack().set(i, stack_frame->get(i));
			}

			if (function_def->IsGenerator()) {
				// 是生成器函数
				// 则直接返回生成器对象
				stack_frame->push(Value(generator));
				return false;
			}
			else {
				// 是异步函数
				// 开始执行
				stack_frame->set_function_val(Value(static_cast<AsyncObject*>(generator)));
			}
		}
		return true;
	}
	case ValueType::kNewConstructor: {
		auto obj = stack_frame->function_val().class_def().NewConstructor(context_, par_count, *stack_frame);
		stack_frame->push(std::move(obj));
		return false;
	}
	case ValueType::kCppFunction: {
		// 切换栈帧
		auto ret = stack_frame->function_val().cpp_function()(context_, par_count, *stack_frame);
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
			stack_frame->push(generator.MakeReturnObject(context_, Value()));
			// 已完成，不再需要执行
			return false;
		}

		bool is_first = !generator.IsExecuting();

		GeneratorRestoreContext(stack_frame, &generator);

		// next参数入栈
		if (!is_first) {
			stack_frame->push(next_val);
		}
		return true;
	}
	case ValueType::kPromiseResolve:
	case ValueType::kPromiseReject: {
		auto& promise = stack_frame->function_val().promise();

		Value value;
		if (par_count > 0) {
			value = stack_frame->get(-1);
		}
		stack_frame->reduce(par_count);

		if (stack_frame->function_val().type() == ValueType::kPromiseResolve) {
			promise.Resolve(context_, value);
		}
		else if (stack_frame->function_val().type() == ValueType::kPromiseReject) {
			promise.Reject(context_, value);
		}

		stack_frame->push(Value());

		return false;
	}
	case ValueType::kAsyncObject: {
		auto& async = stack_frame->function_val().async();
		auto ret = stack_frame->pop();
		GeneratorRestoreContext(stack_frame, &async);
		stack_frame->push(std::move(ret));
		return true;
	}
	default:
		throw VmException("Non callable type.");
	}
}

void Vm::CallInternal(StackFrame* stack_frame, Value func_val, Value this_val, uint32_t param_count) {
	stack_frame->set_function_val(std::move(func_val));
	stack_frame->set_this_val(std::move(this_val));
	
	std::optional<Value> pending_return_val;
	
	if (!FunctionScheduling(stack_frame, param_count)) {
		goto exit_;
	}

	// std::cout << stack_frame->function_def()->Disassembly(context_);

	{
		std::optional<Value> pending_error_val;
		Pc pending_goto_pc = kInvalidPc;

		auto* func_def = stack_frame->function_def();
		while (stack_frame->pc() >= 0 && func_def && stack_frame->pc() < func_def->byte_code().Size()) {
			// OpcodeType opcode_; uint32_t par; auto pc = stack_frame->pc(); std::cout << func_def->byte_code().Disassembly(context_, pc, opcode_, par, func_def) << std::endl;
			
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
				LoadConst(stack_frame, ConstIndex(opcode - OpcodeType::kCLoad_0));
				break;
			}
			case OpcodeType::kCLoad: {
				auto const_idx = ConstIndex(func_def->byte_code().GetI8(stack_frame->pc()));
				stack_frame->set_pc(stack_frame->pc() + 1);
				LoadConst(stack_frame, const_idx);
				break;
			}
			case OpcodeType::kCLoadW: {
				auto const_idx = ConstIndex(func_def->byte_code().GetI16(stack_frame->pc()));
				stack_frame->set_pc(stack_frame->pc() + 2);
				LoadConst(stack_frame, const_idx);
				break;
			}
			case OpcodeType::kCLoadD: {
				auto const_idx = ConstIndex(func_def->byte_code().GetI32(stack_frame->pc()));
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
				auto val = stack_frame->get(-1);
				SetVar(stack_frame, var_idx, std::move(val));
				break;
			}
			case OpcodeType::kVStore_0:
			case OpcodeType::kVStore_1:
			case OpcodeType::kVStore_2:
			case OpcodeType::kVStore_3: {
				auto var_idx = opcode - OpcodeType::kVStore_0;
				auto val = stack_frame->get(-1);
				SetVar(stack_frame, var_idx, std::move(val));
				break;
			}
			case OpcodeType::kPropertyLoad: {
				auto const_idx = ConstIndex(func_def->byte_code().GetI32(stack_frame->pc()));
				stack_frame->set_pc(stack_frame->pc() + 4);
				auto& obj_val = stack_frame->get(-1);
				bool success = false;
				if (obj_val.IsObject()) {
					auto& obj = obj_val.object();
					success = obj.GetProperty(context_, const_idx, &obj_val);
				}
				else if (obj_val.IsClassDef()) {
					auto& class_def = obj_val.class_def();
					success = class_def.GetStaticProperty(&context_->runtime(), const_idx, &obj_val);
				}
				else {
					// 非Object类型，根据类型来处理
					// 如undefined需要报错
					// number等需要转成临时Number Object
				}

				if (!success) {
					obj_val = Value();
				}
				else if(obj_val.IsUpValue()) {
					obj_val = obj_val.up_value().Up();
				}
				break;
			}
			case OpcodeType::kPropertyStore: {
				auto const_idx = ConstIndex(func_def->byte_code().GetI32(stack_frame->pc()));
				stack_frame->set_pc(stack_frame->pc() + 4);
				auto obj_val = stack_frame->pop();
				auto val = stack_frame->get(-1);
				if (obj_val.IsObject()) {
					auto& obj = obj_val.object();
					obj.SetProperty(context_, const_idx, std::move(val));
				}
				else {
					// 非Object类型，根据类型来处理
					// 如undefined需要报错
					// number等需要转成临时Number Object
					// throw std::runtime_error("unrealized.");

				}
				break;
			}
			case OpcodeType::kIndexedLoad: {
				auto idx_val = stack_frame->pop();
				auto& obj_val = stack_frame->get(-1);
				auto& obj = obj_val.object();

				auto success = obj.GetComputedProperty(context_, idx_val, &obj_val);
				if (!success) {
					obj_val = Value();
				}
				break;
			}
			case OpcodeType::kIndexedStore: {
				auto idx_val = stack_frame->pop();
				auto obj_val = stack_frame->pop();
				auto& obj = obj_val.object();

				auto val = stack_frame->get(-1);
				obj.SetComputedProperty(context_, idx_val, std::move(val));
				break;
			}
			case OpcodeType::kAdd: {
				auto a = stack_frame->pop();
				auto& b = stack_frame->get(-1);
				b = b + a;
				break;
			}
			case OpcodeType::kInc: {
				auto& arg = stack_frame->get(-1);
				++arg;
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
				// this，然后让function call处理
				stack_frame->push(Value());
			}
			case OpcodeType::kFunctionCall: {
				auto this_val = stack_frame->pop();
				auto func_val = stack_frame->pop();
				auto param_count = stack_frame->pop().u64();
				
				auto new_stack_frame = StackFrame(stack_frame);

				// 参数已经在栈上了，调整bottom
				new_stack_frame.set_bottom(
					new_stack_frame.bottom() - param_count
				);

				CallInternal(&new_stack_frame, func_val, this_val, param_count);
				auto& ret = new_stack_frame.get(-1);
				if (ret.IsException()) {
					pending_error_val = std::move(ret);
					if (!ThrowExecption(stack_frame, &pending_error_val)) {
						pending_return_val = std::move(pending_error_val);
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
				auto ret_obj = generator.MakeReturnObject(context_, std::move(ret_value));
				stack_frame->push(std::move(ret_obj));

				goto exit_;
				break;
			}
			case OpcodeType::kAsyncReturn: {
				auto& async = stack_frame->function_val().async();
				async.res_promise().promise().Resolve(context_, stack_frame->pop());
				stack_frame->push(async.res_promise());
				goto exit_;
				break;
			}
			case OpcodeType::kAwait: {
				auto val = stack_frame->pop();

				if (!val.IsPromiseObject()) {
					// 不是Promise，则用Promise包装
					val = PromiseObjectClassDef::Resolve(context_, std::move(val));
				}

				auto& promise = val.promise();
				auto& async_obj = stack_frame->function_val().async();
				GeneratorSaveContext(stack_frame, &async_obj);
				promise.Then(context_, Value(&async_obj), Value());

				stack_frame->push(async_obj.res_promise());

				goto exit_;
				break;
			}
			case OpcodeType::kYield: {
				auto& generator = stack_frame->this_val().generator();

				GeneratorSaveContext(stack_frame, &generator);

				auto ret_value = stack_frame->pop();
				auto ret_obj = generator.MakeReturnObject(context_, std::move(ret_value));
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
				if (!ThrowExecption(stack_frame, &pending_error_val)) {
					pending_return_val = std::move(pending_error_val);
					goto exit_;
				}
				break;
			}
			case OpcodeType::kTryEnd: {
				// 先回到try end
				stack_frame->set_pc(stack_frame->pc() - 1);
				if (pending_error_val) {
					// 如果还有记录的异常，就重抛，这里的重抛是直接到上层抛的，因为try end不属于当前层
					if (!ThrowExecption(stack_frame, &pending_error_val)) {
						goto exit_;
					}
				}
				else if (pending_return_val) {
					// finally完成了，有保存的返回值
					auto& table = func_def->exception_table();
					auto* entry = table.FindEntry(stack_frame->pc());
					if (entry && entry->HasFinally()) {
						// 上层还存在finally，继续执行上层finally
						stack_frame->set_pc(entry->finally_start_pc);
						break;
					}
					stack_frame->push(std::move(*pending_return_val));
					stack_frame->push(stack_frame->pop());
					pending_return_val.reset();
					goto exit_;
				}
				else if (pending_goto_pc != kInvalidPc) {
					// finally完成了，有未完成的跳转
					auto& table = func_def->exception_table();
					auto* entry = table.FindEntry(stack_frame->pc());
					auto* goto_entry = table.FindEntry(pending_goto_pc);
					// Goto是否跳过上层finally
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
				// 存在finally的return语句，先跳转到finally
				auto& table = func_def->exception_table();
				auto* entry = table.FindEntry(stack_frame->pc());
				if (!entry || !entry->HasFinally()) {
					throw VmException("Incorrect finally return.");
				}
				pending_return_val = stack_frame->pop();
				if (entry->LocatedInFinally(stack_frame->pc())) {
					// 位于finally的返回，覆盖掉原先的返回，跳转到TryEnd
					stack_frame->set_pc(entry->finally_end_pc);
				}
				else {
					stack_frame->set_pc(entry->finally_start_pc);
				}
				break;
			}
			case OpcodeType::kFinallyGoto: {
				stack_frame->set_pc(stack_frame->pc() - 1);
				// goto会跳过finally，先执行finally
				auto& table = func_def->exception_table();
				auto* entry = table.FindEntry(stack_frame->pc());
				if (!entry || !entry->HasFinally()) {
					throw VmException("Incorrect finally return.");
				}
				pending_goto_pc = func_def->byte_code().CalcPc(stack_frame->pc());
				if (entry->LocatedInFinally(stack_frame->pc())) {
					// 位于finally的goto
					stack_frame->set_pc(entry->finally_end_pc);
				}
				else {
					stack_frame->set_pc(entry->finally_start_pc);
				}
				break;
			}
			case OpcodeType::kGetModule: {
				auto path = stack_frame->pop();
				if (!path.IsString()) {
					throw VmException("Can only provide string paths for module loading.");
				}

				stack_frame->push(context_->runtime().load_module()(context_, path.string_view()));
				break;
			}
			case OpcodeType::kGetModuleAsync: {
				auto path = stack_frame->pop();
				if (!path.IsString()) {
					throw VmException("Can only provide string paths for module loading.");
				}
				auto module = context_->runtime().load_module_async()(context_, path.string_view());
				stack_frame->push(std::move(module));
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
		if (stack_frame->function_val().IsAsyncObject()) {
			auto& async = stack_frame->function_val().async();
			async.res_promise().promise().Reject(context_, *pending_return_val);
			pending_return_val = async.res_promise();
		}
	}

	// 还原栈帧
	stack().resize(stack_frame->bottom());
	stack_frame->push(std::move(*pending_return_val));
	return;
}


bool Vm::ThrowExecption(StackFrame* stack_frame, std::optional<Value>* error_val) {
	auto& table = stack_frame->function_def()->exception_table();

	auto* entry = table.FindEntry(stack_frame->pc());
	if (!entry) {
		return false;
	}

	if (entry->LocatedInTry(stack_frame->pc())) {
		// 位于try中抛出的异常
		if (entry->HasCatch()) {
			// 进了catch，catch可能会异常，如果异常了要先执行finally，然后继续上抛
			SetVar(stack_frame, entry->catch_err_var_idx, std::move(**error_val));
			error_val->reset();
			stack_frame->set_pc(entry->catch_start_pc);
		}
		else {
			// 没有catch，保存error_val，等finally末尾的rethrow重抛
			stack_frame->set_pc(entry->finally_start_pc);
		}
	}
	else if (entry->HasCatch() && entry->LocatedInCatch(stack_frame->pc())) {
		if (entry->HasFinally()) {
			stack_frame->set_pc(entry->finally_start_pc);
		}
	}
	else if (entry->HasFinally() && entry->LocatedInFinally(stack_frame->pc())) {
		// finally抛出异常，覆盖错误并且跳转到最后的TryEnd
		stack_frame->set_pc(entry->finally_end_pc);
	}
	else {
		throw VmException("Incorrect execption address.");
	}
	return true;
}

void Vm::GeneratorSaveContext(StackFrame* stack_frame, GeneratorObject* generator) {
	// 保存当前生成器的pc
	generator->set_pc(stack_frame->pc());

	// 保存当前栈帧到generator的栈帧中
	auto& gen_vector = generator->stack().vector();
	for (int32_t i = 0; i < gen_vector.size(); ++i) {
		gen_vector[i] = std::move(stack_frame->get(i));
	}
}

void Vm::GeneratorRestoreContext(StackFrame* stack_frame, GeneratorObject* generator) {
	// 复制栈帧
	auto& vector = stack().vector();
	auto& gen_vector = generator->stack().vector();
	vector.insert(vector.end(), gen_vector.begin(), gen_vector.end());

	generator->SetExecuting();

	stack_frame->set_function_def(&generator->function_def());
	stack_frame->set_pc(generator->pc());
}

Stack& Vm::stack() {
	return context_->runtime().stack();
}

FunctionDef* Vm::function_def(const Value& func_val) const {
	if (func_val.IsFunctionObject()) {
		return &func_val.function().function_def();
	}
	else if (func_val.IsModuleObject()) {
		return &func_val.module().function_def();
	}
	else if (func_val.IsFunctionDef()) {
		return &func_val.function_def();
	}
	return nullptr;
	throw std::runtime_error("Unavailable function definition.");
}

} // namespace mjs