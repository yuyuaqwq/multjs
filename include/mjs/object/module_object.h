#pragma once

#include <mjs/object/function_object.h>

namespace mjs {

class ModuleObject : public FunctionObject {
public:
    ModuleObject(Context* context, FunctionDef* function_def)
        : FunctionObject(context, function_def)
        , export_map_(context) {}

    void GCForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) override {
        Object::GCForEachChild(context, list, callback);
        for (auto& pair : export_map_) {
            callback(context, list, pair.second);
        }
    }

    bool GetProperty(Context* context, ConstIndex key, Value* value) override {
        auto success = FunctionObject::GetProperty(context, key, value);
        if (success) { return true; }
        auto iter = export_map_.find(key);
        if (iter == export_map_.end()) {
            return false;
        }
        *value = iter->second;
        return true;
    }
    
    auto& export_map() { return export_map_; }

private:
    // value是upvalue，指向closure_value_arr_
    PropertyMap export_map_;
};

} // namespace mjs