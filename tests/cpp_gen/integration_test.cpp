/**
 * @file integration_test.cpp
 * @brief C++代码生成器集成测试
 */

#include <gtest/gtest.h>

#include "src/compiler/lexer.h"
#include "src/compiler/parser.h"

#include "cpp_gen/cpp_code_generator.h"

using namespace mjs::compiler;
using namespace mjs::compiler::cpp_gen;

TEST(CppGenIntegrationTest, SimpleExpression) {
    std::string js_code = "42;";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含命名空间
    EXPECT_NE(cpp_code.find("namespace"), std::string::npos);
}

TEST(CppGenIntegrationTest, BinaryExpression) {
    std::string js_code = "10 + 20;";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
}

TEST(CppGenIntegrationTest, ConfigOptions) {
    std::string js_code = "x = 42;";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    config.namespace_name = "test_namespace";
    config.indent_size = 2;

    CppCodeGenerator generator(config);
    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.find("test_namespace") == std::string::npos);
}

TEST(CppGenIntegrationTest, FunctionDeclaration) {
    std::string js_code = "function add(a, b) { return a + b; }";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含函数名
    EXPECT_NE(cpp_code.find("add"), std::string::npos);
}

TEST(CppGenIntegrationTest, VariableDeclaration) {
    std::string js_code = "let x = 42; let y = 3.14; let name = \"test\";";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含变量名
    EXPECT_NE(cpp_code.find("x"), std::string::npos);
    EXPECT_NE(cpp_code.find("y"), std::string::npos);
    EXPECT_NE(cpp_code.find("name"), std::string::npos);
}

TEST(CppGenIntegrationTest, ArrayLiteral) {
    std::string js_code = "let arr = [1, 2, 3];";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含数组初始化
    EXPECT_NE(cpp_code.find("arr"), std::string::npos);
}

TEST(CppGenIntegrationTest, ObjectLiteral) {
    std::string js_code = "let obj = {name: \"test\", value: 42};";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    config.enable_type_inference = true;
    config.wrap_global_code = true;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含结构体定义
    EXPECT_NE(cpp_code.find("struct"), std::string::npos);
    // 验证包含结构体成员
    EXPECT_NE(cpp_code.find("std::string name;"), std::string::npos);
    EXPECT_NE(cpp_code.find("int64_t value;"), std::string::npos);
    // 验证使用智能指针（指针语义）
    EXPECT_NE(cpp_code.find("std::shared_ptr"), std::string::npos);
    // 验证继承自mjs::generated::JSObject
    EXPECT_NE(cpp_code.find("mjs::generated::JSObject"), std::string::npos);
}

TEST(CppGenIntegrationTest, IfStatement) {
    std::string js_code = "if (x > 0) { return true; } else { return false; }";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含if语句
    EXPECT_NE(cpp_code.find("if"), std::string::npos);
    EXPECT_NE(cpp_code.find("else"), std::string::npos);
}

TEST(CppGenIntegrationTest, WhileStatement) {
    std::string js_code = "while (x < 10) { x++; }";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含while语句
    EXPECT_NE(cpp_code.find("while"), std::string::npos);
}

TEST(CppGenIntegrationTest, ForStatement) {
    std::string js_code = "for (let i = 0; i < 10; i++) { x += i; }";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含for语句
    EXPECT_NE(cpp_code.find("for"), std::string::npos);
}

TEST(CppGenIntegrationTest, StringEscape) {
    std::string js_code = "let s = \"Hello\\nWorld\\t!\";";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含转义字符
    EXPECT_NE(cpp_code.find("\\n"), std::string::npos);
    EXPECT_NE(cpp_code.find("\\t"), std::string::npos);
}

TEST(CppGenIntegrationTest, FunctionCall) {
    std::string js_code = "function add(a, b) { return a + b; } let result = add(10, 20);";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    EXPECT_FALSE(cpp_code.empty());
    // 验证包含函数调用
    EXPECT_NE(cpp_code.find("result"), std::string::npos);
    EXPECT_NE(cpp_code.find("add"), std::string::npos);
}

// 游戏逻辑示例测试 - 展示转译器的实际效果
TEST(CppGenIntegrationTest, GameLogicExample) {
    // 模拟宝可梦游戏伤害计算逻辑
    std::string js_code = R"(
// 计算伤害
function calculateDamage(base, multiplier, critical) {
    let damage = base * multiplier;
    if (critical) {
        damage = damage * 2;
    }
    return damage;
}

// 玩家对象
let player = {
    name: "Ash",
    level: 25,
    health: 100
};

// 计算战斗伤害
let attackPower = 50;
let defense = 20;
let isCritical = true;

let finalDamage = calculateDamage(attackPower, 1.5, isCritical);
let remainingHealth = player.health - finalDamage;

let sb = player["health"];

// 道具数组
let items = ["Potion", "Antidote", 123];

// 循环处理道具
for (let i = 0; i < items.length; i++) {
    let itemName = items[i];
}

// 条件判断
if (remainingHealth <= 0) {
    player.health = 0;
} else {
    player.health = remainingHealth;
}
)";

    Lexer lexer(js_code);
    Parser parser(&lexer);
    parser.ParseProgram();

    CppCodeGeneratorConfig config;
    config.namespace_name = "pokemon_game";
    config.wrap_global_code = true;  // 启用全局代码包装
    config.init_function_name = "initialize";
    config.enable_type_inference = true;
    CppCodeGenerator generator(config);

    std::string cpp_code = generator.Generate(parser);

    std::cout << "\n========== Generated C++ Code ==========\n";
    std::cout << cpp_code << std::endl;
    std::cout << "======================================\n\n";

    // 验证生成的代码包含关键元素
    EXPECT_FALSE(cpp_code.empty());

    // 验证命名空间
    EXPECT_NE(cpp_code.find("pokemon_game"), std::string::npos);

    // 验证函数生成
    EXPECT_NE(cpp_code.find("calculateDamage"), std::string::npos);

    // 验证初始化函数
    EXPECT_NE(cpp_code.find("void initialize()"), std::string::npos);

    // 验证变量声明
    EXPECT_NE(cpp_code.find("damage"), std::string::npos);
    EXPECT_NE(cpp_code.find("player"), std::string::npos);

    // 验证对象结构体生成（优化后的特性）
    EXPECT_NE(cpp_code.find("struct"), std::string::npos);
    EXPECT_NE(cpp_code.find("std::string name;"), std::string::npos);
    EXPECT_NE(cpp_code.find("int64_t level;"), std::string::npos);
    EXPECT_NE(cpp_code.find("int64_t health;"), std::string::npos);

    // 验证指针语义
    EXPECT_NE(cpp_code.find("std::shared_ptr"), std::string::npos);
    EXPECT_NE(cpp_code.find("std::make_shared"), std::string::npos);

    // 验证继承自mjs::generated::JSObject
    EXPECT_NE(cpp_code.find("mjs::generated::JSObject"), std::string::npos);

    // 验证数组
    EXPECT_NE(cpp_code.find("items"), std::string::npos);
    EXPECT_NE(cpp_code.find("std::vector"), std::string::npos);

    // 验证控制流
    EXPECT_NE(cpp_code.find("if"), std::string::npos);
    EXPECT_NE(cpp_code.find("for"), std::string::npos);

    // 验证字符串字面量
    EXPECT_NE(cpp_code.find("Ash"), std::string::npos);
    EXPECT_NE(cpp_code.find("Potion"), std::string::npos);

    // 验证Context参数
    EXPECT_NE(cpp_code.find("mjs::Context* context"), std::string::npos);
}
