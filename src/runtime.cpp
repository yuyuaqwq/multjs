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
public:
    ConsoleObject(mjs::Runtime* rt) 
        : Object(rt, mjs::ClassId::kObject)
    {
        auto log_const_index = rt->const_pool().insert(mjs::Value("log"));
        SetProperty(nullptr, log_const_index, mjs::Value([](mjs::Context* context, uint32_t par_count, const mjs::StackFrame& stack) -> mjs::Value {
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
};

Runtime::Runtime() 
	: shape_manager_(nullptr)
	, global_this_(new Object(this, ClassId::kObject))
	, class_def_table_(this)
{
	auto const_idx = const_pool_.insert(Value("globalThis"));
	auto globalThis = global_this_;
	global_this_.object().SetProperty(nullptr, const_idx, std::move(globalThis));

	const_idx = const_pool_.insert(Value("console"));
	auto console_object = new ConsoleObject(this);
	global_this_.object().SetProperty(nullptr, const_idx, Value(console_object));

}

Runtime::~Runtime() {
	const_pool_.clear();
	global_this_ = Value();
	class_def_table_.Clear();
	module_manager_.ClearModuleCache();
	gc_manager_.GC(nullptr);
}

} // namespace mjs