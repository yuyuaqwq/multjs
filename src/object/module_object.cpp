#include <mjs/object/module_object.h>

namespace mjs {

ModuleObject::ModuleObject(Context* context, ModuleDef* module_def)
    : FunctionObject(context, module_def)
{
    property_map_ = new PropertyMap(context);
    module_env_.export_vars().resize(module_def->export_var_defs().size());
    
}

void ModuleObject::SetProperty(Context* context, ConstIndex key, Value&& value) {
    auto iter = property_map_->find(key);
    if (iter == property_map_->end()) {
        // todo: �����ڵ�����
        // throw;

        property_map_->set(context, key, std::move(value));
        return;
    }
    iter->second.export_var().set_value(std::move(value));
}

bool ModuleObject::GetProperty(Context* context, ConstIndex key, Value* value) {
    auto success = FunctionObject::GetProperty(context, key, value);
    if (success && value->IsExportVar()) {
        *value = value->export_var().value();
    }
    return success;
}

} // namespace mjs