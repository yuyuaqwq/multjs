/**
 * @file regexp_nfa_test.cpp
 * @brief 姝ｅ垯琛ㄨ揪寮廚FA娴嬭�?
 *
 * 娴嬭瘯姝ｅ垯琛ㄨ揪寮廚FA鐨勫尮閰嶅姛鑳斤紝鍖呮嫭:
 * - 鍩烘湰瀛楃鍖归厤
 * - 瀛楃绫诲尮�?
 * - 鐗规畩瀛楃鍖归厤
 * - 杞箟搴忓垪鍖归�?
 * - 閲忚瘝鍖归厤
 * - 閫夋嫨鍖归厤
 * - 鍒嗙粍鍖归厤
 * - 澶у皬鍐欎笉鏁忔劅鍖归�?
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "mjs/regexp/regexp_parser.h"
#include "mjs/regexp/regexp_nfa.h"

namespace mjs {
namespace test {

/**
 * @class RegExpNFATest
 * @brief 姝ｅ垯琛ㄨ揪寮廚FA娴嬭瘯绫?
 */
class RegExpNFATest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}

    /**
     * @brief 杈呭姪鏂规硶锛氭瀯寤篘FA骞舵祴璇曞尮�?
     */
    bool Match(const std::string& pattern, const std::string& str, bool ignore_case = false) {
        RegExpParser parser(pattern);
        auto ast = parser.Parse();
        if (!ast) {
            return false;
        }
        NFA nfa = NFABuilder::Build(ast.get(), ignore_case);
        return nfa.Match(str);
    }

    /**
     * @brief 杈呭姪鏂规硶锛氭瀯寤篘FA骞舵祴璇曟悳�?
     */
    std::optional<size_t> Search(const std::string& pattern, const std::string& str,
                                  size_t start_pos = 0, bool ignore_case = false) {
        RegExpParser parser(pattern);
        auto ast = parser.Parse();
        if (!ast) {
            return std::nullopt;
        }
        NFA nfa = NFABuilder::Build(ast.get(), ignore_case);
        return nfa.MatchEnd(str, start_pos);
    }

    /**
     * @brief 杈呭姪鏂规硶锛氫娇鐢ㄥ畬鏁存爣蹇楁瀯寤篘FA骞舵祴璇曞尮�?
     */
    bool MatchWithFlags(const std::string& pattern, const std::string& str,
                        bool ignore_case = false, bool multiline = false,
                        bool dot_all = false, bool unicode = false) {
        RegExpParser parser(pattern, unicode);
        auto ast = parser.Parse();
        if (!ast) {
            return false;
        }
        NFA nfa = NFABuilder::Build(ast.get(), ignore_case, multiline, dot_all, unicode);
        return nfa.Match(str);
    }

    /**
     * @brief 杈呭姪鏂规硶锛氫娇鐢ㄥ畬鏁存爣蹇楁瀯寤篘FA骞舵祴璇曟悳�?
     */
    std::optional<size_t> SearchWithFlags(const std::string& pattern, const std::string& str,
                                           size_t start_pos = 0, bool ignore_case = false,
                                           bool multiline = false, bool dot_all = false,
                                           bool unicode = false) {
        RegExpParser parser(pattern, unicode);
        auto ast = parser.Parse();
        if (!ast) {
            return std::nullopt;
        }
        NFA nfa = NFABuilder::Build(ast.get(), ignore_case, multiline, dot_all, unicode);
        return nfa.MatchEnd(str, start_pos);
    }
};

// ============================================================================
// 鍩烘湰瀛楃鍖归厤娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯鍗曚釜瀛楃鍖归厤
 */
TEST_F(RegExpNFATest, SingleCharacterMatch) {
    EXPECT_TRUE(Match("a", "a"));
    EXPECT_FALSE(Match("a", "b"));
    EXPECT_FALSE(Match("a", ""));
}

/**
 * @test 娴嬭瘯澶氫釜瀛楃鍖归厤
 */
TEST_F(RegExpNFATest, MultipleCharacterMatch) {
    EXPECT_TRUE(Match("abc", "abc"));
    EXPECT_FALSE(Match("abc", "abd"));
    EXPECT_FALSE(Match("abc", "ab"));
    EXPECT_FALSE(Match("abc", "abcd"));
}

/**
 * @test 娴嬭瘯澶у皬鍐欎笉鏁忔劅鍖归厤
 */
TEST_F(RegExpNFATest, CaseInsensitiveMatch) {
    EXPECT_TRUE(Match("a", "A", true));
    EXPECT_TRUE(Match("A", "a", true));
    EXPECT_TRUE(Match("abc", "ABC", true));
    EXPECT_TRUE(Match("abc", "AbC", true));
}

// ============================================================================
// 瀛楃绫诲尮閰嶆祴璇?
// ============================================================================

/**
 * @test 娴嬭瘯鍩烘湰瀛楃绫诲尮�?
 */
TEST_F(RegExpNFATest, BasicCharacterClassMatch) {
    EXPECT_TRUE(Match("[abc]", "a"));
    EXPECT_TRUE(Match("[abc]", "b"));
    EXPECT_TRUE(Match("[abc]", "c"));
    EXPECT_FALSE(Match("[abc]", "d"));
}

/**
 * @test 娴嬭瘯瀛楃绫昏寖鍥村尮閰?
 */
TEST_F(RegExpNFATest, CharacterClassRangeMatch) {
    EXPECT_TRUE(Match("[a-z]", "a"));
    EXPECT_TRUE(Match("[a-z]", "m"));
    EXPECT_TRUE(Match("[a-z]", "z"));
    EXPECT_FALSE(Match("[a-z]", "A"));
    EXPECT_FALSE(Match("[a-z]", "0"));

    EXPECT_TRUE(Match("[0-9]", "0"));
    EXPECT_TRUE(Match("[0-9]", "5"));
    EXPECT_TRUE(Match("[0-9]", "9"));
    EXPECT_FALSE(Match("[0-9]", "a"));
}

/**
 * @test 娴嬭瘯鍚﹀畾瀛楃绫诲尮�?
 */
TEST_F(RegExpNFATest, NegatedCharacterClassMatch) {
    EXPECT_FALSE(Match("[^abc]", "a"));
    EXPECT_FALSE(Match("[^abc]", "b"));
    EXPECT_FALSE(Match("[^abc]", "c"));
    EXPECT_TRUE(Match("[^abc]", "d"));
    EXPECT_TRUE(Match("[^abc]", "0"));
}

/**
 * @test 娴嬭瘯澶氫釜瀛楃绫昏寖鍥村尮閰?
 */
TEST_F(RegExpNFATest, MultipleCharacterClassRanges) {
    EXPECT_TRUE(Match("[a-zA-Z]", "a"));
    EXPECT_TRUE(Match("[a-zA-Z]", "Z"));
    EXPECT_TRUE(Match("[a-zA-Z]", "M"));
    EXPECT_FALSE(Match("[a-zA-Z]", "0"));

    EXPECT_TRUE(Match("[a-z0-9]", "a"));
    EXPECT_TRUE(Match("[a-z0-9]", "5"));
    EXPECT_FALSE(Match("[a-z0-9]", "A"));
}

TEST_F(RegExpNFATest, EmptyCharacterClassMatchesNothing) {
    EXPECT_FALSE(Match("[]", ""));
    EXPECT_FALSE(Match("[]", "a"));
}

TEST_F(RegExpNFATest, NegatedEmptyCharacterClassMatchesAnySingleCharacter) {
    EXPECT_TRUE(Match("[^]", "a"));
    EXPECT_TRUE(Match("[^]", "\n"));
    EXPECT_FALSE(Match("[^]", ""));
}

TEST_F(RegExpNFATest, IgnoreCaseAppliesToCharacterClasses) {
    EXPECT_TRUE(Match("[a-z]+", "ABC", true));
    EXPECT_TRUE(Match("[A-Z]+", "abc", true));
}

// ============================================================================
// 鐗规畩瀛楃鍖归厤娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯鐐瑰彿锛堜换鎰忓瓧绗︼級鍖归厤
 */
TEST_F(RegExpNFATest, DotCharacterMatch) {
    EXPECT_TRUE(Match(".", "a"));
    EXPECT_TRUE(Match(".", "b"));
    EXPECT_TRUE(Match(".", "0"));
    EXPECT_TRUE(Match(".", " "));
    EXPECT_FALSE(Match(".", "\n"));  // 鐐瑰彿涓嶅尮閰嶆崲琛岀�?
    EXPECT_FALSE(Match(".", ""));
}

// ============================================================================
// 杞箟搴忓垪鍖归厤娴嬭瘯
// ============================================================================

/**
 * @test 娴嬭瘯\d杞箟搴忓垪锛堟暟瀛楋級鍖归厤
 */
TEST_F(RegExpNFATest, DigitEscapeMatch) {
    EXPECT_TRUE(Match("\\d", "0"));
    EXPECT_TRUE(Match("\\d", "5"));
    EXPECT_TRUE(Match("\\d", "9"));
    EXPECT_FALSE(Match("\\d", "a"));
    EXPECT_FALSE(Match("\\d", " "));
}

/**
 * @test 娴嬭瘯\D杞箟搴忓垪锛堥潪鏁板瓧锛夊尮閰?
 */
TEST_F(RegExpNFATest, NotDigitEscapeMatch) {
    EXPECT_FALSE(Match("\\D", "0"));
    EXPECT_FALSE(Match("\\D", "9"));
    EXPECT_TRUE(Match("\\D", "a"));
    EXPECT_TRUE(Match("\\D", " "));
}

/**
 * @test 娴嬭瘯\w杞箟搴忓垪锛堝崟璇嶅瓧绗︼級鍖归厤
 */
TEST_F(RegExpNFATest, WordEscapeMatch) {
    EXPECT_TRUE(Match("\\w", "a"));
    EXPECT_TRUE(Match("\\w", "Z"));
    EXPECT_TRUE(Match("\\w", "0"));
    EXPECT_TRUE(Match("\\w", "_"));
    EXPECT_FALSE(Match("\\w", " "));
    EXPECT_FALSE(Match("\\w", "-"));
}

/**
 * @test 娴嬭瘯\W杞箟搴忓垪锛堥潪鍗曡瘝瀛楃锛夊尮�?
 */
TEST_F(RegExpNFATest, NotWordEscapeMatch) {
    EXPECT_FALSE(Match("\\W", "a"));
    EXPECT_FALSE(Match("\\W", "0"));
    EXPECT_FALSE(Match("\\W", "_"));
    EXPECT_TRUE(Match("\\W", " "));
    EXPECT_TRUE(Match("\\W", "-"));
}

/**
 * @test 娴嬭瘯\s杞箟搴忓垪锛堢┖鐧藉瓧绗︼級鍖归厤
 */
TEST_F(RegExpNFATest, SpaceEscapeMatch) {
    EXPECT_TRUE(Match("\\s", " "));
    EXPECT_TRUE(Match("\\s", "\t"));
    EXPECT_TRUE(Match("\\s", "\n"));
    EXPECT_TRUE(Match("\\s", "\r"));
    EXPECT_FALSE(Match("\\s", "a"));
    EXPECT_FALSE(Match("\\s", "0"));
}

/**
 * @test 娴嬭瘯\S杞箟搴忓垪锛堥潪绌虹櫧瀛楃锛夊尮�?
 */
TEST_F(RegExpNFATest, NotSpaceEscapeMatch) {
    EXPECT_FALSE(Match("\\S", " "));
    EXPECT_FALSE(Match("\\S", "\t"));
    EXPECT_TRUE(Match("\\S", "a"));
    EXPECT_TRUE(Match("\\S", "0"));
}

// ============================================================================
// 閲忚瘝鍖归厤娴嬭�?
// ============================================================================

/**
 * @test 娴嬭�?閲忚瘝锛?娆℃垨澶氭锛夊尮閰?
 */
TEST_F(RegExpNFATest, StarQuantifierMatch) {
    EXPECT_TRUE(Match("a*", ""));
    EXPECT_TRUE(Match("a*", "a"));
    EXPECT_TRUE(Match("a*", "aaa"));
    EXPECT_FALSE(Match("a*", "b"));

    EXPECT_TRUE(Match("[0-9]*", ""));
    EXPECT_TRUE(Match("[0-9]*", "123"));
    EXPECT_TRUE(Match("[0-9]*", "456"));
}

/**
 * @test 娴嬭�?閲忚瘝锛?娆℃垨澶氭锛夊尮閰?
 */
TEST_F(RegExpNFATest, PlusQuantifierMatch) {
    EXPECT_FALSE(Match("a+", ""));
    EXPECT_TRUE(Match("a+", "a"));
    EXPECT_TRUE(Match("a+", "aaa"));
    EXPECT_FALSE(Match("a+", "b"));

    EXPECT_FALSE(Match("[0-9]+", ""));
    EXPECT_TRUE(Match("[0-9]+", "123"));
}

/**
 * @test 娴嬭�?閲忚瘝锛?娆℃�?娆★級鍖归厤
 */
TEST_F(RegExpNFATest, QuestionQuantifierMatch) {
    EXPECT_TRUE(Match("a?", ""));
    EXPECT_TRUE(Match("a?", "a"));
    EXPECT_FALSE(Match("a?", "aa"));
    EXPECT_FALSE(Match("a?", "b"));
}

/**
 * @test 娴嬭瘯閲忚瘝缁勫悎鍖归厤
 */
TEST_F(RegExpNFATest, CombinedQuantifiersMatch) {
    EXPECT_TRUE(Match("a*b?", ""));
    EXPECT_TRUE(Match("a*b?", "a"));
    EXPECT_TRUE(Match("a*b?", "b"));
    EXPECT_TRUE(Match("a*b?", "ab"));
    EXPECT_TRUE(Match("a*b?", "aaa"));
    EXPECT_TRUE(Match("a*b?", "aaab"));

    EXPECT_TRUE(Match("[a-z]+[0-9]*", "abc"));
    EXPECT_TRUE(Match("[a-z]+[0-9]*", "abc123"));
    EXPECT_TRUE(Match("[a-z]+[0-9]*", "abc123456"));
}

// ============================================================================
// 閫夋嫨鍖归厤娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯鍩烘湰閫夋嫨鍖归厤
 */
TEST_F(RegExpNFATest, BasicAlternationMatch) {
    EXPECT_TRUE(Match("a|b", "a"));
    EXPECT_TRUE(Match("a|b", "b"));
    EXPECT_FALSE(Match("a|b", "c"));

    EXPECT_TRUE(Match("cat|dog", "cat"));
    EXPECT_TRUE(Match("cat|dog", "dog"));
    EXPECT_FALSE(Match("cat|dog", "bird"));
}

/**
 * @test 娴嬭瘯澶氫釜閫夋嫨鍖归厤
 */
TEST_F(RegExpNFATest, MultipleAlternationMatch) {
    EXPECT_TRUE(Match("a|b|c", "a"));
    EXPECT_TRUE(Match("a|b|c", "b"));
    EXPECT_TRUE(Match("a|b|c", "c"));
    EXPECT_FALSE(Match("a|b|c", "d"));
}

/**
 * @test 娴嬭瘯閫夋嫨涓庨噺璇嶇粍鍚堝尮閰?
 */
TEST_F(RegExpNFATest, AlternationWithQuantifiersMatch) {
    EXPECT_TRUE(Match("a+|b+", "a"));
    EXPECT_TRUE(Match("a+|b+", "aaa"));
    EXPECT_TRUE(Match("a+|b+", "b"));
    EXPECT_TRUE(Match("a+|b+", "bbb"));
    EXPECT_FALSE(Match("a+|b+", ""));
}

// ============================================================================
// 鍒嗙粍鍖归厤娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯鍩烘湰鍒嗙粍鍖归厤
 */
TEST_F(RegExpNFATest, BasicGroupMatch) {
    EXPECT_TRUE(Match("(ab)", "ab"));
    EXPECT_FALSE(Match("(ab)", "a"));
    EXPECT_FALSE(Match("(ab)", "b"));
}

/**
 * @test 娴嬭瘯鍒嗙粍涓庨噺璇嶅尮�?
 */
TEST_F(RegExpNFATest, GroupWithQuantifiersMatch) {
    EXPECT_TRUE(Match("(ab)+", "ab"));
    EXPECT_TRUE(Match("(ab)+", "abab"));
    EXPECT_TRUE(Match("(ab)+", "ababab"));
    EXPECT_FALSE(Match("(ab)+", "a"));

    EXPECT_TRUE(Match("(ab)*", ""));
    EXPECT_TRUE(Match("(ab)*", "ab"));
    EXPECT_TRUE(Match("(ab)*", "abab"));
}

/**
 * @test 娴嬭瘯澶氫釜鍒嗙粍鍖归厤
 */
TEST_F(RegExpNFATest, MultipleGroupsMatch) {
    EXPECT_TRUE(Match("(a)(b)", "ab"));
    EXPECT_FALSE(Match("(a)(b)", "ba"));

    EXPECT_TRUE(Match("(a+)(b+)", "aaabbb"));
    EXPECT_TRUE(Match("(a+)(b+)", "ab"));
    EXPECT_FALSE(Match("(a+)(b+)", "a"));
}

// ============================================================================
// 澶嶆潅妯″紡鍖归厤娴嬭瘯
// ============================================================================

/**
 * @test 娴嬭瘯閭妯″紡鍖归�?
 */
TEST_F(RegExpNFATest, EmailPatternMatch) {
    EXPECT_TRUE(Match("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}",
                       "user@example.com"));
    EXPECT_TRUE(Match("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}",
                       "user.name@example.com"));
    EXPECT_TRUE(Match("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}",
                       "user+tag@example.co.uk"));
    EXPECT_FALSE(Match("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}",
                        "user@example"));
}

/**
 * @test 娴嬭瘯URL妯″紡鍖归�?
 */
TEST_F(RegExpNFATest, URLPatternMatch) {
    EXPECT_TRUE(Match("https?://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(/\\S*)?",
                       "http://example.com"));
    EXPECT_TRUE(Match("https?://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(/\\S*)?",
                       "https://example.com"));
    EXPECT_TRUE(Match("https?://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(/\\S*)?",
                       "https://example.com/path"));
    EXPECT_TRUE(Match("https?://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(/\\S*)?",
                       "https://example.com/path/to/resource"));
}

/**
 * @test 娴嬭瘯IP鍦板潃妯″紡鍖归�?
 */
TEST_F(RegExpNFATest, IPAddressPatternMatch) {
    EXPECT_TRUE(Match("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}",
                       "192.168.1.1"));
    EXPECT_TRUE(Match("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}",
                       "10.0.0.1"));
    EXPECT_TRUE(Match("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}",
                       "255.255.255.255"));
}

/**
 * @test 娴嬭瘯鍗佸叚杩涘埗棰滆壊妯″紡鍖归�?
 */
TEST_F(RegExpNFATest, HexColorPatternMatch) {
    EXPECT_TRUE(Match("#[0-9a-fA-F]{6}", "#ffffff"));
    EXPECT_TRUE(Match("#[0-9a-fA-F]{6}", "#000000"));
    EXPECT_TRUE(Match("#[0-9a-fA-F]{6}", "#ff00ff"));
    EXPECT_TRUE(Match("#[0-9a-fA-F]{6}", "#123ABC"));
    EXPECT_FALSE(Match("#[0-9a-fA-F]{6}", "#gggggg"));
}

/**
 * @test 娴嬭瘯鏃ユ湡妯″紡鍖归�?
 */
TEST_F(RegExpNFATest, DatePatternMatch) {
    EXPECT_TRUE(Match("\\d{4}-\\d{2}-\\d{2}", "2025-01-15"));
    EXPECT_TRUE(Match("\\d{4}-\\d{2}-\\d{2}", "1999-12-31"));
    EXPECT_TRUE(Match("\\d{4}-\\d{2}-\\d{2}", "2025-01-01"));
    EXPECT_FALSE(Match("\\d{4}-\\d{2}-\\d{2}", "2025-1-1"));
}

// ============================================================================
// 杈圭晫鎯呭喌娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯绌哄瓧绗︿覆鍖归厤
 */
TEST_F(RegExpNFATest, EmptyStringMatch) {
    EXPECT_TRUE(Match("", ""));
    EXPECT_FALSE(Match("", "a"));
}

/**
 * @test 娴嬭瘯绌烘ā寮忓尮�?
 */
TEST_F(RegExpNFATest, EmptyPatternMatch) {
    EXPECT_TRUE(Match("", ""));
    EXPECT_TRUE(Match("", ""));
    EXPECT_TRUE(Match("a*", ""));
}

/**
 * @test 娴嬭瘯鐗规畩瀛楃鍖归厤
 */
TEST_F(RegExpNFATest, SpecialCharacterMatch) {
    EXPECT_TRUE(Match("\\.", "."));
    EXPECT_TRUE(Match("\\*", "*"));
    EXPECT_TRUE(Match("\\+", "+"));
    EXPECT_TRUE(Match("\\?", "?"));
    EXPECT_TRUE(Match("\\$", "$"));
    EXPECT_TRUE(Match("\\^", "^"));
    EXPECT_TRUE(Match("\\|", "|"));
    EXPECT_TRUE(Match("\\[", "["));
    EXPECT_TRUE(Match("\\]", "]"));
    EXPECT_TRUE(Match("\\(", "("));
    EXPECT_TRUE(Match("\\)", ")"));
    EXPECT_TRUE(Match("\\{", "{"));
    EXPECT_TRUE(Match("\\}", "}"));
    EXPECT_TRUE(Match("\\\\", "\\"));
}

TEST_F(RegExpNFATest, BareBracesAreLiteralsWithoutUnicodeMode) {
    EXPECT_TRUE(Match("a{", "a{"));
    EXPECT_TRUE(Match("a}", "a}"));
}

// ============================================================================
// 鎼滅储鍔熻兘娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯鎼滅储鍔熻�?
 */
TEST_F(RegExpNFATest, SearchFunctionality) {
    auto result1 = Search("abc", "abc");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    auto result2 = Search("abc", "xyzabcdef");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 6);  // 鎵惧�?abc"锛岀粨鏉熶綅缃�?

    auto result3 = Search("abc", "xyz");
    EXPECT_FALSE(result3.has_value());
}

/**
 * @test 娴嬭瘯甯﹁捣濮嬩綅缃殑鎼滅�?
 */
TEST_F(RegExpNFATest, SearchWithStartPosition) {
    auto result1 = Search("abc", "abcabcabc", 0);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    auto result2 = Search("abc", "abcabcabc", 3);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 6);

    auto result3 = Search("abc", "abcabcabc", 6);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(*result3, 9);

    auto result4 = Search("abc", "abcabcabc", 9);
    EXPECT_FALSE(result4.has_value());
}

// ============================================================================
// 鏂板杞箟搴忓垪娴嬭瘯锛圽xHH, \uHHHH, \0鍏繘鍒讹級
// ============================================================================

/**
 * @test 娴嬭瘯鍗佸叚杩涘埗杞箟搴忓�?\xHH
 */
TEST_F(RegExpNFATest, HexEscapeSequence) {
    EXPECT_TRUE(Match("\\x41", "A"));   // \x41 = 'A'
    EXPECT_TRUE(Match("\\x61", "a"));   // \x61 = 'a'
    EXPECT_TRUE(Match("\\x48\\x65\\x6c\\x6c\\x6f", "Hello"));  // "Hello"
}

/**
 * @test 娴嬭瘯Unicode杞箟搴忓垪 \uHHHH
 */
TEST_F(RegExpNFATest, UnicodeEscapeSequence) {
    EXPECT_TRUE(Match("\\u0041", "A"));   // \u0041 = 'A'
    EXPECT_TRUE(Match("\\u0061", "a"));   // \u0061 = 'a'
    EXPECT_TRUE(Match("\\u0048\\u0065\\u006c\\u006c\\u006f", "Hello"));  // "Hello"
}

/**
 * @test 娴嬭瘯鍏繘鍒惰浆涔夊簭�?\0
 */
TEST_F(RegExpNFATest, OctalEscapeSequence) {
    EXPECT_TRUE(Match("\\0101", "A"));   // 鍏繘鍒?01 = 'A'
    EXPECT_TRUE(Match("\\0141", "a"));   // 鍏繘鍒?41 = 'a'
    EXPECT_TRUE(Match("\\01", "\1"));      // 鍏繘鍒?
}

// ============================================================================
// 鍚庣灮鏂█娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯姝ｅ悜鍚庣灮鏂█
 */
TEST_F(RegExpNFATest, PositiveLookbehind) {
    // (?<=abc)def 鍖归厤鍓嶉潰鏈塧bc鐨刣ef
    // 浣跨敤Search鍑芥暟娴嬭瘯閮ㄥ垎鍖归厤
    auto result1 = Search("(?<=abc)def", "abcdef");
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 6);  // 鍖归厤鍒癲ef锛岀粨鏉熶綅缃�?

    auto result2 = Search("(?<=abc)def", "xyzdef");
    EXPECT_FALSE(result2.has_value());  // 鍓嶉潰娌℃湁abc锛屼笉鍖归厤
}

/**
 * @test 娴嬭瘯璐熷悜鍚庣灮鏂█
 */
TEST_F(RegExpNFATest, NegativeLookbehind) {
    // (?<!abc)def 鍖归厤鍓嶉潰娌℃湁abc鐨刣ef
    // 浣跨敤Search鍑芥暟娴嬭瘯閮ㄥ垎鍖归厤
    auto result1 = Search("(?<!abc)def", "xyzdef");
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 6);  // 鍓嶉潰娌℃湁abc锛屽尮閰嶆垚�?

    auto result2 = Search("(?<!abc)def", "abcdef");
    EXPECT_FALSE(result2.has_value());  // 鍓嶉潰鏈塧bc锛屼笉鍖归厤
}

/**
 * @test 娴嬭瘯鍚庣灮鏂█鍦ㄥ鏉傛ā寮忎�?
 */
TEST_F(RegExpNFATest, LookbehindInComplexPattern) {
    // (?<=a)b 鍖归厤鍓嶉潰鏈塧鐨刡
    // 浣跨敤Search鍑芥暟娴嬭瘯閮ㄥ垎鍖归厤
    auto result1 = Search("(?<=a)b", "abc");
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 2);  // 鍖归厤鍒癰锛岀粨鏉熶綅缃�?

    auto result2 = Search("(?<=a)b", "xbc");
    EXPECT_FALSE(result2.has_value());  // 鍓嶉潰娌℃湁a锛屼笉鍖归厤
}

// ============================================================================
// 鍛藉悕鎹曡幏缁勬祴璇?
// ============================================================================

/**
 * @test 娴嬭瘯鍛藉悕鎹曡幏缁?
 */
TEST_F(RegExpNFATest, NamedCaptureGroup) {
    // (?<name>abc) 鍖归�?abc"骞舵崟鑾峰埌鍚嶄�?name"鐨勭�?
    EXPECT_TRUE(Match("(?<name>abc)", "abc"));

    // 娴嬭瘯鍛藉悕鍙嶅悜寮曠敤
    EXPECT_TRUE(Match("(?<name>abc)\\k<name>", "abcabc"));
    EXPECT_FALSE(Match("(?<name>abc)\\k<name>", "abcxyz"));
}

/**
 * @test 娴嬭瘯澶氫釜鍛藉悕鎹曡幏�?
 */
TEST_F(RegExpNFATest, MultipleNamedCaptureGroups) {
    // (?<first>a)(?<second>b)\k<first>\k<second>
    EXPECT_TRUE(Match("(?<first>a)(?<second>b)\\k<first>\\k<second>", "abab"));
}

/**
 * @test 娴嬭瘯鍛藉悕鎹曡幏缁勫拰鏅€氭崟鑾风粍娣峰悎浣跨敤
 */
TEST_F(RegExpNFATest, MixedNamedAndNumberedCaptureGroups) {
    // (?<name>a)(b)\2\1 - 鎹曡幏缁?(name)=a, 鎹曡幏缁?=b, \2寮曠敤缁?(b), \1寮曠敤缁?(a)
    EXPECT_TRUE(Match("(?<name>a)(b)\\2\\1", "abba"));
}

TEST_F(RegExpNFATest, UnmatchedBackreferenceMatchesEmptyString) {
    EXPECT_TRUE(Match("(a)?\\1", ""));
    EXPECT_TRUE(Match("(a)?\\1", "aa"));
    EXPECT_TRUE(Match("\\1(a)", "a"));
}

TEST_F(RegExpNFATest, ForwardNamedBackreferenceMatchesFutureCapture) {
    EXPECT_TRUE(Match("\\k<name>(?<name>a)", "a"));
}

// ============================================================================
// Unicode 鐮佺偣杞箟娴嬭�?
// ============================================================================

/**
 * @test 娴嬭�?Unicode 鐮佺偣杞箟 \u{HHHHHH}
 */
TEST_F(RegExpNFATest, UnicodeCodePointEscape) {
    // \u{41} 鍖归�?'A'
    EXPECT_TRUE(MatchWithFlags("\\u{41}", "A", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\u{41}", "B", false, false, false, true));

    // \u{61} 鍖归�?'a'
    EXPECT_TRUE(MatchWithFlags("\\u{61}", "a", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\u{61}", "b", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 鐮佺偣杞箟鍦ㄥ鏉傛ā寮忎腑
 */
TEST_F(RegExpNFATest, UnicodeCodePointEscapeInPattern) {
    // \u{41}\u{42}\u{43} 鍖归�?"ABC"
    EXPECT_TRUE(MatchWithFlags("\\u{41}\\u{42}\\u{43}", "ABC", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\u{41}\\u{42}\\u{43}", "ABD", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 鐮佺偣杞箟涓庡瓧绗︾被缁撳�?
 */
TEST_F(RegExpNFATest, UnicodeCodePointEscapeWithCharClass) {
    // [\u{41}-\u{5A}] 鍖归厤澶у啓瀛楁�?A-Z
    EXPECT_TRUE(MatchWithFlags("[\\u{41}-\\u{5A}]", "A", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("[\\u{41}-\\u{5A}]", "Z", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("[\\u{41}-\\u{5A}]", "a", false, false, false, true));
}

// ============================================================================
// Unicode 灞炴€ц浆涔夋祴璇?
// ============================================================================

/**
 * @test 娴嬭�?Unicode 灞炴€ц浆�?\p{L}锛堝瓧姣嶏級
 */
TEST_F(RegExpNFATest, UnicodePropertyLetter) {
    // \p{L} 鍖归厤瀛楁�?
    EXPECT_TRUE(MatchWithFlags("\\p{L}", "a", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\p{L}", "Z", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{L}", "0", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{L}", " ", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 灞炴€ц浆�?\P{L}锛堥潪瀛楁瘝锛?
 */
TEST_F(RegExpNFATest, UnicodePropertyNotLetter) {
    // \P{L} 鍖归厤闈炲瓧�?
    EXPECT_FALSE(MatchWithFlags("\\P{L}", "a", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\P{L}", "Z", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\P{L}", "0", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\P{L}", " ", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 灞炴€ц浆�?\p{N}锛堟暟瀛楋�?
 */
TEST_F(RegExpNFATest, UnicodePropertyNumber) {
    // \p{N} 鍖归厤鏁板瓧
    EXPECT_TRUE(MatchWithFlags("\\p{N}", "0", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\p{N}", "9", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{N}", "a", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{N}", " ", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 灞炴€ц浆�?\p{Lu}锛堝ぇ鍐欏瓧姣嶏�?
 */
TEST_F(RegExpNFATest, UnicodePropertyUppercaseLetter) {
    // \p{Lu} 鍖归厤澶у啓瀛楁�?
    EXPECT_TRUE(MatchWithFlags("\\p{Lu}", "A", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\p{Lu}", "Z", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{Lu}", "a", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{Lu}", "0", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 灞炴€ц浆�?\p{Ll}锛堝皬鍐欏瓧姣嶏�?
 */
TEST_F(RegExpNFATest, UnicodePropertyLowercaseLetter) {
    // \p{Ll} 鍖归厤灏忓啓瀛楁�?
    EXPECT_TRUE(MatchWithFlags("\\p{Ll}", "a", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\p{Ll}", "z", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{Ll}", "A", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{Ll}", "0", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 灞炴€ц浆�?\p{P}锛堟爣鐐圭鍙凤�?
 */
TEST_F(RegExpNFATest, UnicodePropertyPunctuation) {
    // \p{P} 鍖归厤鏍囩偣绗﹀�?
    EXPECT_TRUE(MatchWithFlags("\\p{P}", "!", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\p{P}", "?", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{P}", "a", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{P}", "0", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 灞炴€ц浆涔夊湪澶嶆潅妯″紡�?
 */
TEST_F(RegExpNFATest, UnicodePropertyInComplexPattern) {
    // \p{L}+ 鍖归厤涓€涓垨澶氫釜瀛楁�?
    EXPECT_TRUE(MatchWithFlags("\\p{L}+", "abc", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\p{L}+", "ABC", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{L}+", "123", false, false, false, true));

    // \p{N}{3} 鍖归�?涓暟�?
    EXPECT_TRUE(MatchWithFlags("\\p{N}{3}", "123", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{N}{3}", "12", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("\\p{N}{3}", "abc", false, false, false, true));
}

/**
 * @test 娴嬭�?Unicode 灞炴€ц浆涔変笌瀛楃绫荤粨�?
 */
TEST_F(RegExpNFATest, UnicodePropertyWithCharacterClass) {
    // [\p{L}\p{N}] 鍖归厤瀛楁瘝鎴栨暟�?
    EXPECT_TRUE(MatchWithFlags("[\\p{L}\\p{N}]", "a", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("[\\p{L}\\p{N}]", "0", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("[\\p{L}\\p{N}]", "!", false, false, false, true));
}

TEST_F(RegExpNFATest, UnicodePropertyEscapesRequireUnicodeMode) {
    EXPECT_TRUE(MatchWithFlags("\\p{L}", "p{L}"));
    EXPECT_FALSE(MatchWithFlags("\\p{L}", "A"));
    EXPECT_TRUE(MatchWithFlags("\\p{L}", "A", false, false, false, true));
}

// ============================================================================
// 杈圭晫鏂█娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯瀛楃涓插紑澶存柇瑷�?^
 */
TEST_F(RegExpNFATest, CaretAnchor) {
    EXPECT_TRUE(Match("^abc", "abc"));

    auto result1 = Search("^abc", "abcdef");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    EXPECT_FALSE(Search("^abc", "xyzabc").has_value());
    EXPECT_FALSE(Search("^abc", "xyzabcdef").has_value());
}

/**
 * @test 娴嬭瘯瀛楃涓茬粨灏炬柇瑷�?$
 */
TEST_F(RegExpNFATest, DollarAnchor) {
    EXPECT_TRUE(Match("abc$", "abc"));

    auto result1 = Search("abc$", "xyzabc");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 6);

    EXPECT_FALSE(Search("abc$", "abcdef").has_value());
    EXPECT_FALSE(Search("abc$", "abcxyz").has_value());
}

/**
 * @test 娴嬭瘯璇嶈竟�?\b
 */
TEST_F(RegExpNFATest, WordBoundary) {
    EXPECT_TRUE(Match("\\bcat\\b", "cat"));

    auto result1 = Search("\\bcat\\b", "cat dog");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    EXPECT_FALSE(Search("\\bcat\\b", "category").has_value());
    EXPECT_FALSE(Search("\\bcat\\b", "scat").has_value());
    EXPECT_FALSE(Search("\\bcat\\b", "cats").has_value());
}

/**
 * @test 娴嬭瘯闈炶瘝杈圭�?\B
 */
TEST_F(RegExpNFATest, NotWordBoundary) {
    auto result1 = Search("cat\\B", "category");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    auto result2 = Search("cat\\B", "cats");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 3);

    auto result3 = Search("\\Bcat\\B", "scatology");
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(*result3, 4);

    EXPECT_FALSE(Search("cat\\B", "cat").has_value());
    EXPECT_FALSE(Search("cat\\B", "cat dog").has_value());
}

/**
 * @test 娴嬭瘯杈圭晫鏂█缁勫悎
 */
TEST_F(RegExpNFATest, BoundaryAnchorsCombination) {
    EXPECT_TRUE(Match("^cat$", "cat"));
    EXPECT_FALSE(Match("^cat$", "cat "));
    EXPECT_FALSE(Match("^cat$", " cat"));
    EXPECT_FALSE(Match("^cat$", "cats"));
}

// ============================================================================
// 鍓嶇灮鏂█娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯姝ｅ悜鍓嶇�?(?=...)
 */
TEST_F(RegExpNFATest, PositiveLookahead) {
    auto result1 = Search("a(?=b)", "ab");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 1);

    auto result2 = Search("a(?=b)", "abc");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 1);

    EXPECT_FALSE(Search("a(?=b)", "ac").has_value());
    EXPECT_FALSE(Search("a(?=b)", "a").has_value());
}

/**
 * @test 娴嬭瘯璐熷悜鍓嶇�?(?!...)
 */
TEST_F(RegExpNFATest, NegativeLookahead) {
    auto result1 = Search("a(?!b)", "ac");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 1);

    auto result2 = Search("a(?!b)", "a");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 1);

    EXPECT_FALSE(Search("a(?!b)", "ab").has_value());
    EXPECT_FALSE(Search("a(?!b)", "abc").has_value());
}

/**
 * @test 娴嬭瘯鍓嶇灮鏂█鍦ㄥ鏉傛ā寮忎�?
 */
TEST_F(RegExpNFATest, LookaheadInComplexPattern) {
    auto result1 = Search("\\d+(?=\\.\\d)", "123.456");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    EXPECT_FALSE(Search("\\d+(?=\\.\\d)", "123").has_value());
    EXPECT_FALSE(Search("\\d+(?=\\.\\d)", "abc").has_value());
}

/**
 * @test 娴嬭瘯澶氫釜鍓嶇灮鏂█
 */
TEST_F(RegExpNFATest, MultipleLookaheads) {
    EXPECT_FALSE(Search("a(?=b)(?=c)", "abc").has_value());
    EXPECT_FALSE(Search("a(?=b)(?=c)", "abd").has_value());

    auto result = Search("a(?=bc)(?=b)", "abc");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 1);
}

// ============================================================================
// 澶氳妯″紡娴嬭瘯锛坢鏍囧織�?
// ============================================================================

/**
 * @test 娴嬭瘯澶氳妯″紡涓嬬�?^ 鏂�?
 */
TEST_F(RegExpNFATest, MultilineModeCaret) {
    auto result1 = SearchWithFlags("^abc", "abc\ndef", 0, false, true);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    auto result2 = SearchWithFlags("^abc", "xyz\nabc", 0, false, true);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 7);

    auto result3 = SearchWithFlags("^abc", "123\nabc\n456", 0, false, true);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(*result3, 7);

    EXPECT_FALSE(Search("^abc", "xyz\nabc").has_value());
}

/**
 * @test 娴嬭瘯澶氳妯″紡涓嬬�?$ 鏂�?
 */
TEST_F(RegExpNFATest, MultilineModeDollar) {
    auto result1 = SearchWithFlags("abc$", "abc\ndef", 0, false, true);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    auto result2 = SearchWithFlags("abc$", "def\nabc", 0, false, true);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 7);

    auto result3 = SearchWithFlags("abc$", "123\nabc\n456", 0, false, true);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(*result3, 7);

    EXPECT_FALSE(Search("abc$", "abc\ndef").has_value());
}

/**
 * @test 娴嬭瘯澶氳妯″紡涓嬬殑杈圭晫鏂�?
 */
TEST_F(RegExpNFATest, MultilineModeBoundaries) {
    EXPECT_TRUE(MatchWithFlags("^abc$", "abc", false, true));

    auto result1 = SearchWithFlags("^abc$", "abc\n", 0, false, true);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    auto result2 = SearchWithFlags("^abc$", "\nabc\n", 0, false, true);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 4);
}

// ============================================================================
// dotAll妯″紡娴嬭瘯锛坰鏍囧織�?
// ============================================================================

/**
 * @test 娴嬭瘯dotAll妯″紡涓嬬殑鐐瑰彿鍖归厤鎹㈣�?
 */
TEST_F(RegExpNFATest, DotAllModeNewline) {
    EXPECT_TRUE(MatchWithFlags(".", "\n", false, false, true));
    EXPECT_TRUE(MatchWithFlags(".", "\r", false, false, true));

    auto result = SearchWithFlags(".", "\r\n", 0, false, false, true);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 1);

    EXPECT_FALSE(Match(".", "\n"));
    EXPECT_FALSE(Match(".", "\r"));
}

/**
 * @test 娴嬭瘯dotAll妯″紡涓嬬殑澶嶆潅妯″紡
 */
TEST_F(RegExpNFATest, DotAllModeComplexPattern) {
    EXPECT_TRUE(MatchWithFlags("a.b", "a\nb", false, false, true));
    EXPECT_TRUE(MatchWithFlags("a.b", "a\rb", false, false, true));
    EXPECT_TRUE(MatchWithFlags("a..b", "a\r\nb", false, false, true));
    EXPECT_TRUE(MatchWithFlags("a.*b", "a\n\n\nb", false, false, true));

    EXPECT_FALSE(Match("a.b", "a\nb"));
    EXPECT_FALSE(Match("a.b", "a\rb"));
}

/**
 * @test 娴嬭瘯dotAll妯″紡涓巑ultiline妯″紡缁撳�?
 */
TEST_F(RegExpNFATest, DotAllWithMultiline) {
    // dotAll鍜宮ultiline妯″紡鍙互鍚屾椂浣跨�?
    EXPECT_TRUE(MatchWithFlags("a.b", "a\nb", false, true, true));
    EXPECT_TRUE(MatchWithFlags("^a.*b$", "a\nb", false, true, true));
}

// ============================================================================
// 闈炴崟鑾风粍娴嬭瘯锛??:...)�?
// ============================================================================

/**
 * @test 娴嬭瘯闈炴崟鑾风粍鍩烘湰鍔熻�?
 */
TEST_F(RegExpNFATest, NonCaptureGroupBasic) {
    EXPECT_TRUE(Match("(?:abc)+", "abc"));
    EXPECT_TRUE(Match("(?:abc)+", "abcabc"));
    EXPECT_TRUE(Match("(?:abc)+", "abcabcabc"));
    EXPECT_FALSE(Match("(?:abc)+", "ab"));
}

/**
 * @test 娴嬭瘯闈炴崟鑾风粍涓庨€夋嫨
 */
TEST_F(RegExpNFATest, NonCaptureGroupWithAlternation) {
    EXPECT_TRUE(Match("(?:a|b|c)+", "a"));
    EXPECT_TRUE(Match("(?:a|b|c)+", "ab"));
    EXPECT_TRUE(Match("(?:a|b|c)+", "abc"));
    EXPECT_TRUE(Match("(?:a|b|c)+", "cba"));
}

/**
 * @test 娴嬭瘯闈炴崟鑾风粍涓庨噺�?
 */
TEST_F(RegExpNFATest, NonCaptureGroupWithQuantifiers) {
    EXPECT_TRUE(Match("(?:ab){2}", "abab"));
    EXPECT_TRUE(Match("(?:ab){2,4}", "abab"));
    EXPECT_TRUE(Match("(?:ab){2,4}", "ababab"));
    EXPECT_TRUE(Match("(?:ab){2,4}", "abababab"));
    EXPECT_FALSE(Match("(?:ab){2,4}", "ab"));
    EXPECT_FALSE(Match("(?:ab){2,4}", "ababababab"));
}

/**
 * @test 娴嬭瘯闈炴崟鑾风粍宓屽
 */
TEST_F(RegExpNFATest, NestedNonCaptureGroups) {
    EXPECT_TRUE(Match("(?:a(?:bc)d)+", "abcd"));
    EXPECT_TRUE(Match("(?:a(?:bc)d)+", "abcdabcd"));
}

// ============================================================================
// Unicode妯″紡娴嬭瘯锛坲鏍囧織�?
// ============================================================================

/**
 * @test 娴嬭瘯Unicode妯″紡鍩烘湰鍔熻兘
 */
TEST_F(RegExpNFATest, UnicodeModeBasic) {
    // Unicode妯″紡涓嬶紝搴旇姝ｇ‘澶勭悊Unicode瀛楃�?
    EXPECT_TRUE(MatchWithFlags("\\u{41}", "A", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("\\u{1F600}", "\xF0\x9F\x98\x80", false, false, false, true)); // 馃榾
}

/**
 * @test 娴嬭瘯Unicode妯″紡涓嬬殑瀛楃绫?
 */
TEST_F(RegExpNFATest, UnicodeModeCharacterClass) {
    EXPECT_TRUE(MatchWithFlags("[\\u{41}-\\u{5A}]", "A", false, false, false, true));
    EXPECT_TRUE(MatchWithFlags("[\\u{41}-\\u{5A}]", "Z", false, false, false, true));
    EXPECT_FALSE(MatchWithFlags("[\\u{41}-\\u{5A}]", "a", false, false, false, true));
}

// ============================================================================
// 缁煎悎澶嶆潅妯″紡娴嬭�?
// ============================================================================

/**
 * @test 娴嬭瘯杈圭晫鏂█涓庨噺璇嶇粨鍚?
 */
TEST_F(RegExpNFATest, BoundaryWithQuantifiers) {
    EXPECT_TRUE(Match("^\\d+$", "123"));
    EXPECT_TRUE(Match("^\\w+$", "abc123"));
    EXPECT_FALSE(Match("^\\d+$", "123abc"));
}

/**
 * @test 娴嬭瘯鍓嶇灮鏂█涓庨噺璇嶇粨鍚?
 */
TEST_F(RegExpNFATest, LookaheadWithQuantifiers) {
    auto result1 = Search("\\d+(?=;)", "123;");
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(*result1, 3);

    auto result2 = Search("\\d+(?=;)", "456;");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 3);

    EXPECT_FALSE(Search("\\d+(?=;)", "123").has_value());
}

/**
 * @test 娴嬭瘯澶氳妯″紡鐨勫鏉傚満�?
 */
TEST_F(RegExpNFATest, MultilineModeComplex) {
    auto result = SearchWithFlags("^\\d+", "abc\n123\ndef", 0, false, true);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(*result, 7); // "123"缁撴潫浣嶇疆

    auto result2 = SearchWithFlags("\\d+$", "abc\n123\ndef", 0, false, true);
    EXPECT_TRUE(result2.has_value());
    EXPECT_EQ(*result2, 7); // "123"缁撴潫浣嶇疆
}

/**
 * @test 娴嬭瘯闈炴崟鑾风粍鍦ㄥ鏉傛ā寮忎�?
 */
TEST_F(RegExpNFATest, NonCaptureGroupComplex) {
    // 閭楠岃瘉涓殑闈炴崟鑾风�?
    EXPECT_TRUE(Match("[a-zA-Z0-9._%+-]+@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,}", "user@example.com"));
    EXPECT_TRUE(Match("[a-zA-Z0-9._%+-]+@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,}", "user@mail.example.co.uk"));
}

// ============================================================================
// MatchDetail 带捕获组的Unicode属性测试
// ============================================================================

TEST_F(RegExpNFATest, MatchDetailWithCapturesAndUnicodeProp) {
    RegExpParser parser("(\\p{L}+)", true);
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get(), false, false, false, true);
    auto result = nfa.MatchDetail("abc", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->matched_text, "abc");
    ASSERT_GE(result->captures.size(), 1u);
    EXPECT_EQ(result->captures[0], "abc");
}

TEST_F(RegExpNFATest, MatchDetailWithCapturesAndUnicodeCodePoint) {
    RegExpParser parser("(\\u{41})", true);
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get(), false, false, false, true);
    auto result = nfa.MatchDetail("A", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->matched_text, "A");
    ASSERT_GE(result->captures.size(), 1u);
    EXPECT_EQ(result->captures[0], "A");
}

TEST_F(RegExpNFATest, MatchDetailWithNumberedBackref) {
    RegExpParser parser("(a)\\1");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get());
    EXPECT_TRUE(nfa.Match("aa"));
    EXPECT_FALSE(nfa.Match("ab"));

    auto result = nfa.MatchDetail("aa", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->matched_text, "aa");
    ASSERT_GE(result->captures.size(), 1u);
    EXPECT_EQ(result->captures[0], "a");
}

TEST_F(RegExpNFATest, MatchDetailEmptyCaptureGroup) {
    RegExpParser parser("(a)?b");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get());

    auto result = nfa.MatchDetail("b", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->matched_text, "b");
    ASSERT_GE(result->captures.size(), 1u);
    EXPECT_EQ(result->captures[0], "");
}

TEST_F(RegExpNFATest, MatchDetailNestedCaptures) {
    RegExpParser parser("((a)(b))");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get());

    auto result = nfa.MatchDetail("ab", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->matched_text, "ab");
    ASSERT_GE(result->captures.size(), 3u);
    EXPECT_EQ(result->captures[0], "ab");   // Group 1: (ab)
    EXPECT_EQ(result->captures[1], "a");    // Group 2: (a)
    EXPECT_EQ(result->captures[2], "b");    // Group 3: (b)
}

TEST_F(RegExpNFATest, MatchDetailGreedyAndNonGreedy) {
    // Greedy match: (a+) should match as many 'a's as possible
    RegExpParser greedy_parser("(a+)a*");
    auto greedy_ast = greedy_parser.Parse();
    ASSERT_NE(greedy_ast, nullptr);
    NFA greedy_nfa = NFABuilder::Build(greedy_ast.get());
    auto greedy_result = greedy_nfa.MatchDetail("aaa", 0);
    ASSERT_TRUE(greedy_result.has_value());
    EXPECT_EQ(greedy_result->captures[0], "aaa");

    // Non-greedy match: (a+?) should match as few 'a's as possible
    RegExpParser non_greedy_parser("(a+?)a*");
    auto non_greedy_ast = non_greedy_parser.Parse();
    ASSERT_NE(non_greedy_ast, nullptr);
    NFA non_greedy_nfa = NFABuilder::Build(non_greedy_ast.get());
    auto non_greedy_result = non_greedy_nfa.MatchDetail("aaa", 0);
    ASSERT_TRUE(non_greedy_result.has_value());
    EXPECT_EQ(non_greedy_result->captures[0], "a");
}

TEST_F(RegExpNFATest, MatchDetailWithLookaheadInsideGroup) {
    RegExpParser parser("(a(?=b))");
    auto ast = parser.Parse();
    ASSERT_NE(ast, nullptr);

    NFA nfa = NFABuilder::Build(ast.get());
    auto result = nfa.MatchDetail("ab", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->matched_text, "a");
    ASSERT_GE(result->captures.size(), 1u);
    EXPECT_EQ(result->captures[0], "a");
}

} // namespace test
} // namespace mjs
