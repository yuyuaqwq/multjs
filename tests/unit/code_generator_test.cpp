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
            auto it = BytecodeTable::opcode_type_map().find(current_opcode);
            if (it != BytecodeTable::opcode_type_map().end()) {
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
            auto it = BytecodeTable::opcode_type_map().find(current_opcode);
            if (it != BytecodeTable::opcode_type_map().end()) {
                for (char par_size : it->second.par_size_list) {
                    pc += par_size;
                }
            }
        }
        
        return count;
    }

    /**
     * @brief 获取完整的指令序列
     * @param module_value 模块值
     * @return 指令序列
     */
    std::vector<OpcodeType> GetOpcodeSequence(const Value& module_value) {
        std::vector<OpcodeType> opcodes;
        
        if (!module_value.IsModuleDef()) {
            return opcodes;
        }
        
        const auto& bytecode_table = module_value.module_def().bytecode_table();
        
        for (Pc pc = 0; pc < bytecode_table.Size(); ) {
            OpcodeType current_opcode = bytecode_table.GetOpcode(pc);
            opcodes.push_back(current_opcode);
            
            // 根据指令类型跳过参数
            pc++;
            
            // 获取指令信息
            auto it = BytecodeTable::opcode_type_map().find(current_opcode);
            if (it != BytecodeTable::opcode_type_map().end()) {
                for (char par_size : it->second.par_size_list) {
                    pc += par_size;
                }
            }
        }
        
        return opcodes;
    }

    /**
     * @brief 检查指令序列是否包含指定的子序列
     * @param module_value 模块值
     * @param expected_opcodes 期望的操作码序列
     * @return 是否包含该子序列
     */
    bool ContainsOpcodeSequence(const Value& module_value, const std::vector<OpcodeType>& expected_opcodes) {
        auto actual_opcodes = GetOpcodeSequence(module_value);
        
        if (actual_opcodes.size() < expected_opcodes.size()) {
            return false;
        }
        
        // 使用滑动窗口查找子序列
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

    /**
     * @brief 验证代码生成是否成功
     * @param module_value 模块值
     * @return 是否成功生成
     */
    bool IsValidModule(const Value& module_value) {
        return module_value.IsModuleDef() && 
               module_value.module_def().bytecode_table().Size() > 0;
    }

    Runtime runtime_;
    std::optional<Context> context_;
};

// ============================================================================
// 基础字面量测试
// ============================================================================

TEST_F(CodeGeneratorTest, UndefinedLiteral) {
    Value module_value = GenerateCode("undefined;");
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kUndefined));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kPop));
}

TEST_F(CodeGeneratorTest, NullLiteral) {
    Value module_value = GenerateCode("null;");
    
    EXPECT_TRUE(IsValidModule(module_value));
    // null 作为常量加载
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) || 
                ContainsOpcode(module_value, OpcodeType::kCLoad_0));
}

TEST_F(CodeGeneratorTest, BooleanLiterals) {
    Value module_value = GenerateCode("true; false;");
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kPop), 2); // 两个表达式语句
}

TEST_F(CodeGeneratorTest, NumberLiterals) {
    Value module_value = GenerateCode("42; 3.14; -5;");
    
    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有常量加载指令
    int cload_count = CountOpcode(module_value, OpcodeType::kCLoad_0) +
                     CountOpcode(module_value, OpcodeType::kCLoad_1) +
                     CountOpcode(module_value, OpcodeType::kCLoad_2) +
                     CountOpcode(module_value, OpcodeType::kCLoad_3) +
                     CountOpcode(module_value, OpcodeType::kCLoad_4) +
                     CountOpcode(module_value, OpcodeType::kCLoad_5) +
                     CountOpcode(module_value, OpcodeType::kCLoad) +
                     CountOpcode(module_value, OpcodeType::kCLoadW) +
                     CountOpcode(module_value, OpcodeType::kCLoadD);
    EXPECT_GE(cload_count, 3);
}

TEST_F(CodeGeneratorTest, StringLiteral) {
    Value module_value = GenerateCode("'hello'; \"world\";");
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kPop), 2);
}

// ============================================================================
// 算术运算测试
// ============================================================================

TEST_F(CodeGeneratorTest, ArithmeticOperators) {
    Value module_value = GenerateCode(
        "1 + 2;\n"
        "5 - 3;\n"
        "4 * 6;\n"
        "8 / 2;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kSub));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kMul));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kDiv));
}

TEST_F(CodeGeneratorTest, ArithmeticOperatorLevel) {
    Value module_value = GenerateCode("1 + 2 * 3;");
    
    EXPECT_TRUE(IsValidModule(module_value));
    // 应该先执行乘法，再执行加法
    auto opcodes = GetOpcodeSequence(module_value);
    
    // 查找乘法和加法指令的位置
    auto mul_pos = std::find(opcodes.begin(), opcodes.end(), OpcodeType::kMul);
    auto add_pos = std::find(opcodes.begin(), opcodes.end(), OpcodeType::kAdd);
    
    EXPECT_NE(mul_pos, opcodes.end());
    EXPECT_NE(add_pos, opcodes.end());
    EXPECT_LT(mul_pos, add_pos); // 乘法应该在加法之前
}

TEST_F(CodeGeneratorTest, UnaryOperators) {
    Value module_value = GenerateCode("-5;");
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kNeg));
}

// ============================================================================
// 比较运算测试
// ============================================================================

TEST_F(CodeGeneratorTest, ComparisonOperators) {
    Value module_value = GenerateCode(
        "1 < 2;\n"
        "3 > 4;\n"
        "5 <= 6;\n"
        "7 >= 8;\n"
        "9 == 10;\n"
        "11 != 12;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kLt));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGt));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kLe));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGe));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kEq));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kNe));
}

// ============================================================================
// 位运算测试
// ============================================================================

TEST_F(CodeGeneratorTest, BitwiseOperators) {
    Value module_value = GenerateCode(
        "1 << 2;\n"
        "8 >> 1;\n"
        "15 & 7;\n"
        "8 | 4;\n"
        "5 ^ 3;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kShl));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kShr));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kBitAnd));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kBitOr));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kBitXor));
}

// ============================================================================
// 变量声明和访问测试
// ============================================================================

TEST_F(CodeGeneratorTest, VariableDeclaration) {
    Value module_value = GenerateCode(
        "let a = 5;\n"
        "const b = 10;\n"
        "a = 15;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有变量存储指令
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kVStore_0) +
              CountOpcode(module_value, OpcodeType::kVStore_1) +
              CountOpcode(module_value, OpcodeType::kVStore_2) +
              CountOpcode(module_value, OpcodeType::kVStore), 3);
}

TEST_F(CodeGeneratorTest, VariableAccess) {
    Value module_value = GenerateCode(
        "let x = 10;\n"
        "let y = x + 5;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有变量加载指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kVLoad_0) ||
                ContainsOpcode(module_value, OpcodeType::kVLoad));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
}

TEST_F(CodeGeneratorTest, VariableAssignment) {
    Value module_value = GenerateCode(
        "let x = 5;\n"
        "x = 10;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有两次变量存储
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kVStore_0) +
              CountOpcode(module_value, OpcodeType::kVStore), 2);
}

// ============================================================================
// 数组测试
// ============================================================================

TEST_F(CodeGeneratorTest, ArrayLiteral) {
    Value module_value = GenerateCode("let arr = [1, 2, 3, 4, 5];");
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kCLoad), 7);
}

TEST_F(CodeGeneratorTest, ArrayAccess) {
    Value module_value = GenerateCode( 
        "let arr = [1, 2, 3];\n"
        "let x = arr[1];\n"
    );
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kCLoad), 6);
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIndexedLoad));
}

TEST_F(CodeGeneratorTest, ArrayAssignment) {
    Value module_value = GenerateCode(
        "let arr = [1, 2, 3];\n"
        "arr[0] = 10;\n"
    );
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kCLoad), 7);
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kIndexedStore), 1);
}

TEST_F(CodeGeneratorTest, EmptyArray) {
    Value module_value = GenerateCode("let arr = [];");
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kCLoad), 2);
}

// ============================================================================
// 对象测试
// ============================================================================

TEST_F(CodeGeneratorTest, ObjectLiteral) {
    Value module_value = GenerateCode("let obj = { a: 1, b: 2, c: 3 };");
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kCLoad), 8);
}

TEST_F(CodeGeneratorTest, ObjectAccess) {
    Value module_value = GenerateCode(
        "let obj = { x: 10 };\n"
        "let y = obj.x;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kPropertyLoad));
}

TEST_F(CodeGeneratorTest, ObjectAssignment) {
    Value module_value = GenerateCode(
        "let obj = { x: 5 };\n"
        "obj.x = 10;\n"
    );

    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kCLoad), 5);
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kPropertyStore), 1);
}

TEST_F(CodeGeneratorTest, EmptyObject) {
    Value module_value = GenerateCode("let obj = {};");
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kCLoad), 2);
}

// ============================================================================
// 控制流测试
// ============================================================================

TEST_F(CodeGeneratorTest, IfStatement) {
    Value module_value = GenerateCode(
        "let x = 5;\n"
        "if (x > 3) {\n"
        "  x = 10;\n"
        "}\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGt));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIfEq));
}

TEST_F(CodeGeneratorTest, IfElseStatement) {
    Value module_value = GenerateCode(
        "let x = 5;\n"
        "if (x > 10) {\n"
        "  x = 1;\n"
        "} else {\n"
        "  x = 2;\n"
        "}\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIfEq));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGoto));
}

TEST_F(CodeGeneratorTest, IfElseIfStatement) {
    Value module_value = GenerateCode(
        "let x = 5;\n"
        "if (x > 10) {\n"
        "  x = 1;\n"
        "} else if (x > 5) {\n"
        "  x = 2;\n"
        "} else {\n"
        "  x = 3;\n"
        "}\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kIfEq), 2);
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kGoto), 2);
}

TEST_F(CodeGeneratorTest, WhileLoop) {
    Value module_value = GenerateCode(
        "let i = 0;\n"
        "while (i < 5) {\n"
        "  i = i + 1;\n"
        "}\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kLt));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIfEq));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGoto));
}

TEST_F(CodeGeneratorTest, ForLoop) {
    Value module_value = GenerateCode(
        "for (let i = 0; i < 5; i++) {\n"
        "  let x = i;\n"
        "}\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kLt));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIfEq));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kGoto));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kInc));
}

// ============================================================================
// 函数测试
// ============================================================================

TEST_F(CodeGeneratorTest, FunctionDeclaration) {
    Value module_value = GenerateCode(
        "function add(a, b) {\n"
        "  return a + b;\n"
        "}\n"
    );

    // std::cout << GetDisassembly(module_value);
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kVStore_0));
}

TEST_F(CodeGeneratorTest, FunctionCall) {
    Value module_value = GenerateCode(
        "function greet() {\n"
        "  return 'hello';\n"
        "}\n"
        "greet();\n"
    );

    // std::cout << GetDisassembly(module_value);
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kFunctionCall));
}

TEST_F(CodeGeneratorTest, FunctionWithParameters) {
    Value module_value = GenerateCode(
        "function multiply(x, y) {\n"
        "  return x * y;\n"
        "}\n"
        "multiply(3, 4);\n"
    );

    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kCLoad), 3);
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kFunctionCall));
}

TEST_F(CodeGeneratorTest, ArrowFunction) {
    Value module_value = GenerateCode(
        "let add = (a, b) => a + b;\n"
        "add(1, 2);\n"
    );
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kFunctionCall));
}

TEST_F(CodeGeneratorTest, NestedFunctions) {
    Value module_value = GenerateCode(
        "function outer() {\n"
        "  function inner() {\n"
        "    return 42;\n"
        "  }\n"
        "  return inner();\n"
        "}\n"
        "outer();\n"
    );
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kFunctionCall), 1);
}

// ============================================================================
// 异常处理测试
// ============================================================================

TEST_F(CodeGeneratorTest, TryStatement) {
    Value module_value = GenerateCode(
        "try {\n"
        "  let x = 1;\n"
        "} catch (e) {\n"
        "  let y = 2;\n"
        "}\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kTryBegin));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kTryEnd));
}

TEST_F(CodeGeneratorTest, TryFinally) {
    Value module_value = GenerateCode(
        "try {\n"
        "  let x = 1;\n"
        "} finally {\n"
        "  let y = 2;\n"
        "}\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kTryBegin));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kTryEnd));
}

TEST_F(CodeGeneratorTest, ThrowStatement) {
    Value module_value = GenerateCode(
        "try {\n"
        "  throw 'error';\n"
        "} catch (e) {\n"
        "  let x = e;\n"
        "}\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kThrow));
}

// ============================================================================
// 复杂表达式测试
// ============================================================================

TEST_F(CodeGeneratorTest, ComplexArithmeticExpression) {
    Value module_value = GenerateCode("let result = (1 + 2) * (3 - 4) / 5;");
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kSub));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kMul));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kDiv));
}

TEST_F(CodeGeneratorTest, ChainedMemberAccess) {
    Value module_value = GenerateCode(
        "let obj = { a: { b: { c: 42 } } };\n"
        "let value = obj.a.b.c;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kPropertyLoad), 3);
}

TEST_F(CodeGeneratorTest, MixedArrayObjectAccess) {
    Value module_value = GenerateCode(
        "let data = [{ x: 1 }, { x: 2 }];\n"
        "let value = data[0].x;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kIndexedLoad));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kPropertyLoad));
}

// ============================================================================
// 边界情况测试
// ============================================================================

TEST_F(CodeGeneratorTest, EmptyProgram) {
    Value module_value = GenerateCode("");
    
    EXPECT_TRUE(IsValidModule(module_value));
    // 应该至少有 undefined 和 return 指令
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kUndefined));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kReturn));
}

TEST_F(CodeGeneratorTest, OnlyComments) {
    Value module_value = GenerateCode("// This is a comment\n/* Another comment */");
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kUndefined));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kReturn));
}

TEST_F(CodeGeneratorTest, SingleExpression) {
    Value module_value = GenerateCode("42;");
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kUndefined));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kReturn));
}

// ============================================================================
// 作用域测试
// ============================================================================

TEST_F(CodeGeneratorTest, BlockScope) {
    Value module_value = GenerateCode(
        "let x = 1;\n"
        "{\n"
        "  let x = 2;\n"
        "  let y = x;\n"
        "}\n"
        "let z = x;\n"
    );
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有多个变量存储指令
    EXPECT_EQ(CountOpcode(module_value, OpcodeType::kVStore_0) +
              CountOpcode(module_value, OpcodeType::kVStore_1) +
              CountOpcode(module_value, OpcodeType::kVStore_2) +
              CountOpcode(module_value, OpcodeType::kVStore_3), 4);
}

TEST_F(CodeGeneratorTest, FunctionScope) {
    Value module_value = GenerateCode(
        "let x = 1;\n"
        "function test() {\n"
        "  let x = 2;\n"
        "  return x;\n"
        "}\n"
        "test();\n"
    );
    
    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kFunctionCall));
}

// ============================================================================
// 类型转换和特殊值测试
// ============================================================================

TEST_F(CodeGeneratorTest, ImplicitTypeConversion) {
    Value module_value = GenerateCode(
        "let str = 'hello';\n"
        "let num = 42;\n"
        "let result = str + num;\n"
    );
    
    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kAdd));
}

TEST_F(CodeGeneratorTest, IncrementOperators) {
    Value module_value = GenerateCode(
        "let x = 5;\n"
        "++x;\n"
        "x++;\n"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kInc), 2);
}

// ============================================================================
// Class 测试
// ============================================================================

TEST_F(CodeGeneratorTest, SimpleClassDeclaration) {
    Value module_value = GenerateCode("class MyClass { }");

    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有构造函数的常量加载
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
}

TEST_F(CodeGeneratorTest, ClassDeclarationWithConstructor) {
    Value module_value = GenerateCode(
        "class Person {\n"
        "  constructor(name) {\n"
        "    this.name = name;\n"
        "  }\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有构造函数定义
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
}

TEST_F(CodeGeneratorTest, ClassDeclarationWithMethods) {
    Value module_value = GenerateCode(
        "class Rectangle {\n"
        "  constructor(w, h) {\n"
        "    this.width = w;\n"
        "    this.height = h;\n"
        "  }\n"
        "  getArea() {\n"
        "    return this.width * this.height;\n"
        "  }\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有多个函数定义（构造函数 + 方法）
    int cload_count = CountOpcode(module_value, OpcodeType::kCLoad) +
                     CountOpcode(module_value, OpcodeType::kCLoadD);
    EXPECT_GE(cload_count, 2);
}

TEST_F(CodeGeneratorTest, ClassDeclarationWithExtends) {
    Value module_value = GenerateCode(
        "class Dog extends Animal {\n"
        "  constructor(name) {\n"
        "    super(name);\n"
        "  }\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有属性存储指令（设置原型链）
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kPropertyStore));
}

TEST_F(CodeGeneratorTest, ClassWithFields) {
    Value module_value = GenerateCode(
        "class Rectangle {\n"
        "  width = 0;\n"
        "  height = 0;\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有构造函数和字段初始化
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
}

TEST_F(CodeGeneratorTest, ClassWithStaticFields) {
    Value module_value = GenerateCode(
        "class Constants {\n"
        "  static PI = 3.14159;\n"
        "  static E = 2.71828;\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 静态字段应该有属性存储指令
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kPropertyStore), 2);
}

TEST_F(CodeGeneratorTest, ClassWithGetter) {
    Value module_value = GenerateCode(
        "class Circle {\n"
        "  get radius() {\n"
        "    return this._radius;\n"
        "  }\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // getter应该有函数定义
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
}

TEST_F(CodeGeneratorTest, ClassWithSetter) {
    Value module_value = GenerateCode(
        "class Circle {\n"
        "  set radius(value) {\n"
        "    this._radius = value;\n"
        "  }\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // setter应该有函数定义
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
}

TEST_F(CodeGeneratorTest, ClassWithStaticMethod) {
    Value module_value = GenerateCode(
        "class MathHelper {\n"
        "  static add(a, b) {\n"
        "    return a + b;\n"
        "  }\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 静态方法应该有函数定义和属性存储
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kPropertyStore));
}

TEST_F(CodeGeneratorTest, AnonymousClassExpression) {
    Value module_value = GenerateCode("const MyClass = class { };");

    EXPECT_TRUE(IsValidModule(module_value));
    // 匿名类应该有函数定义
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
}

TEST_F(CodeGeneratorTest, NamedClassExpression) {
    Value module_value = GenerateCode("const MyClass = class NamedClass { };");

    EXPECT_TRUE(IsValidModule(module_value));
    // 命名类表达式应该有函数定义
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
}

TEST_F(CodeGeneratorTest, ClassExpressionInAssignment) {
    Value module_value = GenerateCode(
        "let MyClass = class {\n"
        "  constructor() {\n"
        "    this.value = 42;\n"
        "  }\n"
        "};"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有变量存储和函数定义
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kVStore) ||
                ContainsOpcode(module_value, OpcodeType::kVStore_0));
}

TEST_F(CodeGeneratorTest, ComplexClassWithAllFeatures) {
    Value module_value = GenerateCode(R"(
        class Student extends Person {
            static count = 0;
            school = 'default';

            constructor(name, age, school) {
                super(name, age);
                this.school = school;
                Student.count++;
            }

            get info() {
                return `${this.name} - ${this.school}`;
            }

            set info(value) {
                // setter implementation
            }

            static getCount() {
                return Student.count;
            }

            study() {
                return `${this.name} is studying`;
            }
        }
    )");

    EXPECT_TRUE(IsValidModule(module_value));
    // 复杂类应该有多个函数定义（构造函数、getter、setter、静态方法、实例方法）
    int cload_count = CountOpcode(module_value, OpcodeType::kCLoad) +
                     CountOpcode(module_value, OpcodeType::kCLoadD);
    EXPECT_GE(cload_count, 5);
    // 应该有属性存储（设置原型链和静态字段）
    EXPECT_GE(CountOpcode(module_value, OpcodeType::kPropertyStore), 3);
}

TEST_F(CodeGeneratorTest, ClassWithSuperCall) {
    Value module_value = GenerateCode(
        "class Child extends Parent {\n"
        "  constructor(name) {\n"
        "    super(name);\n"
        "  }\n"
        "}"
    );

    // std::cout << GetDisassembly(module_value);

    EXPECT_TRUE(IsValidModule(module_value));
    // super调用代码生成已实现，但kGetSuper指令在构造函数的字节码中，不在module级别
    // 这里只验证类能正确生成即可
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kPropertyStore)); // 验证类定义生成
}

TEST_F(CodeGeneratorTest, MultipleClassesInSameModule) {
    Value module_value = GenerateCode(
        "class A { }\n"
        "class B extends A { }\n"
        "class C extends B { }\n"
        "class D { }\n"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 应该有4个构造函数定义
    int cload_count = CountOpcode(module_value, OpcodeType::kCLoad) +
                     CountOpcode(module_value, OpcodeType::kCLoadD);
    EXPECT_GE(cload_count, 4);
}

TEST_F(CodeGeneratorTest, ClassWithComputedPropertyName) {
    Value module_value = GenerateCode(
        "class C {\n"
        "  [methodName]() { }\n"
        "}"
    );

    EXPECT_TRUE(IsValidModule(module_value));
    // 计算属性名应该有函数定义
    EXPECT_TRUE(ContainsOpcode(module_value, OpcodeType::kCLoad) ||
                ContainsOpcode(module_value, OpcodeType::kCLoadD));
}

} // namespace test
} // namespace compiler
} // namespace mjs 