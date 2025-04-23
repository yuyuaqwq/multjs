#pragma once

#include <mjs/object/function_object.h>

namespace mjs {

class ModuleObject : public FunctionObject {
public:
    ModuleObject(Context* context, FunctionDef* function_def)
        : FunctionObject(context, function_def) {}

    virtual void ForEachChild(intrusive_list<Object>* list, void(*callback)(intrusive_list<Object>* list, const Value& child)) override {
        Object::ForEachChild(list, callback);
        for (auto& pair : export_map_) {
            callback(list, pair.second);
        }
    }

    virtual Value* GetProperty(Context* context, ConstIndex key) override {
        auto prop = FunctionObject::GetProperty(context, key);
        if (prop) {
            return prop;
        }
        auto iter = export_map_.find(key);
        if (iter == export_map_.end()) {
            return nullptr;
        }
        return &iter->second;
    }
    
    auto& export_map() { return export_map_; }

private:
    // value是upvalue，指向closure_value_arr_中的变量
    PropertyMap export_map_;
};

} // namespace mjs