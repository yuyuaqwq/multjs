#include <mjs/class.h>

namespace mjs {

std::map<std::string, ClassType> g_classes = {
	{ "Number", ClassType::kNumber },
	{ "String", ClassType::kString },
	{ "Array", ClassType::kNumber },
	{ "Promise", ClassType::kPromise },
};

} // namespace msj