#include <unordered_map>

#include <mjs/value.h>

namespace mjs {

class PropertyMap : public std::unordered_map<Value, Value, ValueHash> {
public:
	void NewMethod(Value&& name, Value&& func) {
		emplace(std::move(name), std::move(func));
	}
};

} // namespace mjs