#include <mjs/regexp/regexp_parser.h>
#include <mjs/regexp/regexp_nfa.h>
#include <iostream>
using namespace mjs;

int main() {
    RegExpParser parser("");
    auto ast = parser.Parse();
    if (!ast) {
        std::cout << "Parse failed" << std::endl;
        return 1;
    }
    
    NFA nfa = NFABuilder::Build(ast.get());
    
    // Test empty string
    std::string empty_str = "";
    bool result1 = nfa.Match(empty_str);
    std::cout << "Match(\"\", \"\"): " << (result1 ? "true" : "false") << std::endl;
    
    // Test non-empty string
    std::string non_empty = "a";
    bool result2 = nfa.Match(non_empty);
    std::cout << "Match(\"\", \"a\"): " << (result2 ? "true" : "false") << std::endl;
    
    // Test MatchDetail
    auto detail_result = nfa.MatchDetail("a", 0);
    if (detail_result.has_value()) {
        std::cout << "MatchDetail(\"a\", 0): start=" << detail_result->start_pos 
                  << ", end=" << detail_result->end_pos << std::endl;
    } else {
        std::cout << "MatchDetail(\"a\", 0): no match" << std::endl;
    }
    
    return 0;
}
