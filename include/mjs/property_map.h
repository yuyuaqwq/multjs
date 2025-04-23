#pragma once

#include <unordered_map>

#include <mjs/value.h>

namespace mjs {

class Runtime;
class PropertyMap : public std::unordered_map<ConstIndex, Value> {
public:
	void NewMethod(Runtime* runtime, std::string name, Value&& func);
};

} // namespace mjs