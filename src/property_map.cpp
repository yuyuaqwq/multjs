#include <mjs/property_map.h>

#include <mjs/runtime.h>
#include <mjs/context.h>

namespace mjs {

PropertyMap::PropertyMap(Context* context)
    : Base(0, ConstIndexHasher(), ConstIndexHashKeyEqual()),
    context_(context), 
    runtime_(&context->runtime()) {}

PropertyMap::PropertyMap(Runtime* runtime)
    : Base(0, ConstIndexHasher(), ConstIndexHashKeyEqual()),
    runtime_(runtime),
    context_(nullptr) {}

PropertyMap::~PropertyMap() {
	if (!context_) {
		return;
	}
	for (auto& pair : *this) {
		if (pair.first.is_local_index()) {
			context_->const_pool().DereferenceConst(pair.first);
		}
	}
}


ConstIndex PropertyMap::InsertConst(Runtime* runtime, std::string&& name) {
	auto idx = runtime->const_pool().insert(Value(std::move(name)));
	return idx;
}

ConstIndex PropertyMap::InsertConst(Context* context, std::string&& name) {
	auto idx = context->const_pool().insert(Value(std::move(name)));
	return idx;
}

void PropertyMap::ReferenceConst(Context* context, ConstIndex index) {
	context->const_pool().ReferenceConst(index);
}


inline const Value& GetPoolValue(PropertyMap* property_map, ConstIndex const& key) {
    if (key.is_global_index()) {
        // Perform your context-dependent hashing
        // return ...;
        auto& val = property_map->runtime().const_pool().at(key);
        return val;
    }
    else {
        assert(key.is_local_index());
        auto key_ = key;
        key_.from_local_index();
        auto& val = property_map->context().const_pool().at(key);
        return val;
    }
}

auto ConstIndexHasher::operator()(const ConstIndex& key) const noexcept -> uint64_t {
    PropertyMap* property_map = reinterpret_cast<PropertyMap*>(reinterpret_cast<char*>(const_cast<ConstIndexHasher*>(this)) - offsetof(PropertyMap, m_hash));
    auto& val = GetPoolValue(property_map, key);
    return val.hash();
}

bool ConstIndexHashKeyEqual::operator()(const ConstIndex& lhs, const ConstIndex& rhs) const {
    if (lhs.is_same_pool(rhs)) {
        return lhs == rhs;
    }
    PropertyMap* property_map = reinterpret_cast<PropertyMap*>(reinterpret_cast<char*>(const_cast<ConstIndexHashKeyEqual*>(this)) - offsetof(PropertyMap, m_equal));
    auto& lval = GetPoolValue(property_map, lhs);
    auto& rval = GetPoolValue(property_map, rhs);
    return lval.Comparer(rval) == 0;
}


} // namespace mjs