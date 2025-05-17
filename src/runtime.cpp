#include <mjs/runtime.h>

#include <fstream>

#include <mjs/context.h>
#include <mjs/class_def_impl/symbol_class_def.h>
#include <mjs/class_def_impl/array_object_class_def.h>
#include <mjs/class_def_impl/object_class_def.h>
#include <mjs/class_def_impl/generator_object_class_def.h>
#include <mjs/class_def_impl/promise_object_class_def.h>

namespace mjs {

Runtime::Runtime() 
	: shape_manager_(nullptr)
	, global_this_(new Object(this, ClassId::kObject))
	, class_def_table_(this) {}

Runtime::~Runtime() {
	const_pool_.clear();
	global_this_ = Value();
	class_def_table_.Clear();
	module_manager_.ClearModuleCache();
	gc_manager_.GC(nullptr);
}

} // namespace mjs