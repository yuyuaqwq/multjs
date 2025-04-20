#pragma once

#include <mjs/object/function_object.h>

namespace mjs {

class ModuleObject : public FunctionObject {
public:
    ModuleObject(Context* context, FunctionDef* function_def)
        : FunctionObject(context, function_def) {}

    virtual void ForEachChild(intrusive_list<Object>* list, void(*callback)(intrusive_list<Object>* list, const Value& child)) override {
        Object::ForEachChild(list, callback);
        for (auto& val : export_) {
            callback(list, val.first);
            callback(list, val.second);
        }
    }

    virtual Value* GetProperty(Context* context, const Value& key) override {
        auto prop = FunctionObject::GetProperty(context, key);
        if (prop) {
            return prop;
        }
        auto iter = export_.find(key);
        if (iter == export_.end()) {
            return nullptr;
        }
        return &iter->second;
    }
    
private:
    // value: upvalue

    // upvalue÷∏œÚ¡Àclosure_value_arr_
    PropertyMap export_;
};

} // namespace mjs