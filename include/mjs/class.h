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

class ClassInfo {

};

extern std::map<std::string, ClassType> g_classes;

} // namespace mjs