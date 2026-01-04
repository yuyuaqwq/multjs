#include <mjs/class_def_impl/object_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/shape_property.h>
#include <mjs/object.h>

namespace mjs {

ObjectClassDef::ObjectClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kObject, "Object")
{
	// Object.prototype 是一个特殊的对象,它没有原型(原型为null)
	// 所以我们将prototype_设置为Value() (null/undefined)
	// 这符合JavaScript规范: Object.prototype.__proto__ === null
	prototype_ = Value(nullptr);
}

Value ObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const {
	return Value(Object::New(context));
}

Value ObjectClassDef::LiteralNew(Context* context, uint32_t par_count, const StackFrame& stack) {
	auto obj = Object::New(context);
	for (int32_t i = 0; i < par_count; i += 2) {
		auto key_const_index = stack.get(i).const_index();
		assert(key_const_index != kConstIndexInvalid);
		obj->SetProperty(context, key_const_index, std::move(stack.get(i + 1)));
	}
	return Value(obj);
}

Value ObjectClassDef::SetProperty(Context* context, uint32_t par_count, const StackFrame& stack) {
	// 参数: obj, key, value
	assert(par_count == 3);
	auto* obj = &stack.get(0).object();
	auto key_const_index = stack.get(1).const_index();
	assert(key_const_index != kConstIndexInvalid);
	obj->SetProperty(context, key_const_index, std::move(stack.get(2)));
	return Value(obj);
}

Value ObjectClassDef::DefineProperty(Context* context, uint32_t par_count, const StackFrame& stack) {
	// 参数: obj, key, getter/setter函数, kind(1=getter, 2=setter)
	assert(par_count == 4);
	auto* obj = &stack.get(0).object();
	auto key_const_index = stack.get(1).const_index();
	assert(key_const_index != kConstIndexInvalid);
	auto* accessor = &stack.get(2).function();
	auto kind = static_cast<int>(stack.get(3).i64());

	// 设置正确的 accessor 标志
	uint32_t flags = (kind == 1) ? ShapeProperty::kIsGetter : ShapeProperty::kIsSetter;

	// 使用 SetPropertyWithFlags 存储带标志的属性
	obj->SetPropertyWithFlags(context, key_const_index, Value(accessor), flags);

	return Value(obj);
}

} // namespace mjs