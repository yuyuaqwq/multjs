#include <mjs/vm.h>

#include <iostream>

#include <mjs/error.h>
#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/opcode.h>
#include <mjs/object_impl/array_object.h>
#include <mjs/object_impl/function_object.h>
#include <mjs/object_impl/generator_object.h>
#include <mjs/object_impl/async_object.h>
#include <mjs/object_impl/promise_object.h>
#include <mjs/object_impl/module_object.h>
#include <mjs/object_impl/constructor_object.h>
#include <mjs/class_def_impl/promise_object_class_def.h>

namespace mjs {

VM::VM(Context* context)
	: context_(context) {}

void VM::ModuleInit(Value* module_def_value) {
	auto& module_def = module_def_value->module_def();
	if (module_def.export_var_def_table().export_var_defs().empty()) {
		return;
	}

	auto module_obj = ModuleObject::New(context_, &module_def);
	*module_def_value = Value(module_obj);

	for (auto& def : module_obj->module_def().export_var_def_table().export_var_defs()) {
		module_obj->SetProperty(context_, context_->FindConstOrInsertToGlobal(Value(String::New(def.first))), 
			Value(&module_obj->module_env().export_vars()[def.second.export_var_index])
		);
	}
}

void VM::BindModuleExportVars(StackFrame* stack_frame) {
	auto& func_val = stack_frame->function_val();

	auto& module_obj = func_val.module();
	for (auto& def : module_obj.module_def().export_var_def_table().export_var_defs()) {
		// 栈上的Value关联到导出变量
		stack_frame->set(def.second.var_index, 
			Value(&module_obj.module_env().export_vars()[def.second.export_var_index])
		);
	}
}


Value& VM::GetVar(StackFrame* stack_frame, VarIndex idx) {
	auto* var = &stack_frame->get(idx);
	if (var->IsClosureVar()) {
		var = &var->closure_var().value();
	}
	else if (var->IsExportVar()) {
		var = &var->export_var().value();
	}
	return *var;
}

void VM::SetVar(StackFrame* stack_frame, VarIndex idx, Value&& var) {
	auto* var_ = &stack_frame->get(idx);
	if (var_->IsClosureVar()) {
		var_ = &var_->closure_var().value();
	}
	else if (var_->IsExportVar()) {
		var_ = &var_->export_var().value();
	}
	*var_ = std::move(var);
}


void VM::Closure(const StackFrame& stack_frame, Value* func_def_val) {
	auto& func_def = func_def_val->function_def();
	assert(!func_def.closure_var_table().closure_var_defs().empty() || func_def.has_this() && func_def.is_normal());
	
	*func_def_val = Value(FunctionObject::New(context_, &func_def));
	auto* func_obj = &func_def_val->function();

	auto& env = func_obj->closure_env();

	// 捕获变量
	auto& closure_var_defs = func_def.closure_var_table().closure_var_defs();
	for (auto& def : closure_var_defs) {
		auto& var = stack_frame.get(def.second.parent_var_idx);
		if (!var.IsClosureVar()) {
			var = Value(new ClosureVar(std::move(var)));
		}
		env.closure_var_refs()[def.second.env_var_idx] = Value(&var.closure_var());
	}

	if (func_def.has_this() && func_def.is_arrow()) {
		// 捕获下this
		Value this_val = stack_frame.this_val();
		// this_val = Value(new ClosureVar(std::move(this_val)));
		env.set_lexical_this(std::move(this_val));
	}
}

void VM::BindClosureVars(StackFrame* stack_frame) {
	auto& func_val = stack_frame->function_val();
	auto* func_def = stack_frame->function_def();
	FunctionObject* func_obj = &stack_frame->function_val().function();

	// 需要将栈上对应的局部变量绑定到堆上的ClosureVar
	auto& env = func_obj->closure_env();
	for (auto& def : func_def->closure_var_table().closure_var_defs()) {
		// 栈上的Value关联到闭包变量
		stack_frame->set(def.first, Value(&env.closure_var_refs()[def.second.env_var_idx].closure_var()));
	}
}

void VM::LoadConst(StackFrame* stack_frame, ConstIndex const_idx) {
	stack_frame->push(context_->GetConstValue(const_idx));
}

// 返回值决定是否进vm执行指令
bool VM::FunctionScheduling(StackFrame* stack_frame, uint32_t par_count) {
	switch (stack_frame->function_val().type()) {
	case ValueType::kModuleDef:
	case ValueType::kModuleObject:
	case ValueType::kFunctionDef:
	case ValueType::kFunctionObject: {
		stack_frame->set_function_def(function_def(stack_frame->function_val()));
		auto* function_def = stack_frame->function_def();

		// printf("%s\n", function_def->Disassembly().c_str());

		if (par_count < function_def->param_count()) {
			stack_frame->push(
				Error::Throw(context_, "Wrong number of parameters passed when calling the function.")
			);
			return false;
		}

		// 弹出多余参数
		stack().reduce(par_count - function_def->param_count());

		assert(function_def->var_def_table().var_count() >= function_def->param_count());
		stack_frame->upgrade(function_def->var_def_table().var_count() - function_def->param_count());
		if (stack_frame->function_val().type() == ValueType::kFunctionObject) {
			BindClosureVars(stack_frame);
		}
		else if (stack_frame->function_val().type() == ValueType::kModuleObject) {
			BindModuleExportVars(stack_frame);
		}

		if (function_def->is_generator() || function_def->is_async()) {
			GeneratorObject* generator;
			if (function_def->is_generator()) {
				generator = GeneratorObject::New(context_, stack_frame->function_val());
			}
			else {
				generator = AsyncObject::New(context_, stack_frame->function_val());
			}

			// 提前分配参数和局部变量栈空间
			generator->stack().upgrade(function_def->var_def_table().var_count());

			if (function_def->is_generator() || stack_frame->function_val().type() == ValueType::kFunctionObject) {
				// 复制参数和变量(因为变量可能通过BindClosureVars绑定了)
				for (int32_t i = 0; i < function_def->var_def_table().var_count(); ++i) {
					generator->stack().set(i, stack_frame->get(i));
				}
			}
			assert(stack_frame->function_val().type() != ValueType::kModuleObject);

			if (function_def->is_generator()) {
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
	case ValueType::kConstructorObject: {
		auto target_class_id = stack_frame->function_val().constructor().target_class_id();
		auto& target_class_def = context_->runtime().class_def_table()[target_class_id];
		auto obj = target_class_def.NewConstructor(context_, par_count, *stack_frame);
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
	case ValueType::kAsyncResolveResume: {
		// Async函数恢复执行，目前由await触发
		auto& async = stack_frame->function_val().async();
		auto ret = stack_frame->pop();
		GeneratorRestoreContext(stack_frame, &async);
		stack_frame->push(ret);
		return true;
	}
	case ValueType::kAsyncRejectResume: {
		// Async函数恢复执行，目前由await触发
		auto& async = stack_frame->function_val().async();
		auto ret = stack_frame->pop();
		GeneratorRestoreContext(stack_frame, &async);
		stack_frame->push(ret);
		return false;
	}
	default:
	    stack_frame->push(
			TypeError::Throw(context_, "Non callable type: '{}'.", Value::TypeToString(stack_frame->function_val().type()))
		);
		return false;
	}
}

#define VM_EXCEPTION_CHECK_AND_THROW(VALUE) \
	if (VALUE.IsException()) { \
		pending_error_val = std::move(VALUE); \
		if (!ThrowException(stack_frame, &pending_error_val)) { \
			pending_return_val = std::move(pending_error_val); \
			goto exit_; \
		} \
		break; \
	} \

#define VM_EXCEPTION_THROW(VALUE) \
	pending_error_val = std::move(VALUE); \
	if (!ThrowException(stack_frame, &pending_error_val)) { \
		pending_return_val = std::move(pending_error_val); \
		goto exit_; \
	} \
    break;



void VM::CallInternal(StackFrame* stack_frame, Value func_val, Value this_val, uint32_t param_count) {
	stack_frame->set_function_val(std::move(func_val));
	stack_frame->set_this_val(std::move(this_val));
	
	std::optional<Value> pending_return_val;	// 异常时等待返回的值，需要执行finally才能返回
	std::optional<Value> pending_error_val;		// 异常处理过程中临时保存的异常值，没有被catch处理最后会被重抛
	OpcodeType opcode;
	Pc pending_goto_pc = kInvalidPc;
	const FunctionDefBase* func_def = nullptr;

	if (!FunctionScheduling(stack_frame, param_count)) {
		if (stack_frame->function_val().IsAsyncRejectResume()) {
			// await等待的promise被拒绝，注入异常
			goto inject_exception_;
		}

		// todo: 暂时有点丑陋，先这么干吧，以后再来整体优化
		pending_return_val = stack_frame->pop();
		goto return_;
	}

	// std::cout << stack_frame->function_def()->Disassembly(context_);
	
	func_def = stack_frame->function_def();
	while (stack_frame->pc() >= 0 && func_def && stack_frame->pc() < func_def->bytecode_table().Size()) {
		//{
		//	OpcodeType opcode_; uint32_t par; auto pc = stack_frame->pc(); std::cout << func_def->bytecode_table().Disassembly(context_, pc, opcode_, par, func_def) << std::endl;
		//}

		opcode = func_def->bytecode_table().GetOpcode(stack_frame->pc());
		stack_frame->set_pc(stack_frame->pc() + 1);
		switch (opcode) {
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
			auto const_idx = ConstIndex(func_def->bytecode_table().GetI8(stack_frame->pc()));
			stack_frame->set_pc(stack_frame->pc() + 1);
			LoadConst(stack_frame, const_idx);
			break;
		}
		case OpcodeType::kCLoadW: {
			auto const_idx = ConstIndex(func_def->bytecode_table().GetI16(stack_frame->pc()));
			stack_frame->set_pc(stack_frame->pc() + 2);
			LoadConst(stack_frame, const_idx);
			break;
		}
		case OpcodeType::kCLoadD: {
			auto const_idx = ConstIndex(func_def->bytecode_table().GetI32(stack_frame->pc()));
			stack_frame->set_pc(stack_frame->pc() + 4);
			LoadConst(stack_frame, const_idx);
			break;
		}
		case OpcodeType::kVLoad: {
			auto var_idx = func_def->bytecode_table().GetU8(stack_frame->pc());
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
			auto var_idx = func_def->bytecode_table().GetU8(stack_frame->pc());
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
		case OpcodeType::kClosure: {
			auto const_idx = ConstIndex(func_def->bytecode_table().GetI32(stack_frame->pc()));
			stack_frame->set_pc(stack_frame->pc() + 4);
			auto value = context_->GetConstValue(const_idx);
			Closure(*stack_frame, &value);
			stack_frame->push(std::move(value));
			break;
		}
		case OpcodeType::kPropertyLoad: {
			auto const_idx = ConstIndex(func_def->bytecode_table().GetI32(stack_frame->pc()));
			stack_frame->set_pc(stack_frame->pc() + 4);
			auto& obj_val = stack_frame->get(-1);
			bool success = false;
			if (obj_val.IsObject()) {
				auto& obj = obj_val.object();
				success = obj.GetProperty(context_, const_idx, &obj_val);
			}
			else {
				// 非Object类型，根据类型来处理
				// 如undefined需要报错
				// number等需要转成临时Number Object
				success = obj_val.GetProperty(context_, const_idx, &obj_val);
			}

			if (!success) {
				obj_val = Value();
			}
			break;
		}
		case OpcodeType::kPropertyStore: {
			auto const_idx = ConstIndex(func_def->bytecode_table().GetI32(stack_frame->pc()));
			stack_frame->set_pc(stack_frame->pc() + 4);
			auto obj_val = stack_frame->pop();
			auto val = stack_frame->get(-1);
			if (obj_val.IsObject()) {
				auto& obj = obj_val.object();
				obj.SetProperty(context_, const_idx, std::move(val));
			}
			else {
				throw std::runtime_error("Cannot modify the properties of temporary objects.");
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
		case OpcodeType::kToString: {
			auto& a = stack_frame->get(-1);
			a = a.ToString(context_);
			break;
		}
		case OpcodeType::kAdd: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.Add(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kInc: {
			auto& arg = stack_frame->get(-1);
			arg = arg.Increment(context_);
			VM_EXCEPTION_CHECK_AND_THROW(arg);
			break;
		}
		case OpcodeType::kSub: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.Subtract(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kMul: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.Multiply(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kDiv: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.Divide(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kNeg: {
			auto& a = stack_frame->get(-1);
			a = a.Negate(context_);
			VM_EXCEPTION_CHECK_AND_THROW(a);
			break;
		}

		case OpcodeType::kShl: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.LeftShift(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kShr: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.RightShift(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kBitAnd: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.BitwiseAnd(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kBitOr: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.BitwiseOr(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
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
			VM_EXCEPTION_CHECK_AND_THROW(ret);
			break;
		}
		case OpcodeType::kGetThis: {
			stack_frame->push(stack_frame->this_val());
			break;
		}
		case OpcodeType::kGetOuterThis: {
			auto& lexical_this = stack_frame->function_val().function().closure_env().lexical_this();
			stack_frame->push(lexical_this);
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
			auto return_value = stack_frame->pop();
			async.res_promise().promise().Resolve(context_, return_value);
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

			// 在被等待的promise被解决或拒绝后，恢复当前async函数的执行
			promise.Then(context_, Value(ValueType::kAsyncResolveResume, &async_obj), Value(ValueType::kAsyncRejectResume, &async_obj));

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
			b = b.NotEqualTo(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kEq: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.EqualTo(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kLt: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.LessThan(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kLe: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.LessThanOrEqual(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kGt: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.GreaterThan(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kGe: {
			auto a = stack_frame->pop();
			auto& b = stack_frame->get(-1);
			b = b.GreaterThanOrEqual(context_, a);
			VM_EXCEPTION_CHECK_AND_THROW(b);
			break;
		}
		case OpcodeType::kIfEq: {
			auto boolean_val = stack_frame->pop().ToBoolean();
			if (boolean_val.boolean() == false) {
				stack_frame->set_pc(func_def->bytecode_table().CalcPc(stack_frame->pc() - 1));
			}
			else {
				stack_frame->set_pc(stack_frame->pc() + 2);
			}
			break;
		}
		case OpcodeType::kGoto: {
			stack_frame->set_pc(func_def->bytecode_table().CalcPc(stack_frame->pc() - 1));
			break;
		}
		case OpcodeType::kTryBegin: {
			break;
		}
		case OpcodeType::kThrow: {
			stack_frame->set_pc(stack_frame->pc() - 1);
			VM_EXCEPTION_THROW(stack_frame->pop());
			break;
		}
		case OpcodeType::kTryEnd: {
			// 先回到try end
			stack_frame->set_pc(stack_frame->pc() - 1);
			if (pending_error_val) {
				// 如果还有记录的异常，就重抛，这里的重抛是直接到上层抛的，因为try end不属于当前层
				if (!ThrowException(stack_frame, &pending_error_val)) {
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
				throw std::runtime_error("Incorrect finally return.");
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
				throw std::runtime_error("Incorrect finally return.");
			}
			pending_goto_pc = func_def->bytecode_table().CalcPc(stack_frame->pc());
			if (entry->LocatedInFinally(stack_frame->pc())) {
				// 位于finally的goto
				stack_frame->set_pc(entry->finally_end_pc);
			}
			else {
				stack_frame->set_pc(entry->finally_start_pc);
			}
			break;
		}
		case OpcodeType::kGetGlobal: {
			auto const_idx = ConstIndex(func_def->bytecode_table().GetU32(stack_frame->pc()));
			stack_frame->set_pc(stack_frame->pc() + 4);
			Value value;
			auto success = context_->runtime().global_this().object().GetProperty(context_, const_idx, &value);
			if (!success) {
				VM_EXCEPTION_THROW(
					ReferenceError::Throw(context_, "Failed to retrieve properties from global this: {}.", context_->GetConstValue(const_idx).ToString(context_).string_view())
				);
			}
			stack_frame->push(std::move(value));
			break;
		}
		case OpcodeType::kGetModule: {
			auto path = stack_frame->pop();
			if (!path.IsString()) {
				VM_EXCEPTION_THROW(
					TypeError::Throw(context_, "Can only provide string paths for module loading.")
				);
			}
			auto module = context_->runtime().module_manager().GetModule(context_, path.string_view());
			stack_frame->push(module);
			VM_EXCEPTION_CHECK_AND_THROW(module);
			break;
		}
		case OpcodeType::kGetModuleAsync: {
			auto path = stack_frame->pop();
			if (!path.IsString()) {
				VM_EXCEPTION_THROW(
					TypeError::Throw(context_, "Can only provide string paths for module loading.")
				);
			}
			auto module = context_->runtime().module_manager().GetModuleAsync(context_, path.string_view());
			stack_frame->push(module);
			VM_EXCEPTION_CHECK_AND_THROW(module);
			break;
		}
		default:
			throw std::runtime_error("Unknown instruction.");
			break;
		{
		inject_exception_:
			VM_EXCEPTION_THROW(stack_frame->pop());
			break;
		}
		}
	}
	

exit_:
	if (!pending_return_val) {
		pending_return_val = stack_frame->pop();
		assert(!pending_return_val->IsException());
	}
	else {
		assert(pending_return_val->IsException());
		// pending_return_val->SetException();

		const mjs::StackFrame* tmp_stack_frame = stack_frame;
		while (!tmp_stack_frame->function_def()) {
			if (!tmp_stack_frame->upper_stack_frame()) {
				break;
			}
			tmp_stack_frame = tmp_stack_frame->upper_stack_frame();
		}
		std::string_view func = "<unknown>";
		SourceLine line = kInvalidSourceLine;
		if (tmp_stack_frame->function_def()) {
			func = tmp_stack_frame->function_def()->name();
			auto debug_info = tmp_stack_frame->function_def()->debug_table().FindEntry(tmp_stack_frame->pc());
			if (debug_info) {
				line = debug_info->source_line;
			}
		}
		pending_return_val = Value(String::Format("\n\t[func:{}, line:{}] {}", func, line, pending_return_val->ToString(context_).string_view())).SetException();

		if (stack_frame->function_val().IsAsyncObject()
			|| stack_frame->function_val().IsAsyncResolveResume() 
			|| stack_frame->function_val().IsAsyncRejectResume()) {
			auto& async = stack_frame->function_val().async();
			async.res_promise().promise().Reject(context_, *pending_return_val);
			pending_return_val = async.res_promise();
		}
		else if (stack_frame->function_val().IsGeneratorNext()) {
			auto& generator = stack_frame->this_val().generator();
			generator.SetClosed();
		}
	}

return_:
	// 还原栈帧
	stack().resize(stack_frame->bottom());
	stack_frame->push(std::move(*pending_return_val));
	return;
}


bool VM::ThrowException(StackFrame* stack_frame, std::optional<Value>* error_val) {
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
		throw std::runtime_error("Incorrect execption address.");
	}
	return true;
}

void VM::GeneratorSaveContext(StackFrame* stack_frame, GeneratorObject* generator) {
	// 保存当前生成器的pc
	generator->set_pc(stack_frame->pc());

	// 保存当前栈帧到generator的栈帧中
	auto& gen_vector = generator->stack().vector();
	for (int32_t i = 0; i < gen_vector.size(); ++i) {
		gen_vector[i] = std::move(stack_frame->get(i));
	}
}

void VM::GeneratorRestoreContext(StackFrame* stack_frame, GeneratorObject* generator) {
	// 复制栈帧
	auto& vector = stack().vector();
	auto& gen_vector = generator->stack().vector();
	vector.insert(vector.end(), gen_vector.begin(), gen_vector.end());

	generator->SetExecuting();

	stack_frame->set_function_def(&generator->function_def());
	stack_frame->set_pc(generator->pc());
}

Stack& VM::stack() {
	return context_->runtime().stack();
}

FunctionDefBase* VM::function_def(const Value& func_val) const {
	if (func_val.IsFunctionDef()) {
		return &func_val.function_def();
	}
	else if (func_val.IsFunctionObject()) {
		return &func_val.function().function_def();
	}
	if (func_val.IsModuleDef()) {
		return &func_val.module_def();
	}
	else if (func_val.IsModuleObject()) {
		return &func_val.module().module_def();
	}
	return nullptr;
	throw std::runtime_error("Unavailable function definition.");
}

} // namespace mjs