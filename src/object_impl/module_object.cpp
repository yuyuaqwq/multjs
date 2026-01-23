#include <mjs/object_impl/module_object.h>

#include <mjs/shape.h>

namespace mjs {

ModuleObject::ModuleObject(Context* context, ModuleDef* module_def)
    : FunctionObject(context, module_def, ClassId::kModuleObject)
{
    //property_map_ = new PropertyMap(context);
    module_env_.export_vars().resize(module_def->export_var_def_table().export_var_defs().size());
}

void ModuleObject::SetProperty(Context* context, ConstIndex key, Value&& value) {
    auto index = shape_->Find(key);
    if (index == kPropertySlotIndexInvalid) {
        // todo: 可能需要异常，这里允许设置是给模块加载时初始化用的，初始化后不允许修改模块的命名空间对象
        FunctionObject::SetProperty(context, key, std::move(value));
        return;
    }
    GetPropertyValue(index).export_var().set_value(std::move(value));
}

bool ModuleObject::GetProperty(Context* context, ConstIndex key, Value* value) {
    auto success = FunctionObject::GetProperty(context, key, value);
    if (success && value->IsExportVar()) {
        *value = value->export_var().value();
    }
    return success;
}

} // namespace mjs