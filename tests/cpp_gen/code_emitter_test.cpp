/**
 * @file code_emitter_test.cpp
 * @brief CodeEmitter单元测试
 */

#include <gtest/gtest.h>
#include "cpp_gen/code_emitter.h"

using namespace mjs::compiler::cpp_gen;

TEST(CodeEmitterTest, BasicOutput) {
    CodeEmitter emitter;

    emitter.EmitLine("int x = 42;");
    emitter.EmitLine("double y = 3.14;");

    std::string output = emitter.ToString();
    EXPECT_EQ(output, "int x = 42;\ndouble y = 3.14;\n");
}

TEST(CodeEmitterTest, Indentation) {
    CodeEmitter emitter;

    emitter.EmitLine("if (true) {");
    emitter.Indent();
    emitter.EmitLine("int x = 42;");
    emitter.EmitLine("return x;");
    emitter.Dedent();
    emitter.EmitLine("}");

    std::string output = emitter.ToString();
    EXPECT_EQ(output,
        "if (true) {\n"
        "    int x = 42;\n"
        "    return x;\n"
        "}\n");
}

TEST(CodeEmitterTest, BlockHelpers) {
    CodeEmitter emitter;

    emitter.EmitLine("void test()");
    emitter.EmitBlockStart();
    emitter.EmitLine("int x = 42;");
    emitter.EmitBlockEnd();

    std::string output = emitter.ToString();
    EXPECT_EQ(output,
        "void test()\n"
        "{\n"
        "    int x = 42;\n"
        "}\n");
}

TEST(CodeEmitterTest, BlankLines) {
    CodeEmitter emitter;

    emitter.EmitLine("int x = 42;");
    emitter.EmitBlankLine();
    emitter.EmitLine("int y = 24;");

    std::string output = emitter.ToString();
    EXPECT_EQ(output, "int x = 42;\n\nint y = 24;\n");
}

TEST(CodeEmitterTest, CustomIndentSize) {
    CodeEmitter emitter(2);

    emitter.EmitLine("if (true) {");
    emitter.Indent();
    emitter.EmitLine("int x = 42;");
    emitter.Dedent();
    emitter.EmitLine("}");

    std::string output = emitter.ToString();
    EXPECT_EQ(output,
        "if (true) {\n"
        "  int x = 42;\n"
        "}\n");
}

TEST(CodeEmitterTest, Clear) {
    CodeEmitter emitter;

    emitter.EmitLine("int x = 42;");
    emitter.Clear();
    emitter.EmitLine("int y = 24;");

    std::string output = emitter.ToString();
    EXPECT_EQ(output, "int y = 24;\n");
}
