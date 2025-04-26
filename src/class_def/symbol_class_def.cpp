#include <mjs/class_def/symbol_class_def.h>

#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/symbol.h>

namespace mjs {

SymbolClassDef::SymbolClassDef(Runtime* runtime)
	: ClassDef(runtime, ClassId::kSymbol, "Symbol")
{
	auto iter = property_map_.emplace(runtime, "iterator", Value());
	iter.first->second = Value(new Symbol(iter.first->first));

	property_map_.emplace(runtime, "for", Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
		if (par_count < 1) {
			return Value();
		}

		auto& symbol_class_def = context->runtime().class_def_table().at<SymbolClassDef>(ClassId::kSymbol);

		auto idx = stack.get(-1).const_index();
		if (idx.is_invalid()) {
			idx = context->const_pool().insert(stack.get(-1));
		}

		return symbol_class_def.For(context, idx);
	}));
}

Value SymbolClassDef::For(Context* context, ConstIndex idx) {
	// ¸Äµô£¬´ÓcontextÖÐÕÒ
	//auto iter = property_map_.find(idx);
	//if (iter == property_map_.end()) {
	//	auto symbol = new Symbol(idx);
	//	property_map_.emplace(idx, Value(symbol));
	//}
	//return iter->second;

	return Value();
}

} // namespace mjs