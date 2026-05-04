#include <gtest/gtest.h>
#include <mjs/regexp/regexp_parser.h>
#include <mjs/regexp/regexp_nfa.h>
#include <iostream>

using namespace mjs;

TEST(BackrefTest, NumberedBackref) {
    // 测试编号反向引用 (a)\1
    RegExpParser parser("(a)\1");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);
    
    NFA nfa = NFABuilder::Build(ast.get());
    
    // 应该匹配 "aa"
    bool result = nfa.Match("aa");
    std::cout << "Match 'aa': " << (result ? "true" : "false") << std::endl;
    EXPECT_TRUE(result);
    
    // 不应该匹配 "ab"
    result = nfa.Match("ab");
    std::cout << "Match 'ab': " << (result ? "true" : "false") << std::endl;
    EXPECT_FALSE(result);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
