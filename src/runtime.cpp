#include <mjs/runtime.h>

#include <fstream>

#include <mjs/context.h>
#include <mjs/class_def_impl/symbol_class_def.h>
#include <mjs/class_def_impl/array_object_class_def.h>
#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/class_def_impl/generator_object_class_def.h>
#include <mjs/class_def_impl/promise_object_class_def.h>

namespace mjs {

class ConsoleObject : public mjs::Object {
private:
    ConsoleObject(mjs::Runtime* runtime)
        : Object(runtime, mjs::ClassId::kObject)
    {
        auto log_const_index = runtime->global_const_pool().insert(mjs::Value("log"));
        SetProperty(runtime, log_const_index, mjs::Value([](mjs::Context* context, uint32_t par_count, const mjs::StackFrame& stack) -> mjs::Value {
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
    static ConsoleObject* New(Runtime* runtime) {
        return new ConsoleObject(runtime);
    }
};

Runtime::Runtime() 
	: key_const_index_table_(&global_const_pool_)
    , shape_manager_(nullptr)
	, global_this_(Object::New(this))
	, class_def_table_(this)
    , module_manager_(std::make_unique<ModuleManager>())
{
    Initialize();
}

Runtime::Runtime(std::unique_ptr<ModuleManagerBase> module_manager)
    : key_const_index_table_(&global_const_pool_)
    , shape_manager_(nullptr)
    , global_this_(Object::New(this))
    , class_def_table_(this)
    , module_manager_(std::move(module_manager))
{
    Initialize();
}

Runtime::~Runtime() {
	global_const_pool_.clear();
	global_this_ = Value();
	class_def_table_.Clear();
	module_manager_->ClearModuleCache();
	gc_manager_.GC(nullptr);
}

void Runtime::AddPropertyToGlobalThis(const char* property_key, Value&& value) {
    auto const_idx = global_const_pool_.insert(Value(property_key));
    global_this_.object().SetProperty(this, const_idx, std::move(value));
}

void Runtime::Initialize() {
    GlobalThisInitialize();
    ConsoleInitialize();
}

void Runtime::GlobalThisInitialize() {
    auto const_idx = global_const_pool_.insert(Value("globalThis"));
    auto globalThis = global_this_;
    global_this_.object().SetProperty(this, const_idx, std::move(globalThis));
}

void Runtime::ConsoleInitialize() {
    auto console_object = ConsoleObject::New(this);
    AddPropertyToGlobalThis("console", Value(console_object));
}

} // namespace mjs