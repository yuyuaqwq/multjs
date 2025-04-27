#include <mjs/class_def/symbol_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

SymbolClassDef::SymbolClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kSymbol, "Symbol")
{
	auto iter = property_map_.emplace(runtime, "iterator", Value());
	iter.first->second = Value(iter.first->first);

	static_property_map_.emplace(runtime, "for", Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return Value("Parameter count mismatch.").SetException();
		}

		auto& par = stack.get(-1);
		if (!par.IsString()) {
			return Value("The parameter must be a string.").SetException();
		}

		auto& symbol_class_def = context->runtime().class_def_table().at<SymbolClassDef>(ClassId::kSymbol);
		return symbol_class_def.For(context, par);
	}));
}

Value SymbolClassDef::For(Context* context, Value name) {
	if (!name.const_index().is_invalid()) {
		context->symbol_table().set(context, name.const_index(), Value(Symbol(context), name.const_index()));
		return context->symbol_table()[name.const_index()];
	}
	else {
		auto const_index = context->const_pool().insert(name);
		auto iter = context->symbol_table().emplace(context, std::move(name.string()), Value(Symbol(context), const_index));
		return iter.first->second;
	}
}

} // namespace mjs