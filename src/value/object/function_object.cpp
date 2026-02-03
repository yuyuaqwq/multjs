#include <mjs/value/object/function_object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/class_def/function_object_class_def.h>
#include <mjs/value/object/object.h>
#include <mjs/shape/shape.h>
#include <mjs/const_index_embedded.h>

namespace mjs {

FunctionObject::FunctionObject(Context* context, FunctionDefBase* function_def) noexcept
	: FunctionObject(context, function_def, ClassId::kFunctionObject) {}

FunctionObject::FunctionObject(Context* context, FunctionDefBase* function_def, ClassId class_id) noexcept
	: Object(context, class_id)
	, function_def_(function_def)
{
	closure_env_.closure_var_refs().resize(function_def->closure_var_table().closure_var_defs().size());
	InitPrototypeProperty(context);
}

void FunctionObject::InitPrototypeProperty(Context* context) {
	// 箭头函数、生成器函数、异步函数不需要 prototype 属性
	if (function_def_->is_arrow() || function_def_->is_generator() || function_def_->is_async()) {
		return;
	}

	// 获取 FunctionObjectClassDef 中缓存的常量索引
	auto& function_object_class_def = context->runtime().class_def_table()[ClassId::kFunctionObject].get<FunctionObjectClassDef>();

	// 创建原型对象
	GCHandleScope<1> scope(context);
	auto prototype_obj = scope.New<Object>();

	// 在原型对象上设置 constructor 属性指向函数本身
	prototype_obj->SetProperty(context, ConstIndexEmbedded::kConstructor, Value(this));

	// 将 prototype 对象添加到函数对象上
	// 属性标志: writable, configurable (非 enumerable)
	uint32_t flags = ShapeProperty::kWritable | ShapeProperty::kConfigurable;
	auto prototype_obj_value = prototype_obj.ToValue();

	auto index = shape_->Find(ConstIndexEmbedded::kPrototype);
	if (index == kPropertySlotIndexInvalid) {
		ShapeProperty prop(ConstIndexEmbedded::kPrototype);
		index = shape_->shape_manager()->AddProperty(&shape_, std::move(prop));
		AddPropertySlot(index, std::move(prototype_obj_value), flags);
	} else {
		SetPropertyValue(index, std::move(prototype_obj_value));
	}
}

void FunctionObject::GCTraverse(Context* context, GCTraverseCallback callback) {
	// 先调用父类方法遍历属性
	Object::GCTraverse(context, callback);

	// 遍历闭包环境
	closure_env_.GCTraverse(context, callback);
}

} // namespace mjs