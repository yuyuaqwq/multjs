# 全局代码包装功能说明

## 问题背景

在JavaScript中，可以在全局作用域直接编写可执行代码。但在C++中，命名空间作用域不能包含可执行语句，只能包含声明。这导致从JavaScript转译到C++时，直接生成的代码会出现编译错误。

### 示例问题

JavaScript代码：
```javascript
function add(a, b) {
    return a + b;
}

let result = add(10, 20);
```

直接转译到C++会导致编译错误：
```cpp
namespace mjs_generated {
    auto add(auto a, auto b) {
        return a + b;
    }

    auto result = add(10, 20);  // 错误：命名空间作用域不能有可执行语句
}
```

## 解决方案

C++代码生成器现在支持将全局代码包装到初始化函数中。函数声明仍然在命名空间作用域，但可执行语句会被包装到指定的初始化函数内。

### 使用方法

#### 1. 通过命令行工具

```bash
# 启用全局代码包装
js2cpp input.js --wrap-global -o output.cpp

# 自定义初始化函数名称
js2cpp input.js --wrap-global --init-name setup -o output.cpp

# 完整示例
js2cpp game_logic.js --namespace pokemon_game --wrap-global --init-name initialize -o game_logic.cpp
```

#### 2. 通过API

```cpp
#include "cpp_gen/cpp_code_generator.h"

// 配置代码生成器
mjs::compiler::cpp_gen::CppCodeGeneratorConfig config;
config.namespace_name = "pokemon_game";
config.wrap_global_code = true;           // 启用全局代码包装
config.init_function_name = "initialize"; // 设置初始化函数名

mjs::compiler::cpp_gen::CppCodeGenerator generator(config);
std::string cpp_code = generator.Generate(parser);
```

## 转译效果

### 输入 (JavaScript)

```javascript
function calculateDamage(base, multiplier, critical) {
    let damage = base * multiplier;
    if (critical) {
        damage = damage * 2;
    }
    return damage;
}

let player = { name: "Ash", level: 25, health: 100 };
let attackPower = 50;
let isCritical = true;
let finalDamage = calculateDamage(attackPower, 1.5, isCritical);
```

### 输出 (C++)

```cpp
namespace pokemon_game {
    // 函数声明在命名空间作用域
    auto calculateDamage(auto base, auto multiplier, auto critical) {
        auto damage = (base * multiplier);
        if (critical) {
            damage = (damage * 2);
        }
        return damage;
    }

    // 全局可执行代码包装在初始化函数中
    void initialize() {
        auto player = JSObject({{"name", "Ash"}, {"level", 25}, {"health", 100}});
        int64_t attackPower = 50;
        bool isCritical = true;
        auto finalDamage = calculateDamage(attackPower, 1.5, isCritical);
    }
}
```

## 使用生成的代码

生成的C++代码可以这样使用：

```cpp
#include "game_logic.cpp"

int main() {
    // 调用初始化函数执行全局代码
    pokemon_game::initialize();

    // 使用定义的函数
    auto damage = pokemon_game::calculateDamage(100, 1.5, true);

    return 0;
}
```

## 配置选项

- `wrap_global_code`: 是否启用全局代码包装 (默认: false)
- `init_function_name`: 初始化函数名称 (默认: "initialize")

## 注意事项

1. 只有启用 `wrap_global_code` 后，全局代码才会被包装
2. 函数声明始终在命名空间作用域，不会被包装
3. 初始化函数会自动处理变量声明、函数调用、循环、条件语句等
