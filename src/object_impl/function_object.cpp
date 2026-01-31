#include <mjs/object_impl/function_object.h>
#include <mjs/class_def_impl/function_object_class_def.h>
#include <mjs/object.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/shape.h>
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
	auto* prototype_obj = Object::New(context);

	// 在原型对象上设置 constructor 属性指向函数本身
	prototype_obj->SetProperty(context, ConstIndexEmbedded::kConstructor, Value(this));

	// 将 prototype 对象添加到函数对象上
	// 属性标志: writable, configurable (非 enumerable)
	uint32_t flags = ShapeProperty::kWritable | ShapeProperty::kConfigurable;

	auto index = shape_->Find(ConstIndexEmbedded::kPrototype);
	if (index == kPropertySlotIndexInvalid) {
		ShapeProperty prop(ConstIndexEmbedded::kPrototype);
		index = shape_->shape_manager()->AddProperty(&shape_, std::move(prop));
		AddPropertySlot(index, Value(prototype_obj), flags);
	} else {
		SetPropertyValue(index, Value(prototype_obj));
	}
}

} // namespace mjs