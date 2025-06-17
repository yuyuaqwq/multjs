/**
 * @file code_generator_test.cpp
 * @brief 代码生成器单元测试
 */
#include <gtest/gtest.h>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value.h>

#include "../src/compiler/code_generator.h"
#include "../src/compiler/parser.h"
#include "../src/compiler/lexer.h"

namespace mjs {
namespace compiler {
namespace test {

class CodeGeneratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        context_.emplace(&runtime_);
    }

    void TearDown() override {

    }

    /**
     * @brief 测试生成代码
     * @param source 源代码
     * @return 生成的模块值
     */
    Value GenerateCode(const std::string& source) {
        // 创建词法分析器
        Lexer lexer(source);
        
        // 创建语法分析器
        Parser parser(&lexer);
        
        parser.ParseProgram();

        // 创建代码生成器
        CodeGenerator generator(&*context_, &parser);
        
        // 生成代码
        return generator.Generate("test", source);
    }

    /**
     * @brief 检查指令序列是否包含特定指令
     * @param module_value 模块值
     * @param opcode 操作码
     * @return 是否包含该指令
     */
    bool ContainsOpcode(const Value& module_value, OpcodeType opcode) {
        if (!module_value.IsModuleDef()) {
            return false;
        }
        
        const auto& bytecode_table = module_value.module_def().bytecode_table();
        
        for (Pc pc = 0; pc < bytecode_table.Size(); ) {
            OpcodeType current_opcode = bytecode_table.GetOpcode(pc);
            
            if (current_opcode == opcode) {
                return true;
            }
            
            // 根据指令类型跳过参数
            pc++;
            
            // 获取指令信息
            auto it = g_instr_symbol.find(current_opcode);
            if (it != g_instr_symbol.end()) {
                for (char par_size : it->second.par_size_list) {
                    pc += par_size;
                }
            }
        }
        
        return false;
    }

    /**
     * @brief 获取指令序列中特定指令的数量
     * @param module_value 模块值
     * @param opcode 操作码
     * @return 指令数量
     */
    int CountOpcode(const Value& module_value, OpcodeType opcode) {
        if (!module_value.IsModuleDef()) {
            return 0;
        }
        
        const auto& bytecode_table = module_value.module_def().bytecode_table();
        int count = 0;
        
        for (Pc pc = 0; pc < bytecode_table.Size(); ) {
            OpcodeType current_opcode = bytecode_table.GetOpcode(pc);
            
            if (current_opcode == opcode) {
                count++;
            }
            
            // 根据指令类型跳过参数
            pc++;
            
            // 获取指令信息
            auto it = g_instr_symbol.find(current_opcode);
            if (it != g_instr_symbol.end()) {
                for (char par_size : it->second.par_size_list) {
                    pc += par_size;
                }
            }
        }
        
        return count;
    }

    /**
     * @brief 检查指令序列
     * @param module_value 模块值
     * @param expected_opcodes 期望的操作码序列
     * @return 是否符合期望
     */
    bool CheckOpcodeSequence(const Value& module_value, const std::vector<OpcodeType>& expected_opcodes) {
        if (!module_value.IsModuleDef()) {
            return false;
        }
        
        const auto& bytecode_table = module_value.module_def().bytecode_table();
        std::vector<OpcodeType> actual_opcodes;
        
        for (Pc pc = 0; pc < bytecode_table.Size(); ) {
            OpcodeType current_opcode = bytecode_table.GetOpcode(pc);
            actual_opcodes.push_back(current_opcode);
            
            // 根据指令类型跳过参数
            pc++;
            
            // 获取指令信息
            auto it = g_instr_symbol.find(current_opcode);
            if (it != g_instr_symbol.end()) {
                for (char par_size : it->second.par_size_list) {
                    pc += par_size;
                }
            }
        }
        
        // 检查序列长度
        if (actual_opcodes.size() < expected_opcodes.size()) {
            return false;
        }
        
        // 检查序列是否包含期望的子序列
        for (size_t i = 0; i <= actual_opcodes.size() - expected_opcodes.size(); ++i) {
            bool match = true;
            for (size_t j = 0; j < expected_opcodes.size(); ++j) {
                if (actual_opcodes[i + j] != expected_opcodes[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return true;
            }
        }
        
        return false;
    }

    /**
     * @brief 获取模块的反汇编代码
     * @param module_value 模块值
     * @return 反汇编代码
     */
    std::string GetDisassembly(const Value& module_value) {
        if (!module_value.IsModuleDef()) {
            return "";
        }
        
        return module_value.module_def().Disassembly(&*context_);
    }

    Runtime runtime_;
    std::optional<Context> context_;
};

// 测试生成简单表达式
TEST_F(CodeGeneratorTest, SimpleExpression) {
    Value module_value = GenerateCode("1 + 2;");
    
    // 检查是否生成了加法指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
    
    // 检查指令序列
    std::vector<OpcodeType> expected_sequence = {
        OpcodeType::kCLoad_1,  // 加载常量1
        OpcodeType::kCLoad_2,  // 加载常量2
        OpcodeType::kAdd       // 执行加法
    };
    
    EXPECT_TRUE(CheckOpcodeSequence(module_value, expected_sequence));
}

// 测试生成变量声明和引用
TEST_F(CodeGeneratorTest, VariableDeclaration) {
    Value module_value = GenerateCode(
        "let a = 5;\n"
        "let b = 3;\n"
        "a + b;"
    );
    
    // 检查是否生成了变量存储和加载指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kVStore_0));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kVStore_1));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kVLoad_0));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kVLoad_1));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
}

// 测试生成条件语句
TEST_F(CodeGeneratorTest, IfStatement) {
    Value module_value = GenerateCode(
        "let a = 5;\n"
        "if (a > 3) {\n"
        "  a = 10;\n"
        "} else {\n"
        "  a = 0;\n"
        "}\n"
    );
    
    // 检查是否生成了条件跳转指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGt));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIfEq));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGoto));
}

// 测试生成循环语句
TEST_F(CodeGeneratorTest, LoopStatement) {
    Value module_value = GenerateCode(
        "let sum = 0;\n"
        "for (let i = 1; i <= 5; i++) {\n"
        "  sum += i;\n"
        "}\n"
    );
    
    // 检查是否生成了循环相关指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kLe));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIfEq));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGoto));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
    
    // 检查goto指令数量（至少有一个用于循环跳转）
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kGoto), 1);
}

// 测试生成函数声明和调用
TEST_F(CodeGeneratorTest, FunctionDeclaration) {
    Value module_value = GenerateCode(
        "function add(a, b) {\n"
        "  return a + b;\n"
        "}\n"
        "add(3, 4);"
    );
    
    // 检查是否生成了函数闭包和调用指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kClosure));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kFunctionCall));
    
    // 检查函数体中是否有加法和返回指令
    std::string disassembly = GetDisassembly(module_value);
    EXPECT_TRUE(disassembly.find("add") != std::string::npos);
    EXPECT_TRUE(disassembly.find("return") != std::string::npos);
}

// 测试生成数组表达式
TEST_F(CodeGeneratorTest, ArrayExpression) {
    Value module_value = GenerateCode(
        "let arr = [1, 2, 3, 4, 5];\n"
    );
    
    // 检查是否生成了数组创建和元素存储指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kNew));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIndexedStore));
    
    // 检查索引存储指令的数量是否等于数组元素数量
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kIndexedStore), 5);
}

// 测试生成对象表达式
TEST_F(CodeGeneratorTest, ObjectExpression) {
    Value module_value = GenerateCode(
        "let obj = { a: 1, b: 2, c: 3 };\n"
    );
    
    // 检查是否生成了对象创建和属性存储指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kNew));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kPropertyStore));
    
    // 检查属性存储指令的数量是否等于对象属性数量
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kPropertyStore), 3);
}

// 测试生成异常处理
TEST_F(CodeGeneratorTest, ExceptionHandling) {
    Value module_value = GenerateCode(
        "try {\n"
        "  throw 'error';\n"
        "} catch (e) {\n"
        "  42;\n"
        "}\n"
    );
    
    // 检查是否生成了异常处理相关指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kTryBegin));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kThrow));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kTryEnd));
}

// 测试生成比较运算符
TEST_F(CodeGeneratorTest, ComparisonOperators) {
    Value module_value = GenerateCode(
        "1 < 2;\n"
        "3 > 4;\n"
        "5 <= 6;\n"
        "7 >= 8;\n"
        "9 == 10;\n"
        "11 != 12;\n"
    );
    
    // 检查是否生成了各种比较指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kLt));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGt));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kLe));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGe));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kEq));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kNe));
}

// 测试生成算术运算符
TEST_F(CodeGeneratorTest, ArithmeticOperators) {
    Value module_value = GenerateCode(
        "1 + 2;\n"
        "3 - 4;\n"
        "5 * 6;\n"
        "7 / 8;\n"
    );
    
    // 检查是否生成了各种算术指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kSub));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kMul));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kDiv));
}

// 测试生成位运算符
TEST_F(CodeGeneratorTest, BitwiseOperators) {
    Value module_value = GenerateCode(
        "1 << 2;\n"
        "3 >> 4;\n"
    );
    
    // 检查是否生成了位运算指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kShl));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kShr));
}

// 测试生成复杂表达式
TEST_F(CodeGeneratorTest, ComplexExpression) {
    Value module_value = GenerateCode(
        "let x = 1 + 2 * 3 - 4 / 2;\n"
    );
    
    // 检查是否生成了各种算术指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kMul));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kSub));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kDiv));
}

// 测试嵌套函数
TEST_F(CodeGeneratorTest, NestedFunctions) {
    Value module_value = GenerateCode(
        "function outer() {\n"
        "  function inner() {\n"
        "    return 42;\n"
        "  }\n"
        "  return inner();\n"
        "}\n"
    );
    
    // 检查是否生成了嵌套的闭包
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kClosure), 2);
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kFunctionCall));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kReturn));
}

} // namespace test
} // namespace compiler
} // namespace mjs 