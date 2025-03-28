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
	stack().Resize(function_def(cur_func_val_)->var_count);

	// 最外层的函数不会通过CLoadFunc加载，所以需要自己初始化
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

	if (func == nullptr || func_def == nullptr) {
		return;
	}

	// 调用的是函数对象，可能需要处理闭包内的upvalue
	auto& arr = func->closure_value_arr_;
	for (auto& def : func_def->closure_var_defs_) {
		// 栈上的对象通过upvalue关联到闭包变量
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
			// array需要初始化为upvalue，指向父函数的ArrayValue
			// 如果没有父函数就不需要，即是顶层函数，默认初始化为未定义

			auto func = func_val.function();
			auto parent_func = cur_func_val_.function();

			// 递增父函数的引用计数，用于延长父函数中的ArrayValue的生命周期
			func->parent_function_ = cur_func_val_;

			// 引用到父函数的ArrayValue
			auto& arr = func->closure_value_arr_;
			auto& parent_arr = parent_func->closure_value_arr_;

			for (auto& def : func->func_def_->closure_var_defs_) {
				if (!def.second.parent_var_idx) {
					// 当前是顶级变量
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

void Vm::SaveStackFrame(FunctionDef** cur_func_def, const Value& func_val, FunctionDef* func_def
	, Value&& this_val, uint32_t par_count, bool is_generator
	)
{
	auto save_func = cur_func_val_;
	auto save_pc = pc_;
	auto save_bottom = stack_frame_.bottom();

	// 栈帧布局：
		// 返回信息
		// 局部变量
		// 参数1
		// ...
		// 参数n    <- bottom
		// 上一栈帧

	// 切换环境和栈帧
	*cur_func_def = func_def;
	cur_func_val_ = func_val;

	// 参数已经入栈了，再分配局部变量部分
	if (!is_generator) {
		assert(stack().Size() >= (*cur_func_def)->par_count);
		stack_frame_.set_bottom(stack().Size() - (*cur_func_def)->par_count);

		assert((*cur_func_def)->var_count >= (*cur_func_def)->par_count);
		stack().Upgrade((*cur_func_def)->var_count - (*cur_func_def)->par_count);
	}
	else {
		stack_frame_.set_bottom(stack().Size() - (*cur_func_def)->var_count);
	}

	// 保存当前环境，用于Ret返回
	stack_frame_.Push(Value(save_func));
	stack_frame_.Push(Value(save_pc));
	stack_frame_.Push(Value(save_bottom));

	stack_frame_.set_this_val(std::move(this_val));

	if (cur_func_val_.IsFunctionObject()) {
		FunctionEnterInit(cur_func_val_);
	}
}


Value Vm::RestoreStackFrame(FunctionDef** cur_func_def) {
	// 将要返回，需要提升当前栈帧上的闭包变量到堆中
	// 拿到返回之后的函数变量，将其赋值为

	auto ret_value = stack_frame_.Pop();

	auto save_bottom = stack_frame_.Pop();
	auto save_pc = stack_frame_.Pop();
	auto save_func = stack_frame_.Pop();

	// 恢复环境和栈帧
	cur_func_val_ = save_func;
	*cur_func_def = function_def(cur_func_val_);
	pc_ = save_pc.u64();

	// 设置栈顶和栈底
	stack().Resize(stack_frame_.bottom());
	stack_frame_.set_bottom(save_bottom.u64());

	// 返回位于原本位于栈帧栈顶的返回值
	return ret_value;
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
		case OpcodeType::kDump: {
			stack_frame_.Push(stack_frame_.Get(-1));
			break;
		}
		case OpcodeType::kSwap: {
			std::swap(stack_frame_.Get(-1), stack_frame_.Get(-2));
			break;
		}
		case OpcodeType::kUndefined: {
			stack_frame_.Push(Value());
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
				// 非Object类型，根据类型来处理
				// 如undefined需要报错
				// number等需要转成临时Number Object

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
				// 非Object类型，根据类型来处理
				// 如undefined需要报错
				// number等需要转成临时Number Object
				// throw std::runtime_error("unrealized.");
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

			auto this_val = stack_frame_.Pop();
			auto func_val = stack_frame_.Pop();
			FunctionSwitch(&cur_func_def, std::move(this_val), std::move(func_val));
			break;
		}
		case OpcodeType::kGetThis: {
			stack_frame_.Push(stack_frame_.this_val());
			break;
		}
		case OpcodeType::kSetThis: {
			stack_frame_.set_this_val(stack_frame_.Pop());
			break;
		}
		case OpcodeType::kReturn: {
			stack_frame_.Push(RestoreStackFrame(&cur_func_def));
			break;
		}
		case OpcodeType::kGeneratorReturn: {
			auto generator = stack_frame_.this_val().generator();

			generator->SetClosed();

			auto ret_value = RestoreStackFrame(&cur_func_def);
			auto ret_obj = generator->MakeReturnObject(std::move(ret_value));
			stack_frame_.Push(std::move(ret_obj));
			break;
		}
		case OpcodeType::kYield: {
			auto generator = stack_frame_.this_val().generator();

			// 保存当前生成器的pc
			generator->set_pc(pc_);

			// 保存当前栈帧到generator的栈帧中
			auto& gen_vector = generator->stack().vector();
			for (int32_t i = 0; i < gen_vector.size(); ++i) {
				gen_vector[i] = std::move(stack_frame_.Get(i));
			}

			auto ret_value = RestoreStackFrame(&cur_func_def);
			auto ret_obj = generator->MakeReturnObject(std::move(ret_value));
			stack_frame_.Push(std::move(ret_obj));
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

void Vm::FunctionSwitch(FunctionDef** cur_func_def, Value&& this_val, Value&& func_val) {
	auto par_count = stack_frame_.Pop().u64();

	switch (func_val.type()) {
	case ValueType::kFunctionDef:
	case ValueType::kFunctionObject: {
		auto func_def = function_def(func_val);

		if (func_def->is_generator) {
			// 是生成器函数
			// 直接返回生成器对象
			
			// 提前分配参数和局部变量栈空间
			auto generator = new GeneratorObject(context_->runtime(), func_val);
			generator->stack().Upgrade(func_def->var_count);

			// 弹出多余参数
			stack().Reduce(par_count - func_def->par_count);

			// 复制参数
			for (int32_t i = func_def->par_count - 1; i >= 0; --i) {
				generator->stack().Set(i, std::move(stack().Pop()));
			}

			stack_frame_.Push(Value(generator));
			return;
		}
		// printf("%s\n", func_def->Disassembly().c_str());

		if (par_count < func_def->par_count) {
			throw VmException("Wrong number of parameters passed when calling the function");
		}

		// 把多余的参数弹出
		stack().Reduce(par_count - func_def->par_count);
		SaveStackFrame(cur_func_def, func_val, func_def, std::move(this_val), par_count, false);
		pc_ = 0;
		break;
	}
	case ValueType::kCppFunction: {
		// 切换栈帧
		auto old_bottom = stack_frame_.bottom();
		stack_frame_.set_bottom(stack().Size() - par_count);

		auto ret = func_val.cpp_function()(context_, par_count, &stack_frame_);

		// 还原栈帧
		stack_frame_.set_bottom(old_bottom);
		stack().Reduce(par_count);
		stack_frame_.Push(std::move(ret));
		break;
	}
	case ValueType::kGeneratorNext: {
		auto generator = func_val.generator();

		auto next_val = Value();
		if (par_count > 0) {
			stack().Reduce(par_count - 1);
			next_val = stack().Pop();
		}
		if (generator->IsClosed()) {
			stack_frame_.Push(generator->MakeReturnObject(Value()));
			break;
		}

		bool is_exec = generator->IsExecuting();

		// 复制栈帧
		auto& vector = stack().vector();
		auto& gen_vector = generator->stack().vector();
		vector.insert(vector.end(), gen_vector.begin(), gen_vector.end());

		generator->SetExecuting();
		
		SaveStackFrame(cur_func_def, generator->function(), generator->function_def()
			, std::move(this_val), 0, true);
		pc_ = generator->pc();

		// next参数入栈
		if (is_exec) {
			stack().Push(next_val);
		}

		break;
	}
	default:
		throw VmException("Non callable types.");
	}
}

} // namespace mjs