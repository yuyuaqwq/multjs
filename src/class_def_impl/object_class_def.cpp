#include <mjs/class_def_impl/object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/shape_property.h>
#include <mjs/object.h>
#include <mjs/error.h>

namespace mjs {

ObjectClassDef::ObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kObject, "Object")
{
	// Object.prototype 是一个特殊的对象,它没有原型(原型为null)
	prototype_.object().SetPrototype(runtime, ConstIndexEmbedded::kProto, Value(nullptr));

	// 注册 Object.freeze 静态方法到构造函数对象
	constructor_.object().SetProperty(runtime, ConstIndexEmbedded::kFreeze, Value(Freeze));

	// 注册 Object.seal 静态方法到构造函数对象
	constructor_.object().SetProperty(runtime, ConstIndexEmbedded::kSeal, Value(Seal));

	// 注册 Object.preventExtensions 静态方法到构造函数对象
	constructor_.object().SetProperty(runtime, ConstIndexEmbedded::kPreventExtensions, Value(PreventExtensions));

	// 注册 Object.defineProperty 静态方法到构造函数对象
	constructor_.object().SetProperty(runtime, ConstIndexEmbedded::kDefineProperty, Value(DefineProperty));
}

Value ObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const {
	return Value(Object::New(context));
}

Value ObjectClassDef::LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack) {
	auto obj = Object::New(context);
	for (int32_t i = 0; i < par_count; i += 2) {
		auto key_const_index = stack.get(i).const_index();
		if (key_const_index == kConstIndexInvalid) {
			return TypeError::Throw(context, "Object property key must be a valid string");
		}

		obj->SetProperty(context, key_const_index, std::move(stack.get(i + 1)));
	}
	return Value(obj);
}

Value ObjectClassDef::SetProperty(Context* context, uint32_t par_count, const StackFrame& stack) {
	// 参数: obj, key, value
	if (par_count != 3) {
		return TypeError::Throw(context, "Object.setProperty requires exactly 3 arguments");
	}
	auto* obj = &stack.get(0).object();
	auto key_const_index = stack.get(1).const_index();
	if (key_const_index == kConstIndexInvalid) {
		return TypeError::Throw(context, "Object property key must be a valid string");
	}
	obj->SetProperty(context, key_const_index, std::move(stack.get(2)));
	return Value(obj);
}

Value ObjectClassDef::DefineProperty(Context* context, uint32_t par_count, const StackFrame& stack) {
	// 支持两种调用方式:
	// 1. 标准 JavaScript 语法: Object.defineProperty(obj, prop, descriptor) - 3个参数
	// 2. 内部 getter/setter 语法: Object.defineProperty(obj, key, accessor, kind) - 4个参数
	if (!stack.get(0).IsObject()) {
		return TypeError::Throw(context, "Object.defineProperty requires an object as first argument");
	}

	auto* obj = &stack.get(0).object();
	auto key_const_index = stack.get(1).const_index();
	if (key_const_index == kConstIndexInvalid) {
		return TypeError::Throw(context, "Object property key must be a valid string");
	}

	if (par_count == 3) {
		// 标准 JavaScript 语法: Object.defineProperty(obj, prop, descriptor)
		// descriptor 是一个对象，包含 value, writable, enumerable, configurable, get, set 等属性
		auto& descriptor = stack.get(2);

		if (descriptor.IsObject()) {
			auto& desc_obj = descriptor.object();

			// 尝试获取 value 属性
			auto value_const_index = context->runtime().global_const_pool().Insert(Value("value"));
			Value value_value;
			if (desc_obj.GetProperty(context, value_const_index, &value_value)) {
				// 如果有 value 属性，设置属性值
				obj->SetProperty(context, key_const_index, std::move(value_value));
			}
		}
	} else if (par_count == 4) {
		// 内部 getter/setter 语法: Object.defineProperty(obj, key, accessor, kind)
		auto& accessor = stack.get(2).ToFunctionDef();
		auto kind = static_cast<int>(stack.get(3).i64());

		// 设置正确的 accessor 标志
		uint32_t flags = (kind == 1) ? ShapeProperty::kIsGetter : ShapeProperty::kIsSetter;

		// 使用 SetPropertyWithFlags 存储带标志的属性
		obj->SetPropertyWithFlags(context, key_const_index, Value(&accessor), flags);
	} else {
		// 不支持的参数数量
		return TypeError::Throw(context, "Object.defineProperty requires 3 or 4 arguments");
	}

	return Value(obj);
}

Value ObjectClassDef::Freeze(Context* context, uint32_t par_count, const StackFrame& stack) {
	// TODO: 当前的实现存在缺陷，没有设置属性的writable等描述符

	// 参数: obj
	if (par_count < 1 || !stack.get(0).IsObject()) {
		return TypeError::Throw(context, "Object.freeze requires an object argument");
	}

	auto* obj = &stack.get(0).object();

	// 冻结对象
	obj->Freeze();

	// 返回被冻结的对象
	return Value(obj);
}

Value ObjectClassDef::Seal(Context* context, uint32_t par_count, const StackFrame& stack) {
	// TODO: 当前的实现存在缺陷，没有设置属性的writable等描述符

	if (par_count < 1 || !stack.get(0).IsObject()) {
		return TypeError::Throw(context, "Object.seal requires an object argument");
	}

	// 参数: obj
	auto* obj = &stack.get(0).object();

	// 密封对象
	obj->Seal();

	// 返回被密封的对象
	return Value(obj);
}

Value ObjectClassDef::PreventExtensions(Context* context, uint32_t par_count, const StackFrame& stack) {
	// TODO: 当前的实现存在缺陷，没有设置属性的writable等描述符

	// 参数: obj
	if (par_count != 1 || !stack.get(0).IsObject()) {
		return TypeError::Throw(context, "Object.preventExtensions requires an object argument");
	}
	auto* obj = &stack.get(0).object();

	// 阻止对象扩展
	obj->PreventExtensions();

	// 返回被阻止扩展的对象
	return Value(obj);
}

} // namespace mjs