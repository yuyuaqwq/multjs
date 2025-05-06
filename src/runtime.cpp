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
	: class_def_table_(this)
	, global_this_(new Object(this)) {}

} // namespace mjs