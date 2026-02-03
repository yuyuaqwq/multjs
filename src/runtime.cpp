#include <mjs/runtime.h>

#include <fstream>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/class_def/symbol_class_def.h>
#include <mjs/class_def/array_object_class_def.h>
#include <mjs/class_def/object_class_def.h>
#include <mjs/class_def/generator_object_class_def.h>
#include <mjs/class_def/promise_object_class_def.h>

namespace mjs {

class ConsoleObject : public mjs::Object {
private:
    ConsoleObject(mjs::Context* context)
        : Object(context, mjs::ClassId::kObject)
    {
        auto log_const_index = context->runtime().global_const_pool().FindOrInsert(mjs::Value("log"));
        SetProperty(context, log_const_index, mjs::Value([](mjs::Context* context, uint32_t par_count, const mjs::StackFrame& stack) -> mjs::Value {
            for (size_t i = 0; i < par_count; i++) {
                auto val = stack.get(i);
                try {
                    std::cout << val.ToString(context).string_view();
                }
                catch (const std::exception&)
                {
                    std::cout << "unknown";
                }
            }
            printf("\n");
            return mjs::Value();
        }));
    }

public:
    static ConsoleObject* New(Context* context) {
        return new ConsoleObject(context);
    }
};

Runtime::Runtime()
	: default_context_(this)
    , global_this_()
    , class_def_table_(this)
    , module_manager_(std::make_unique<ModuleManager>())
{
    Initialize();
}

Runtime::Runtime(std::unique_ptr<ModuleManagerBase> module_manager)
    : default_context_(this)
    , global_this_()
    , class_def_table_(this)
    , module_manager_(std::move(module_manager))
{
    // 创建全局对象
    GCHandleScope<1> scope(&default_context_);
    auto global_obj = scope.New<Object>();
    global_this_ = global_obj.ToValue();

    Initialize();
}

Runtime::~Runtime() {
	global_const_pool_.Clear();
	global_this_ = Value();
    default_context_.gc_manager().RemoveRoot(&global_this_);

	class_def_table_.Clear();
	module_manager_->ClearModuleCache();
}

void Runtime::AddPropertyToGlobalThis(const char* property_key, Value&& value) {
    auto const_idx = global_const_pool_.FindOrInsert(Value(property_key));
    global_this_.object().SetProperty(&default_context_, const_idx, std::move(value));
}

void Runtime::Initialize() {
    // 创建全局对象
    GCHandleScope<1> scope(&default_context_);
    global_this_ = scope.New<Object>().ToValue();
    default_context_.gc_manager().AddRoot(&global_this_);

    global_const_pool_.Initialize();
    class_def_table_.Initialize(this);
    GlobalThisInitialize();
    ConsoleInitialize();
}

void Runtime::GlobalThisInitialize() {
    auto const_idx = global_const_pool_.FindOrInsert(Value("globalThis"));
    auto globalThis = global_this_;
    global_this_.object().SetProperty(&default_context_, const_idx, std::move(globalThis));
}

void Runtime::ConsoleInitialize() {
    auto console_object = ConsoleObject::New(&default_context_);
    AddPropertyToGlobalThis("console", Value(console_object));
}

} // namespace mjs