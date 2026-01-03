/**
 * @file test_computed_access.cpp
 * @brief 测试计算属性访问功能
 */

#include <iostream>
#include <string>
#include "cpp_gen/mjs_runtime.h"

using namespace mjs::generated;

// 模拟生成的结构体
struct TestPlayer : public DynamicObject {
    std::string name;
    int64_t level;
    int64_t health;

    TestPlayer() = default;

    // 重载 [] 运算符
    template<typename T>
    auto& operator[](const T& key) {
        if constexpr (std::is_same_v<T, std::string>) {
            if (key == "name") { return name; }
            else if (key == "level") { return level; }
            else if (key == "health") { return health; }
            else { return DynamicObject::operator[](key); }
        } else {
            return DynamicObject::operator[](key);
        }
    }

    template<typename T>
    auto operator[](const T& key) const {
        if constexpr (std::is_same_v<T, std::string>) {
            if (key == "name") { return name; }
            else if (key == "level") { return level; }
            else if (key == "health") { return health; }
            else { return DynamicObject::operator[](key); }
        } else {
            return DynamicObject::operator[](key);
        }
    }
};

int main() {
    TestPlayer player;
    player.name = "Ash";
    player.level = 25;
    player.health = 100;

    std::cout << "Testing computed property access:" << std::endl;
    std::cout << "player.name = " << player.name << std::endl;
    std::cout << "player.level = " << player.level << std::endl;
    std::cout << "player.health = " << player.health << std::endl;

    // 测试计算属性访问
    std::string health_key = "health";
    auto& health_ref = player[health_key];
    std::cout << "\nplayer[\"health\"] = " << health_ref << std::endl;

    // 修改值
    player["health"] = 80;
    std::cout << "After player[\"health\"] = 80:" << std::endl;
    std::cout << "player.health = " << player.health << std::endl;

    // 测试动态属性访问(非结构体成员)
    player["dynamicProp"] = JSValue::FromString("test");
    std::cout << "\nDynamic property: player[\"dynamicProp\"] = "
              << player["dynamicProp"].AsString() << std::endl;

    // 测试访问不存在的属性
    auto unknown = player["unknown"];
    std::cout << "Unknown property returns: " << (unknown.IsNull() ? "null" : "value") << std::endl;

    return 0;
}
