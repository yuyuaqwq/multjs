#include <mjs/property_map.h>

#pragma once

#include <mjs/runtime.h>

namespace mjs {

void PropertyMap::NewMethod(Runtime* runtime, std::string name, Value&& func) {
	auto idx = runtime->const_pool().insert(Value(std::move(name)));
	auto& name_val = runtime->const_pool()[idx];
	emplace(name_val, std::move(func));
}


} // namespace mjs