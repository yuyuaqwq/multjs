#include <map>

namespace mjs {

// std::unordered_map<Value, Value>;
class PropertyMap : public std::map<Value, Value> {
public:
	void NewMethod(Value&& name, Value&& func) {
		emplace(std::move(name), std::move(func));
	}
};

} // namespace mjs