/**
 * @file jit_stubs.cpp
 * @brief JIT辅助函数实现
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include <mjs/jit/jit_stubs.h>

#ifdef ENABLE_JIT

#include <mjs/context.h>
#include <mjs/stack_frame.h>
#include <mjs/vm.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include <mjs/value/object/function_object.h>
#include <mjs/value/object/constructor_object.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>

namespace mjs::jit {

void JitStubs::LoadConst(Context* context, StackFrame* stack_frame, ConstIndex const_idx) {
	context->vm().LoadConst(stack_frame, const_idx);
}

void JitStubs::LoadVar(Context* context, StackFrame* stack_frame, VarIndex var_idx) {
	auto& var = context->vm().GetVar(*stack_frame, var_idx);
	stack_frame->push(var);
}

void JitStubs::StoreVar(Context* context, StackFrame* stack_frame, VarIndex var_idx) {
	auto val = stack_frame->get(-1);
	context->vm().SetVar(stack_frame, var_idx, std::move(val));
}

void JitStubs::Pop(StackFrame* stack_frame) {
	stack_frame->pop();
}

void JitStubs::LoadGlobal(Context* context, StackFrame* stack_frame, ConstIndex const_idx) {
	Value value;
	auto success = context->runtime().global_this().object().GetProperty(context, const_idx, &value);
	if (!success) {
		value = ReferenceError::Throw(context, "Failed to retrieve properties from global this: {}.",
			context->GetConstValue(const_idx).ToString(context).string_view());
	}
	stack_frame->push(std::move(value));
}

void JitStubs::LoadProperty(Context* context, StackFrame* stack_frame, ConstIndex const_idx) {
	auto& obj_val = stack_frame->get(-1);
	bool success = false;
	if (obj_val.IsObject()) {
		auto& obj = obj_val.object();
		success = obj.GetProperty(context, const_idx, &obj_val);
	} else {
		success = obj_val.ToObject().object().GetProperty(context, const_idx, &obj_val);
	}
	if (!success) {
		obj_val = Value();
	}
}

void JitStubs::StoreProperty(Context* context, StackFrame* stack_frame, ConstIndex const_idx) {
	auto obj_val = stack_frame->pop();
	auto val = stack_frame->get(-1);
	if (obj_val.IsObject()) {
		auto& obj = obj_val.object();
		obj.SetProperty(context, const_idx, std::move(val));
	} else {
		TypeError::Throw(context, "Not an object: {}", obj_val.TypeToString(obj_val.type()));
	}
}

void JitStubs::Add(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.Add(context, a);
}

void JitStubs::Sub(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.Subtract(context, a);
}

void JitStubs::Mul(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.Multiply(context, a);
}

void JitStubs::Div(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.Divide(context, a);
}

void JitStubs::Mod(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.Modulo(context, a);
}

void JitStubs::Neg(Context* context, StackFrame* stack_frame) {
	auto& a = stack_frame->get(-1);
	a = a.Negate(context);
}

void JitStubs::Inc(Context* context, StackFrame* stack_frame) {
	auto& arg = stack_frame->get(-1);
	arg = arg.Increment(context);
}

void JitStubs::Eq(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.EqualTo(context, a);
}

void JitStubs::Ne(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.NotEqualTo(context, a);
}

void JitStubs::Lt(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.LessThan(context, a);
}

void JitStubs::Le(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.LessThanOrEqual(context, a);
}

void JitStubs::Gt(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.GreaterThan(context, a);
}

void JitStubs::Ge(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.GreaterThanOrEqual(context, a);
}

void JitStubs::Typeof(Context* context, StackFrame* stack_frame) {
	auto& a = stack_frame->get(-1);
	a = a.Typeof(context);
}

void JitStubs::ToString(Context* context, StackFrame* stack_frame) {
	auto& a = stack_frame->get(-1);
	a = a.ToString(context);
}

void JitStubs::FunctionCall(Context* context, StackFrame* stack_frame) {
	// 从栈上获取参数
	auto this_val = stack_frame->pop();
	auto func_val = stack_frame->pop();
	auto param_count = static_cast<uint32_t>(stack_frame->pop().u64());

	// 调用VM::CallInternal
	context->vm().CallInternal(stack_frame, std::move(func_val), std::move(this_val), param_count);
}

void JitStubs::GetThis(StackFrame* stack_frame) {
	stack_frame->push(stack_frame->this_val());
}

void JitStubs::GetOuterThis(StackFrame* stack_frame) {
	auto& lexical_this = stack_frame->function_val().function().closure_env().lexical_this();
	stack_frame->push(lexical_this);
}

void JitStubs::Closure(Context* context, StackFrame* stack_frame, ConstIndex const_idx) {
	auto value = context->GetConstValue(const_idx);
	context->vm().Closure(*stack_frame, &value);
	stack_frame->push(std::move(value));
}

void JitStubs::New(Context* context, StackFrame* stack_frame) {
	auto func_val = stack_frame->pop();
	auto param_count = static_cast<uint32_t>(stack_frame->pop().u64());

	if (func_val.type() == ValueType::kConstructorObject) {
		auto target_class_id = func_val.constructor().target_class_id();
		auto& target_class_def = context->runtime().class_def_table()[target_class_id];
		auto obj = target_class_def.NewConstructor(context, param_count, *stack_frame);
		stack_frame->push(std::move(obj));
		return;
	}

	if (func_val.IsFunctionObject()) {
		// 用户定义的构造函数 - 暂不支持，回退到VM
		// TODO: 实现完整的new逻辑
		auto& func = func_val.function();

		// 创建对象
		Value obj_val;
		{
			GCHandleScope<1> scope(context);
			auto obj = scope.New<Object>();
			obj_val = obj.ToValue();

			Value prototype_val;
			if (!func.GetProperty(context, ConstIndexEmbedded::kPrototype, &prototype_val) || !prototype_val.IsObject()) {
				auto& object_class_def = context->runtime().class_def_table()[ClassId::kObject];
				prototype_val = object_class_def.prototype();
			}
			obj_val.object().SetPrototype(context, prototype_val);
		}

		auto new_stack_frame = StackFrame(stack_frame);
		new_stack_frame.set_bottom(new_stack_frame.bottom() - param_count);
		new_stack_frame.set_this_val(Value(obj_val));
		context->vm().CallInternal(&new_stack_frame, func_val, obj_val, param_count);

		auto& ret = new_stack_frame.get(-1);
		if (ret.type() == ValueType::kUndefined || !ret.IsObject()) {
			stack_frame->push(obj_val);
		} else {
			stack_frame->push(ret);
		}
		return;
	}

	TypeError::Throw(context, "Not a type that can be instantiated: '{}'.",
		Value::TypeToString(func_val.type()));
}

void JitStubs::IndexedLoad(Context* context, StackFrame* stack_frame) {
	auto idx_val = stack_frame->pop();
	auto& obj_val = stack_frame->get(-1);
	auto& obj = obj_val.object();
	auto success = obj.GetComputedProperty(context, idx_val, &obj_val);
	if (!success) {
		obj_val = Value();
	}
}

void JitStubs::IndexedStore(Context* context, StackFrame* stack_frame) {
	auto idx_val = stack_frame->pop();
	auto obj_val = stack_frame->pop();
	auto& obj = obj_val.object();
	auto val = stack_frame->get(-1);
	obj.SetComputedProperty(context, idx_val, std::move(val));
}

bool JitStubs::IsFalsy(const Value* value) {
	// JavaScript falsy值：
	// - undefined
	// - null
	// - false
	// - 0 (包括+0, -0)
	// - NaN
	// - "" (空字符串)
	switch (value->type()) {
		case ValueType::kUndefined:
		case ValueType::kNull:
			return true;
		case ValueType::kBoolean:
			return !value->boolean();
		case ValueType::kFloat64:
		case ValueType::kInt64: {
			// 检查是否为0或NaN
			auto num = value->f64();
			if (num == 0.0) {
				return true;
			}
			// NaN != NaN 是IEEE 754标准的特性
			if (num != num) {
				return true;
			}
			return false;
		}
		case ValueType::kString: {
			auto str = value->string().data();
			return str[0] == '\0';
		}
		default:
			// 对象、函数等都是truthy
			return false;
	}
}

// ========== 位运算指令 ==========

void JitStubs::Shl(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.LeftShift(context, a);
}

void JitStubs::Shr(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.RightShift(context, a);
}

void JitStubs::UShr(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.UnsignedRightShift(context, a);
}

void JitStubs::BitAnd(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.BitwiseAnd(context, a);
}

void JitStubs::BitOr(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.BitwiseOr(context, a);
}

void JitStubs::BitXor(Context* context, StackFrame* stack_frame) {
	auto a = stack_frame->pop();
	auto& b = stack_frame->get(-1);
	b = b.BitwiseXor(context, a);
}

void JitStubs::BitNot(Context* context, StackFrame* stack_frame) {
	auto& a = stack_frame->get(-1);
	a = a.BitwiseNot(context);
}

// ========== 逻辑运算指令 ==========

void JitStubs::LogicalAnd(Context* context, StackFrame* stack_frame) {
	// 逻辑与：如果左操作数是falsy，返回左操作数；否则返回右操作数
	auto& rhs = stack_frame->get(-1);
	auto lhs = stack_frame->pop();

	if (IsFalsy(&lhs)) {
		// 返回左操作数
		rhs = std::move(lhs);
	}
	// 否则rhs已经保存了右操作数，不需要修改
}

void JitStubs::LogicalOr(Context* context, StackFrame* stack_frame) {
	// 逻辑或：如果左操作数是truthy，返回左操作数；否则返回右操作数
	auto& rhs = stack_frame->get(-1);
	auto lhs = stack_frame->pop();

	if (!IsFalsy(&lhs)) {
		// 返回左操作数
		rhs = std::move(lhs);
	}
	// 否则rhs已经保存了右操作数，不需要修改
}

void JitStubs::NullishCoalescing(Context* context, StackFrame* stack_frame) {
	// 空值合并：只有当左操作数是null或undefined时才返回右操作数
	auto& rhs = stack_frame->get(-1);
	auto lhs = stack_frame->pop();

	// 只检查null和undefined，不包括其他falsy值
	if (lhs.type() != ValueType::kNull && lhs.type() != ValueType::kUndefined) {
		// 返回左操作数
		rhs = std::move(lhs);
	}
	// 否则rhs已经保存了右操作数，不需要修改
}

Value* JitStubs::Get(StackFrame* stack_frame, int64_t offset) {
	return &stack_frame->get(offset);
}

void JitStubs::PushValue(StackFrame* stack_frame, Value* value_ptr) {
	stack_frame->push(*value_ptr);
}

void JitStubs::SwapStub(StackFrame* stack_frame) {
	auto val1 = stack_frame->pop();
	auto val2 = stack_frame->pop();
	stack_frame->push(std::move(val1));
	stack_frame->push(std::move(val2));
}

} // namespace mjs::jit

#endif // ENABLE_JIT
