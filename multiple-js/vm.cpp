#include "vm.h"

namespace mjs {

VM::VM(ConstPool* const_pool)
	: const_pool_(const_pool)
{
	cur_func_ = const_pool_->Get(0).function_body();
}

std::string VM::Disassembly() {
	return cur_func_->Disassembly();
}

Value& VM::GetVar(uint32_t idx) {
	if (cur_func_->stack_frame.Get(idx).type() == ValueType::kUpValue) {
		// upvalue可能形成链表(代码生成阶段，根据从外层作用域名字找到了变量，但是该变量实际上也是upvalue)，因此要重复向上找直到不是upvalue
		// 有时间可以从代码生成那边优化，也是做循环向上找，直到不再指向upvalue
		auto func = cur_func_;
		auto up_value = func->stack_frame.Get(idx).up_value();
		while (up_value->func_body->stack_frame.Get(up_value->index).type() == ValueType::kUpValue) {
			func = up_value->func_body;
			up_value = func->stack_frame.Get(up_value->index).up_value();
		}
		return up_value->func_body->stack_frame.Get(up_value->index);
	}
	return cur_func_->stack_frame.Get(idx);
}

void VM::SetVar(uint32_t idx, Value&& var) {
	if (idx >= cur_func_->stack_frame.Size()) {
		cur_func_->stack_frame.ReSize(idx + 1);
	}

	else if (cur_func_->stack_frame.Get(idx).type() == ValueType::kUpValue) {
		auto func = cur_func_;
		auto up_value = func->stack_frame.Get(idx).up_value();
		while (up_value->func_body->stack_frame.Get(up_value->index).type() == ValueType::kUpValue) {
			func = up_value->func_body;
			up_value = func->stack_frame.Get(up_value->index).up_value();
		}
		up_value->func_body->stack_frame.Set(up_value->index, std::move(var));
		return;
	}

	cur_func_->stack_frame.Set(idx, std::move(var));
}

void VM::SetVar(uint32_t idx, Value* var) {
	SetVar(idx, std::move(*var));
}


void VM::Run() {
	do {
		auto opcode = cur_func_->byte_code.GetOpcode(pc_++);
		switch (opcode) {
		case OpcodeType::kCLoad_0: {
		case OpcodeType::kCLoad_1:
		case OpcodeType::kCLoad_2:
		case OpcodeType::kCLoad_3:
		case OpcodeType::kCLoad_4:
		case OpcodeType::kCLoad_5:
			auto const_idx = opcode - OpcodeType::kCLoad_0;
			stack_frame_.Push(const_pool_->Get(const_idx));
			break;
		}
		case OpcodeType::kCLoad: {
			auto const_idx = cur_func_->byte_code.GetU8(pc_);
			pc_ += 1;
			stack_frame_.Push(const_pool_->Get(const_idx));
			break;
		}
		case OpcodeType::kCLoadW: {
			auto const_idx = cur_func_->byte_code.GetU16(pc_);
			pc_ += 2;
			stack_frame_.Push(const_pool_->Get(const_idx));
			break;
		}
		case OpcodeType::kVLoad: {
			auto var_idx = cur_func_->byte_code.GetU8(pc_);
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
			auto var_idx = cur_func_->byte_code.GetU8(pc_);
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
		case OpcodeType::kAdd: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = a + b;
			break;
		}
		case OpcodeType::kSub: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = a - b;
			break;
		}
		case OpcodeType::kMul: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = a * b;
			break;
		}
		case OpcodeType::kDiv: {
			auto a = stack_frame_.Pop();
			auto& b = stack_frame_.Get(-1);
			b = a / b;
			break;
		}
		case OpcodeType::kInvokeStatic: {
			auto var_idx = cur_func_->byte_code.GetU16(pc_);
			pc_ += 2;

			auto& func = GetVar(var_idx);
			auto par_count = stack_frame_.Pop().u64();

			if (func.type() == ValueType::kFunctionBody) {
				auto call_func = func.function_body();

				//printf("%s\n", callFunc->Disassembly().c_str());

				if (par_count < call_func->par_count) {
					throw VMException("Wrong number of parameters passed when calling the function");
				}

				auto save_func = cur_func_;
				auto save_pc = pc_;

				// 切换环境
				cur_func_ = call_func;
				pc_ = 0;

				// 移动栈上的参数到新函数的局部变量表
				//m_varIdxBase = m_var_sect.size();
				for (int i = cur_func_->par_count - 1; i >= 0; i--) {
					SetVar(i, std::move(stack_frame_.Pop()));
				}

				// 保存当前环境
				stack_frame_.Push(Value((uint64_t)save_func));
				stack_frame_.Push(Value(save_pc));
			}
			else if (func.type() == ValueType::kFunctionBridge) {
				auto call_func = func.function_bridge();
				stack_frame_.Push(call_func(par_count, &stack_frame_));
			}
			break;
		}
		case OpcodeType::kReturn: {
			auto ret_value = stack_frame_.Pop();
			auto pc = stack_frame_.Pop();
			auto& cur_func = stack_frame_.Get(-1);

			// 恢复环境
			cur_func_ = cur_func.object<FunctionBodyObject>();
			pc_ = pc.u64();

			cur_func = std::move(ret_value);
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
			auto offset = cur_func_->byte_code.GetU16(pc_);
			pc_ += 2;
			auto boolean = stack_frame_.Pop().boolean();
			if (boolean == false) {
				pc_ = cur_func_->byte_code.CalcPc(offset);
			}
			break;
		}
		case OpcodeType::kGoto: {
			auto offset = cur_func_->byte_code.GetU16(pc_);
			pc_ = cur_func_->byte_code.CalcPc(offset);
			break;
		}
		default:
			throw VMException("Unknown instruction");
		}
	} while (pc_ < cur_func_->byte_code.Size());
}

} // namespace mjs