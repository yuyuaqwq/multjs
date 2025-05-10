#include <mjs/class_def_impl/symbol_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>

namespace mjs {

SymbolClassDef::SymbolClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kSymbol, "Symbol")
{
	auto iter = property_map_.insert(runtime, "iterator", Value());
	iter.first->second = Value(iter.first->first);

	static_property_map_.insert(runtime, "for", Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return Value("Parameter count mismatch.").SetException();
		}

		auto& par = stack.get(-1);
		if (!par.IsString()) {
			return Value("The parameter must be a string.").SetException();
		}

		auto& symbol_class_def = context->runtime().class_def_table()[ClassId::kSymbol].get<SymbolClassDef>();
		return symbol_class_def.For(context, par);
	}));
}

Value SymbolClassDef::For(Context* context, Value name) {
	ConstIndex name_const_index = name.const_index();
	if (name_const_index.is_invalid()) {
		name_const_index = context->const_pool().insert(name);
	}

	auto iter = context->symbol_table().find(name_const_index);
	if (iter != context->symbol_table().end()) {
		return iter->second;
	}

	auto symbol_const_index = context->const_pool().insert(Value(new Symbol()));
	auto symbol = context->const_pool()[symbol_const_index];

	return context->symbol_table().set(context, name_const_index, std::move(symbol))->second;

}

} // namespace mjs