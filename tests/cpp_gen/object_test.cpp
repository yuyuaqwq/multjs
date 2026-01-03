/**
 * @file object_test.cpp
 * @brief 对象结构体生成测试
 */

#include <gtest/gtest.h>
#include <sstream>
#include "cpp_gen/cpp_code_generator.h"
#include "src/compiler/parser.h"
#include "src/compiler/lexer.h"

using namespace mjs::compiler::cpp_gen;

class ObjectGenerationTest : public ::testing::Test {
protected:
    std::string GenerateFromJS(const std::string& js_code) {
        mjs::compiler::Lexer lexer(js_code);
        mjs::compiler::Parser parser(&lexer);
        parser.ParseProgram();

        CppCodeGeneratorConfig config;
        config.enable_type_inference = true;
        config.wrap_global_code = true;

        CppCodeGenerator generator(config);
        return generator.Generate(parser);
    }
};

TEST_F(ObjectGenerationTest, GenerateSimpleObject) {
    std::string js = R"(
        let player = { name: "Ash", level: 25, health: 100 };
    )";

    std::string cpp = GenerateFromJS(js);

    // 检查是否生成了结构体定义
    EXPECT_NE(cpp.find("struct"), std::string::npos);
    EXPECT_NE(cpp.find("Struct_"), std::string::npos);

    // 检查结构体是否包含正确的成员
    EXPECT_NE(cpp.find("std::string name;"), std::string::npos);
    EXPECT_NE(cpp.find("int64_t level;"), std::string::npos);
    EXPECT_NE(cpp.find("int64_t health;"), std::string::npos);

    // 检查是否使用了结构体初始化而不是 JSObject
    EXPECT_NE(cpp.find("Struct_0"), std::string::npos);
}

TEST_F(ObjectGenerationTest, GenerateNestedObject) {
    std::string js = R"(
        let game = {
            player: { name: "Ash", level: 25 },
            enemy: { name: "Gary", level: 30 }
        };
    )";

    std::string cpp = GenerateFromJS(js);

    // 应该生成多个结构体（PlayerStruct_0 和 EnemyStruct_1，或者通用的 Struct_0 和 Struct_1）
    size_t struct_count = 0;
    size_t pos = 0;
    while ((pos = cpp.find("struct", pos)) != std::string::npos) {
        struct_count++;
        pos += 6;
    }

    EXPECT_GE(struct_count, 2); // 至少两个结构体
}

TEST_F(ObjectGenerationTest, ObjectMemberAccess) {
    std::string js = R"(
        let player = { name: "Ash", level: 25 };
        let playerName = player.name;
        let playerLevel = player.level;
    )";

    std::string cpp = GenerateFromJS(js);

    // 检查成员访问是否直接使用点运算符
    EXPECT_NE(cpp.find("player.name"), std::string::npos);
    EXPECT_NE(cpp.find("player.level"), std::string::npos);
}

TEST_F(ObjectGenerationTest, ObjectInFunction) {
    std::string js = R"(
        function createPlayer(name, level) {
            return { name: name, level: level };
        }

        let player = createPlayer("Misty", 20);
    )";

    std::string cpp = GenerateFromJS(js);

    // 检查函数是否正确生成
    EXPECT_NE(cpp.find("createPlayer"), std::string::npos);
}

TEST_F(ObjectGenerationTest, EmptyObjectFallback) {
    std::string js = R"(
        let empty = {};
    )";

    std::string cpp = GenerateFromJS(js);

    // 空对象应该回退到动态类型
    EXPECT_NE(cpp.find("mjs::generated::JSObject"), std::string::npos);
}

TEST_F(ObjectGenerationTest, MixedTypesInObject) {
    std::string js = R"(
        let data = {
            id: 123,
            score: 99.5,
            active: true,
            name: "Player"
        };
    )";

    std::string cpp = GenerateFromJS(js);

    // 检查是否为不同类型生成了正确的成员类型
    EXPECT_NE(cpp.find("int64_t id;"), std::string::npos);
    EXPECT_NE(cpp.find("double score;"), std::string::npos);
    EXPECT_NE(cpp.find("bool active;"), std::string::npos);
    EXPECT_NE(cpp.find("std::string name;"), std::string::npos);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
