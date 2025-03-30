#pragma once

#include <map>
#include <string>

namespace mjs {

enum class ClassType {
    kNumber,
    kString,
    kArray,
    kPromise,
};

enum class ObjectInternalMethods {
	kGetPrototypeOf = 1 << 0,
	kSetPrototypeOf = 1 << 1,
	kIsExtensible = 1 << 2,
	kPreventExtensions = 1 << 3,
	kGetOwnProperty = 1 << 4,
	kDefineOwnProperty = 1 << 5,
	kHasProperty = 1 << 6,
	kGet = 1 << 7,
	kSet = 1 << 8,
	kDelete = 1 << 9,
	kOwnPropertyKeys = 1 << 10,
};

enum class FunctionInternalMethods {
	kCall = 1 << 1,
};


struct ClassDef {
	std::string name;


};

extern std::map<std::string, ClassType> g_classes;

} // namespace mjs