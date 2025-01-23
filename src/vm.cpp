#include <mjs/vm.h>

#include <mjs/runtime.h>
#include <mjs/context.h>
#include <mjs/func_obj.h>

#include "instr.h"

#include <iostream>

namespace mjs {

Vm::Vm(Context* context)
	: context_(context)
	, stack_frame_(&context_->runtime().stack()) {}

const ConstPool& Vm::const_pool() const {
	return context_->runtime().const_pool();
}

void Vm::SetEvalFunction(const Value& func) {
	cur_func_ = func.function_body();
	context_->runtime().stack().ReSize(cur_func_->var_count);
}

Value& Vm::GetVar(uint32_t idx) {
	// if (stack_frame_.Get(idx).type() == ValueType::kUpValue) {
	// }
	return stack_frame_.Get(idx);
}

void Vm::SetVar(uint32_t idx, Value&& var) {
	// if (stack_frame_.Get(idx).type() == ValueType::kUpValue) {
	// }

	stack_frame_.Set(idx, std::move(var));
}


void Vm::Run() {
	if (!cur_func_) return;
	do {
		// auto pc = pc_; std::cout << cur_func_->byte_code.Disassembly(pc) << std::endl;
		auto opcode = cur_func_->byte_code.GetOpcode(pc_++);
		switch (opcode) {
		//case OpcodeType::kStop:
		//	return;
		case OpcodeType::kCLoad_0: {
		case OpcodeType::kCLoad_1:
		case OpcodeType::kCLoad_2:
		case OpcodeType::kCLoad_3:
		case OpcodeType::kCLoad_4:
		case OpcodeType::kCLoad_5:
			auto const_idx = opcode - OpcodeType::kCLoad_0;
			stack_frame_.Push(const_pool().Get(const_idx));
			break;
		}
		case OpcodeType::kCLoad: {
			auto const_idx = cur_func_->byte_code.GetU8(pc_);
			pc_ += 1;
			stack_frame_.Push(const_pool().Get(const_idx));
			break;
		}
		case OpcodeType::kCLoadW: {
			auto const_idx = cur_func_->byte_code.GetU16(pc_);
			pc_ += 2;
			stack_frame_.Push(const_pool().Get(const_idx));
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
		case OpcodeType::kInvokeStatic: {
			auto var_idx = cur_func_->byte_code.GetU16(pc_);
			pc_ += 2;

			auto& func = GetVar(var_idx);
			auto par_count = stack_frame_.Pop().u64();

			switch (func.type()) {
			case ValueType::kFunctionBody: {
				auto call_func = func.function_body();

				std::cout << call_func->Disassembly();

				if (par_count < call_func->par_count) {
					throw VmException("Wrong number of parameters passed when calling the function");
				}

				auto save_func = cur_func_;
				auto save_pc = pc_;
				auto save_offset = stack_frame_.offset();
				
				// 栈布局：
				// 返回信息
				// 局部变量
				// 参数    <- offset
				// 原始栈

				// 切换环境和栈帧
				cur_func_ = call_func;
				pc_ = 0;
				assert(context_->runtime().stack().Size() >= cur_func_->par_count);
				stack_frame_.set_offset(context_->runtime().stack().Size() - cur_func_->par_count);

				// 参数已经入栈了，再分配局部变量部分
				assert(cur_func_->var_count >= cur_func_->par_count);
				context_->runtime().stack().Upgrade(cur_func_->var_count - cur_func_->par_count);

				// 保存当前环境，用于Ret返回
				stack_frame_.Push(Value(save_func));
				stack_frame_.Push(Value(save_pc));
				stack_frame_.Push(Value(save_offset));

				break;
			}
			case ValueType::kFunctionBridge: {
				stack_frame_.Push(func.function_bridge()(par_count, &stack_frame_));
				break;
			}
			default:
				throw VmException("Non callable types.");
			}
			break;
		}
		case OpcodeType::kReturn: {
			auto ret_value = stack_frame_.Pop();

			auto save_offset = stack_frame_.Pop();
			auto save_pc = stack_frame_.Pop();
			auto save_func = stack_frame_.Pop();

			// 恢复环境和栈帧
			cur_func_ = save_func.function_body();
			pc_ = save_pc.u64();
			stack_frame_.set_offset(save_offset.u64());

			// 清理参数和局部变量栈空间
			context_->runtime().stack().Reduce(cur_func_->var_count);

			// 返回位于原本位于栈帧栈顶的返回值
			stack_frame_.Push(ret_value);
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
			auto boolean = stack_frame_.Pop().boolean();
			if (boolean == false) {
				pc_ = cur_func_->byte_code.CalcPc(--pc_);
			}
			else {
				pc_ += 2;
			}
			break;
		}
		case OpcodeType::kGoto: {
			pc_ = cur_func_->byte_code.CalcPc(--pc_);
			break;
		}
		default:
			throw VmException("Unknown instruction");
		}
	} while (pc_ >= 0 && pc_ < cur_func_->byte_code.Size());
}

} // namespace mjs