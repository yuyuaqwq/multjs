/**
 * @file regexp_nfa.cpp
 * @brief 正则表达式 NFA 实现
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <mjs/regexp/regexp_nfa.h>
#include <mjs/regexp/unicode_prop_ranges.h>
#include <cctype>
#include <queue>
#include <cassert>

namespace mjs {

// ==================== NFA ====================

NFA::NFA()
    : start_state_(kInvalidNFAStateId),
      accept_state_(kInvalidNFAStateId),
      capture_count_(0),
      multiline_(false),
      dot_all_(false),
      unicode_(false),
      unicode_sets_(false),
      ignore_case_(false) {}

NFAStateId NFA::CreateState(NFAStateType type) {
    NFAStateId id = states_.size();
    states_.emplace_back(type);
    return id;
}

void NFA::SetCharTransition(NFAStateId from, NFAStateId to, char ch) {
    assert(from < states_.size() && "Invalid from state ID");
    assert(to < states_.size() && "Invalid to state ID");
    auto& state = states_[from];
    state.type = NFAStateType::kChar;
    state.ch = ch;
    state.next1 = to;
}

void NFA::SetCharClassTransition(NFAStateId from, NFAStateId to, bool negated,
                                const std::vector<std::pair<char, char>>& ranges) {
    auto& state = states_[from];
    state.type = NFAStateType::kCharClass;
    state.char_class.negated = negated;
    state.char_class.range_count = std::min(ranges.size(), kCharClassRangeLimit);

    for (size_t i = 0; i < state.char_class.range_count; ++i) {
        state.char_class.ranges[i * 2] = ranges[i].first;
        state.char_class.ranges[i * 2 + 1] = ranges[i].second;
    }
    state.next1 = to;
}

void NFA::SetDotTransition(NFAStateId from, NFAStateId to) {
    auto& state = states_[from];
    state.type = NFAStateType::kDot;
    state.next1 = to;
}

void NFA::SetTypeTransition(NFAStateId from, NFAStateId to, NFAStateType type) {
    auto& state = states_[from];
    state.type = type;
    state.next1 = to;
}

void NFA::SetEpsilonTransition(NFAStateId from, NFAStateId to) {
    assert(from < states_.size() && "Invalid from state ID");
    assert(to < states_.size() && "Invalid to state ID");
    auto& state = states_[from];

    // 不要改变状态类型！只设置转移
    // 对于kMatch状态，我们应该使用epsilon_transitions而不是改变类型

    // epsilon转移有两个主要出口（next1和next2）
    // 如果都已被占用，使用epsilon_transitions vector
    if (state.type == NFAStateType::kMatch || state.type == NFAStateType::kCaptureEnd ||
        state.type == NFAStateType::kLookahead || state.type == NFAStateType::kLookbehind ||
        state.type == NFAStateType::kBoundary || state.type == NFAStateType::kBackref ||
        state.type == NFAStateType::kChar || state.type == NFAStateType::kDot ||
        state.type == NFAStateType::kDigit || state.type == NFAStateType::kNotDigit ||
        state.type == NFAStateType::kWord || state.type == NFAStateType::kNotWord ||
        state.type == NFAStateType::kSpace || state.type == NFAStateType::kNotSpace ||
        state.type == NFAStateType::kCharClass || state.type == NFAStateType::kUnicodeCodePoint ||
        state.type == NFAStateType::kUnicodeProp || state.type == NFAStateType::kNotUnicodeProp) {
        // 对于这些状态，使用epsilon_transitions而不是改变类型
        state.epsilon_transitions.push_back(to);
    } else {
        // 对于epsilon类型的状态，可以安全地改变类型
        if (state.type != NFAStateType::kEpsilon &&
            state.type != NFAStateType::kStar &&
            state.type != NFAStateType::kPlus &&
            state.type != NFAStateType::kQuestion &&
            state.type != NFAStateType::kCaptureStart) {
            state.type = NFAStateType::kEpsilon;
        }

        if (state.next1 == kInvalidNFAStateId) {
            state.next1 = to;
        } else if (state.next2 == kInvalidNFAStateId) {
            state.next2 = to;
        } else {
            state.epsilon_transitions.push_back(to);
        }
    }
}

void NFA::SetBoundaryTransition(NFAStateId from, NFAStateId to, BoundaryType boundary_type) {
    auto& state = states_[from];
    state.type = NFAStateType::kBoundary;
    state.boundary_type = boundary_type;
    state.next1 = to;
}

void NFA::SetLookaheadTransition(NFAStateId from, NFAStateId to, bool positive,
                                 NFAStateId lookahead_start, NFAStateId lookahead_accept) {
    auto& state = states_[from];
    state.type = NFAStateType::kLookahead;
    state.lookahead.positive = positive;
    state.lookahead.lookahead_start = lookahead_start;
    state.lookahead.lookahead_accept = lookahead_accept;
    state.next1 = to;
}

void NFA::SetLookbehindTransition(NFAStateId from, NFAStateId to, bool positive, NFAStateId lookbehind_start, NFAStateId lookbehind_accept) {
    auto& state = states_[from];
    state.type = NFAStateType::kLookbehind;
    state.lookbehind.positive = positive;
    state.lookbehind.lookbehind_start = lookbehind_start;
    state.lookbehind.lookbehind_accept = lookbehind_accept;
    state.next1 = to;
}

void NFA::SetCaptureStartTransition(NFAStateId from, NFAStateId to, uint32_t capture_index) {
    assert(from < states_.size() && "Invalid from state ID");
    assert(to < states_.size() && "Invalid to state ID");
    auto& state = states_[from];
    state.type = NFAStateType::kCaptureStart;
    state.capture.capture_index = capture_index;
    state.next1 = to;
}

void NFA::SetCaptureEndTransition(NFAStateId from, NFAStateId to, uint32_t capture_index) {
    assert(from < states_.size() && "Invalid from state ID");
    assert(to < states_.size() && "Invalid to state ID");
    auto& state = states_[from];
    state.type = NFAStateType::kCaptureEnd;
    state.capture.capture_index = capture_index;
    state.next1 = to;
}

void NFA::SetBackrefTransition(NFAStateId from, NFAStateId to, uint32_t ref_index) {
    auto& state = states_[from];
    state.type = NFAStateType::kBackref;
    state.backref.ref_index = ref_index;
    state.next1 = to;
}

void NFA::SetUnicodeCodePointTransition(NFAStateId from, NFAStateId to, uint32_t code_point) {
    auto& state = states_[from];
    state.type = NFAStateType::kUnicodeCodePoint;
    state.unicode_code_point.code_point = code_point;
    state.next1 = to;
}

void NFA::SetUnicodePropTransition(NFAStateId from, NFAStateId to, bool negated,
                                    const std::vector<std::pair<char, char>>& ranges) {
    auto& state = states_[from];
    state.type = negated ? NFAStateType::kNotUnicodeProp : NFAStateType::kUnicodeProp;
    state.unicode_prop.negated = negated;
    state.unicode_prop.range_count = std::min(ranges.size(), kUnicodePropRangeLimit);

    for (size_t i = 0; i < state.unicode_prop.range_count; ++i) {
        state.unicode_prop.ranges[i * 2] = ranges[i].first;
        state.unicode_prop.ranges[i * 2 + 1] = ranges[i].second;
    }
    state.next1 = to;
}

std::set<NFAStateId> NFA::EpsilonClosure(const std::set<NFAStateId>& states) const {
    std::set<NFAStateId> result;
    for (auto state_id : states) {
        EpsilonClosureRecursive(state_id, result);
    }
    return result;
}

std::set<NFAStateId> NFA::EpsilonClosure(NFAStateId state) const {
    std::set<NFAStateId> result;
    EpsilonClosureRecursive(state, result);
    return result;
}

void NFA::EpsilonClosureRecursive(NFAStateId state, std::set<NFAStateId>& result) const {
    constexpr size_t MAX_RECURSION_DEPTH = 10000;

    // 使用thread_local来避免递归深度检查的开销
    thread_local size_t recursion_depth = 0;

    if (recursion_depth > MAX_RECURSION_DEPTH) {
        return;  // 防止栈溢出
    }

    if (state >= states_.size()) {
        return;
    }

    if (result.find(state) != result.end()) {
        return;  // 已经访问过
    }

    result.insert(state);
    recursion_depth++;

    const auto& s = states_[state];
    // 对于epsilon类型的状态，处理next1和next2转移
    if (s.type == NFAStateType::kEpsilon || s.type == NFAStateType::kStar ||
        s.type == NFAStateType::kPlus || s.type == NFAStateType::kQuestion ||
        s.type == NFAStateType::kCaptureStart || s.type == NFAStateType::kCaptureEnd) {
        if (s.next1 != kInvalidNFAStateId) {
            EpsilonClosureRecursive(s.next1, result);
        }
        if (s.next2 != kInvalidNFAStateId) {
            EpsilonClosureRecursive(s.next2, result);
        }
    }
    // 处理额外的epsilon转移（所有状态类型都可能存在）
    for (auto next_id : s.epsilon_transitions) {
        EpsilonClosureRecursive(next_id, result);
    }

    recursion_depth--;
}

std::set<NFAStateId> NFA::EpsilonWithContext(const std::set<NFAStateId>& states, const std::string& str, size_t pos) const {
    std::set<NFAStateId> result;
    for (auto state_id : states) {
        EpsilonWithContextRecursive(state_id, result, str, pos);
    }
    return result;
}

std::set<NFAStateId> NFA::EpsilonWithContext(NFAStateId state, const std::string& str, size_t pos) const {
    std::set<NFAStateId> result;
    EpsilonWithContextRecursive(state, result, str, pos);
    return result;
}

void NFA::EpsilonWithContextRecursive(NFAStateId state, std::set<NFAStateId>& result, const std::string& str, size_t pos) const {
    constexpr size_t MAX_RECURSION_DEPTH = 10000;

    // 使用thread_local来避免递归深度检查的开销
    thread_local size_t recursion_depth = 0;

    if (recursion_depth > MAX_RECURSION_DEPTH) {
        return;  // 防止栈溢出
    }

    if (state >= states_.size()) {
        return;
    }

    if (result.find(state) != result.end()) {
        return;  // 已经访问过
    }

    const auto& s = states_[state];
    recursion_depth++;

    // 处理边界断言
    if (s.type == NFAStateType::kBoundary) {
        if (MatchBoundary(s, str, pos)) {
            // 边界匹配，可以转移到下一个状态
            if (s.next1 != kInvalidNFAStateId) {
                EpsilonWithContextRecursive(s.next1, result, str, pos);
            }
        }
        // 边界不匹配，不转移到下一个状态
        recursion_depth--;
        return;
    }

    // 处理前瞻断言
    if (s.type == NFAStateType::kLookahead) {
        // 保存递归深度，防止嵌套的 EpsilonWithContext 调用（在 MatchLookahead 内部）
        // 共享 thread_local recursion_depth 导致计数污染
        auto saved_depth = recursion_depth;
        bool matched = MatchLookahead(s, str, pos);
        recursion_depth = saved_depth;
        if (matched) {
            // 前瞻匹配，可以转移到下一个状态
            if (s.next1 != kInvalidNFAStateId) {
                EpsilonWithContextRecursive(s.next1, result, str, pos);
            }
        }
        // 前瞻不匹配，不转移到下一个状态
        recursion_depth--;
        return;
    }

    // 处理后瞻断言
    if (s.type == NFAStateType::kLookbehind) {
        // 保存递归深度，防止嵌套的 EpsilonClosure 调用（在 MatchLookbehind 内部）
        // 共享 thread_local recursion_depth 导致计数污染
        auto saved_depth = recursion_depth;
        bool matched = MatchLookbehind(s, str, pos);
        recursion_depth = saved_depth;
        if (matched) {
            // 后瞻匹配，可以转移到下一个状态
            if (s.next1 != kInvalidNFAStateId) {
                EpsilonWithContextRecursive(s.next1, result, str, pos);
            }
        }
        // 后瞻不匹配，不转移到下一个状态
        recursion_depth--;
        return;
    }

    // 处理量词状态（kStar, kPlus, kQuestion）
    // 这些状态类似epsilon，但转移顺序会影响贪婪/非贪婪行为
    if (s.type == NFAStateType::kStar || s.type == NFAStateType::kPlus ||
        s.type == NFAStateType::kQuestion) {
        // 先处理next1（优先路径）
        if (s.next1 != kInvalidNFAStateId) {
            EpsilonWithContextRecursive(s.next1, result, str, pos);
        }
        // 再处理next2（次优先路径）
        if (s.next2 != kInvalidNFAStateId) {
            EpsilonWithContextRecursive(s.next2, result, str, pos);
        }
        // 处理额外的epsilon转移
        for (auto next_id : s.epsilon_transitions) {
            EpsilonWithContextRecursive(next_id, result, str, pos);
        }
        recursion_depth--;
        return;
    }

    // 处理捕获组标记（kCaptureStart, kCaptureEnd）
    // 这些状态类似epsilon，直接转移
    if (s.type == NFAStateType::kCaptureStart || s.type == NFAStateType::kCaptureEnd) {
        result.insert(state);  // 首先添加当前状态到结果集
        if (s.next1 != kInvalidNFAStateId) {
            EpsilonWithContextRecursive(s.next1, result, str, pos);
        }
        // 处理额外的epsilon转移
        for (auto next_id : s.epsilon_transitions) {
            EpsilonWithContextRecursive(next_id, result, str, pos);
        }
        recursion_depth--;
        return;
    }

    result.insert(state);

    // 对于epsilon状态，处理next1和next2转移
    if (s.type == NFAStateType::kEpsilon) {
        if (s.next1 != kInvalidNFAStateId) {
            EpsilonWithContextRecursive(s.next1, result, str, pos);
        }
        if (s.next2 != kInvalidNFAStateId) {
            EpsilonWithContextRecursive(s.next2, result, str, pos);
        }
    }
    // 处理额外的epsilon转移（所有状态类型都可能存在）
    for (auto next_id : s.epsilon_transitions) {
        EpsilonWithContextRecursive(next_id, result, str, pos);
    }

    recursion_depth--;
}

std::set<NFAStateId> NFA::Move(const std::set<NFAStateId>& states, char ch) const {
    std::set<NFAStateId> result;

    for (auto state_id : states) {
        if (state_id >= states_.size()) {
            continue;
        }

        const auto& state = states_[state_id];

        if (MatchChar(state, ch)) {
            if (state.next1 != kInvalidNFAStateId) {
                result.insert(state.next1);
            }
        }
    }

    return result;
}

bool NFA::Match(const std::string& str) const {
    // 完整匹配：正则必须消费整个字符串
    // 使用 MatchDetail 来支持捕获组和反向引用
    auto result = MatchDetail(str, 0);
    if (result.has_value()) {
        // 检查是否完整匹配（消费了整个字符串）
        return result->end_pos == str.size();
    }
    return false;
}

std::optional<size_t> NFA::MatchEnd(const std::string& str, size_t start_pos) const {
    // 搜索模式：从指定位置开始搜索第一个匹配
    return MatchEnd(str, start_pos, false);
}

std::optional<size_t> NFA::MatchEnd(const std::string& str, size_t start_pos, bool full_match) const {
    if (start_state_ == kInvalidNFAStateId || accept_state_ == kInvalidNFAStateId) {
        return std::nullopt;
    }

    // 如果是搜索模式，尝试从每个位置开始匹配
    size_t search_start = start_pos;
    size_t search_end = full_match ? start_pos : str.size();

    for (size_t pos = search_start; pos <= search_end; ++pos) {
        auto result = TryMatchFrom(str, pos);
        if (result.has_value()) {
            // 在完整匹配模式下，必须消费整个字符串
            if (full_match) {
                if (*result == str.size()) {
                    return *result;
                }
            } else {
                // 搜索模式，只要找到匹配即可
                return *result;
            }
        }

        // 如果是完整匹配模式，只尝试从start_pos开始
        if (full_match) {
            break;
        }
    }

    return std::nullopt;
}

std::optional<size_t> NFA::TryMatchFrom(const std::string& str, size_t pos) const {
    // 从指定位置开始尝试匹配，返回匹配结束的位置（最长的匹配）
    auto current = EpsilonWithContext(start_state_, str, pos);

    std::optional<size_t> last_match_pos;  // 记录最后一次到达accept状态的位置

    // 初始检查：是否可以从开始状态直接到达accept状态（匹配空字符串）
    if (current.find(accept_state_) != current.end()) {
        last_match_pos = pos;
    }

    // 逐字符匹配
    for (size_t i = pos; i < str.size(); ++i) {
        char ch = str[i];
        auto next = Move(current, ch);

        // 计算epsilon闭包（带上下文）
        current = EpsilonWithContext(next, str, i + 1);

        if (current.empty()) {
            break;  // 没有可达状态
        }

        // 检查当前状态集合是否包含接受状态
        if (current.find(accept_state_) != current.end()) {
            last_match_pos = i + 1;  // 更新最后一次匹配的位置
        }
    }

    // 如果我们找到了至少一个匹配，返回最长的
    if (last_match_pos.has_value()) {
        return last_match_pos;
    }

    return std::nullopt;
}

std::optional<NFAMatchResult> NFA::MatchDetail(const std::string& str, size_t start_pos) const {
    // 搜索模式：从指定位置开始搜索第一个匹配
    if (start_state_ == kInvalidNFAStateId || accept_state_ == kInvalidNFAStateId) {
        return std::nullopt;
    }

    // 尝试从每个位置开始匹配
    for (size_t pos = start_pos; pos <= str.size(); ++pos) {
        auto result = TryMatchFromDetail(str, pos);
        if (result.has_value()) {
            return result;
        }
    }

    return std::nullopt;
}

std::optional<NFAMatchResult> NFA::TryMatchFromDetail(const std::string& str, size_t pos) const {
    // 从指定位置开始尝试匹配，返回详细的匹配信息
    // 如果有捕获组，使用带捕获组跟踪的匹配方法
    if (capture_count_ > 0) {
        return TryMatchFromDetailWithCaptures(str, pos);
    }

    auto current = EpsilonWithContext(start_state_, str, pos);

    std::optional<NFAMatchResult> best_match;  // 记录最佳匹配
    size_t best_match_len = 0;

    // 初始检查：是否可以从开始状态直接到达accept状态（匹配空字符串）
    if (current.find(accept_state_) != current.end()) {
        NFAMatchResult result;
        result.start_pos = pos;
        result.end_pos = pos;
        result.matched_text = "";
        best_match = result;
        best_match_len = 0;
    }

    // 逐字符匹配
    for (size_t i = pos; i < str.size(); ++i) {
        char ch = str[i];
        auto next = Move(current, ch);

        if (next.empty()) {
            break;  // 没有可达状态
        }

        // 计算epsilon闭包（带上下文）
        current = EpsilonWithContext(next, str, i + 1);

        // 检查当前状态集合是否包含接受状态
        if (current.find(accept_state_) != current.end()) {
            size_t match_len = i + 1 - pos;
            // 只有当找到更长的匹配时才更新（贪婪匹配）
            if (!best_match.has_value() || match_len > best_match_len) {
                NFAMatchResult result;
                result.start_pos = pos;
                result.end_pos = i + 1;
                result.matched_text = str.substr(pos, match_len);
                best_match = result;
                best_match_len = match_len;
            }
        }
    }

    return best_match;
}

std::optional<NFAMatchResult> NFA::TryMatchFromDetailWithCaptures(const std::string& str, size_t pos) const {
    // 使用深度优先搜索匹配，并跟踪捕获组位置
    struct MatchState {
        NFAStateId state_id;
        size_t str_pos;
        std::vector<std::pair<size_t, size_t>> captures;  // (start, end), end = SIZE_MAX means open
    };


    std::vector<std::pair<size_t, size_t>> initial_captures(capture_count_, {SIZE_MAX, SIZE_MAX});

    std::optional<NFAMatchResult> best_match;
    size_t best_match_len = 0;

    std::vector<MatchState> stack;
    stack.push_back({start_state_, pos, initial_captures});

    // Debug: 打印捕获组数量和accept_state_

    while (!stack.empty()) {
        auto current_state = stack.back();
        stack.pop_back();

        NFAStateId state_id = current_state.state_id;
        size_t str_pos = current_state.str_pos;
        auto captures = current_state.captures;

        // If the accept state is also a kCaptureEnd, update the capture end position first
        if (state_id == accept_state_ && state_id < states_.size()) {
            const auto& accept_state = states_[state_id];
            if (accept_state.type == NFAStateType::kCaptureEnd) {
                uint32_t cap_idx = accept_state.capture.capture_index - 1;
                if (cap_idx < captures.size()) {
                    captures[cap_idx].second = str_pos;
                }
            }
        }

        // 检查是否到达接受状态
        if (state_id == accept_state_) {
            size_t match_len = str_pos - pos;
            if (!best_match.has_value() || match_len > best_match_len) {
                NFAMatchResult result;
                result.start_pos = pos;
                result.end_pos = str_pos;
                result.matched_text = str.substr(pos, match_len);
                result.captures.clear();
                for (const auto& [start, end] : captures) {
                    if (start != SIZE_MAX && end != SIZE_MAX) {
                        result.captures.push_back(str.substr(start, end - start));
                    } else {
                        result.captures.push_back("");
                    }
                }
                result.capture_indices = captures;
                best_match = result;
                best_match_len = match_len;
            }
            continue;
        }

        if (state_id >= states_.size()) {
            continue;
        }

        const auto& state = states_[state_id];

        // 根据状态类型处理
        switch (state.type) {
            case NFAStateType::kMatch:
            case NFAStateType::kEpsilon:
            case NFAStateType::kStar:
            case NFAStateType::kPlus:
            case NFAStateType::kQuestion: {
                // Epsilon-like transitions - add next states without consuming input.
                // Push epsilon_transitions in reverse so the first-added transition
                // is processed first (stack is LIFO, so reversing compensates).
                if (state.next1 != kInvalidNFAStateId) {
                    stack.push_back({state.next1, str_pos, captures});
                }
                if (state.next2 != kInvalidNFAStateId) {
                    stack.push_back({state.next2, str_pos, captures});
                }
                for (auto it = state.epsilon_transitions.rbegin();
                     it != state.epsilon_transitions.rend(); ++it) {
                    stack.push_back({*it, str_pos, captures});
                }
                break;
            }

            case NFAStateType::kCaptureStart: {
                uint32_t cap_idx = state.capture.capture_index - 1;
                if (cap_idx >= captures.size()) {
                    // Invalid capture index, skip
                    break;
                }
                auto new_captures = captures;
                new_captures[cap_idx].first = str_pos;  // Set capture start
                if (state.next1 != kInvalidNFAStateId) {
                    stack.push_back({state.next1, str_pos, new_captures});
                }
                // 处理epsilon_transitions
                for (auto next_id : state.epsilon_transitions) {
                    stack.push_back({next_id, str_pos, new_captures});
                }
                break;
            }

            case NFAStateType::kCaptureEnd: {
                uint32_t cap_idx = state.capture.capture_index - 1;
                if (cap_idx >= captures.size()) {
                    // Invalid capture index, skip
                    break;
                }
                auto new_captures = captures;
                new_captures[cap_idx].second = str_pos;  // Set capture end
                if (state.next1 != kInvalidNFAStateId) {
                    stack.push_back({state.next1, str_pos, new_captures});
                }
                // 处理epsilon_transitions
                for (auto next_id : state.epsilon_transitions) {
                    stack.push_back({next_id, str_pos, new_captures});
                }
                break;
            }

            case NFAStateType::kBoundary: {
                if (!MatchBoundary(state, str, str_pos)) {
                    break;
                }
                if (state.next1 != kInvalidNFAStateId) {
                    stack.push_back({state.next1, str_pos, captures});
                }
                break;
            }

            case NFAStateType::kLookahead: {
                if (!MatchLookahead(state, str, str_pos)) {
                    break;
                }
                if (state.next1 != kInvalidNFAStateId) {
                    stack.push_back({state.next1, str_pos, captures});
                }
                break;
            }

            case NFAStateType::kLookbehind: {
                if (!MatchLookbehind(state, str, str_pos)) {
                    break;
                }
                if (state.next1 != kInvalidNFAStateId) {
                    stack.push_back({state.next1, str_pos, captures});
                }
                break;
            }

            case NFAStateType::kChar:
            case NFAStateType::kDot:
            case NFAStateType::kDigit:
            case NFAStateType::kNotDigit:
            case NFAStateType::kWord:
            case NFAStateType::kNotWord:
            case NFAStateType::kSpace:
            case NFAStateType::kNotSpace:
            case NFAStateType::kCharClass:
            case NFAStateType::kUnicodeCodePoint:
            case NFAStateType::kUnicodeProp:
            case NFAStateType::kNotUnicodeProp: {
                // Check if we can consume a character
                if (str_pos >= str.size()) {
                    break;  // No more input
                }

                char ch = str[str_pos];

                // Check character match
                if (!MatchChar(state, ch)) {
                    break;
                }

                // For Unicode code points encoded as multi-byte UTF-8, consume all continuation bytes
                size_t advance = 1;
                if (state.type == NFAStateType::kUnicodeCodePoint) {
                    unsigned char uch = static_cast<unsigned char>(ch);
                    if ((uch & 0x80) != 0 && (uch & 0xC0) != 0x80) {
                        uint32_t cp = state.unicode_code_point.code_point;
                        if (cp <= 0x7FF) advance = 2;
                        else if (cp <= 0xFFFF) advance = 3;
                        else if (cp <= 0x10FFFF) advance = 4;
                        // Ensure we don't read past the string
                        if (str_pos + advance > str.size()) {
                            break;
                        }
                    }
                }

                // Character matches, move to next state consuming the character(s)
                if (state.next1 != kInvalidNFAStateId) {
                    stack.push_back({state.next1, str_pos + advance, captures});
                }
                break;
            }

            case NFAStateType::kBackref: {
                // Check backreference match
                if (!MatchBackref(state, str, str_pos, captures)) {
                    break;
                }

                // Backreference matches, consume the referenced text and move to next state
                size_t capture_len = 0;
                uint32_t ref_index = state.backref.ref_index;
                if (ref_index > 0 && ref_index <= captures.size()) {
                    const auto& [start, end] = captures[ref_index - 1];
                    if (start != SIZE_MAX && end != SIZE_MAX) {
                        capture_len = end - start;
                    }
                }

                if (state.next1 != kInvalidNFAStateId) {
                    stack.push_back({state.next1, str_pos + capture_len, captures});
                }
                break;
            }

            default:
                break;
        }
    }

    if (best_match.has_value()) {
    }
    return best_match;
}

bool NFA::MatchChar(const NFAState& state, char ch) const {
    switch (state.type) {
        case NFAStateType::kChar:
            return state.ch == ch;

        case NFAStateType::kCharClass:
            return MatchCharClass(state, ch);

        case NFAStateType::kDot:
            // dotAll模式下，. 匹配任意字符包括换行符
            return dot_all_ || (ch != '\n' && ch != '\r');

        case NFAStateType::kDigit:
            return std::isdigit(static_cast<unsigned char>(ch)) != 0;

        case NFAStateType::kNotDigit:
            return std::isdigit(static_cast<unsigned char>(ch)) == 0;

        case NFAStateType::kWord:
            return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';

        case NFAStateType::kNotWord:
            return !(std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_');

        case NFAStateType::kSpace:
            return std::isspace(static_cast<unsigned char>(ch)) != 0;

        case NFAStateType::kNotSpace:
            return std::isspace(static_cast<unsigned char>(ch)) == 0;

        case NFAStateType::kUnicodeCodePoint: {
            // Unicode码点匹配
            // 注意：由于我们在NFA构建时已经将UTF-8序列展开为字符链，
            // 所以理论上不应该到达这里。为了安全起见，我们保留这个case。
            // 这个状态类型可能在未来的实现中使用，或者是为了向后兼容。

            uint32_t code_point = state.unicode_code_point.code_point;

            // 对于ASCII范围内的码点，直接比较
            if (code_point < 128) {
                return static_cast<uint32_t>(static_cast<unsigned char>(ch)) == code_point;
            }

            // 对于非ASCII码点，检查是否是UTF-8序列的第一个字节
            unsigned char uch = static_cast<unsigned char>(ch);

            // 如果是UTF-8后续字节(10xxxxxx)，不匹配
            if ((uch & 0xC0) == 0x80) {
                return false;
            }

            // 计算UTF-8编码的第一个字节并与当前字符比较
            char first_byte = 0;
            if (code_point <= 0x7FF) {
                first_byte = 0xC0 | (code_point >> 6);
            } else if (code_point <= 0xFFFF) {
                first_byte = 0xE0 | (code_point >> 12);
            } else if (code_point <= 0x10FFFF) {
                first_byte = 0xF0 | (code_point >> 18);
            }

            return uch == static_cast<unsigned char>(first_byte);
        }

        case NFAStateType::kUnicodeProp:
        case NFAStateType::kNotUnicodeProp: {
            // Unicode属性匹配
            bool matched = false;
            for (size_t i = 0; i < state.unicode_prop.range_count; ++i) {
                char start = state.unicode_prop.ranges[i * 2];
                char end = state.unicode_prop.ranges[i * 2 + 1];
                if (ch >= start && ch <= end) {
                    matched = true;
                    break;
                }
            }
            return state.unicode_prop.negated ? !matched : matched;
        }

        default:
            return false;
    }
}

bool NFA::MatchBackref(const NFAState& state, const std::string& str, size_t pos,
                       const std::vector<std::pair<size_t, size_t>>& captures) const {
    uint32_t ref_index = state.backref.ref_index;

    // Check if the referenced capture group exists and has been captured
    if (ref_index == 0 || ref_index > captures.size()) {
        return false;
    }

    const auto& [start, end] = captures[ref_index - 1];
    if (start == SIZE_MAX || end == SIZE_MAX) {
        // In JavaScript, an unmatched capture backreference matches the empty string.
        return true;
    }

    // Check if the text at the current position matches the captured text
    size_t capture_len = end - start;
    if (pos + capture_len > str.size()) {
        return false;
    }

    for (size_t i = 0; i < capture_len; ++i) {
        if (str[pos + i] != str[start + i]) {
            return false;
        }
    }

    return true;
}

bool NFA::MatchCharClass(const NFAState& state, char ch) const {
    bool matched = false;

    for (size_t i = 0; i < state.char_class.range_count; ++i) {
        char start = state.char_class.ranges[i * 2];
        char end = state.char_class.ranges[i * 2 + 1];

        bool in_range = ch >= start && ch <= end;
        if (!in_range && ignore_case_) {
            unsigned char uch = static_cast<unsigned char>(ch);
            char lower = static_cast<char>(std::tolower(uch));
            char upper = static_cast<char>(std::toupper(uch));
            in_range = (lower >= start && lower <= end) || (upper >= start && upper <= end);
        }

        if (in_range) {
            matched = true;
            break;
        }
    }

    return state.char_class.negated ? !matched : matched;
}

bool NFA::MatchBoundary(const NFAState& state, const std::string& str, size_t pos) const {
    bool is_word_char_before = false;
    bool is_word_char_after = false;

    // 检查前一个字符是否为单词字符
    if (pos > 0) {
        char prev_ch = str[pos - 1];
        is_word_char_before = (std::isalnum(static_cast<unsigned char>(prev_ch)) != 0 || prev_ch == '_');
    }

    // 检查当前字符是否为单词字符
    if (pos < str.size()) {
        char next_ch = str[pos];
        is_word_char_after = (std::isalnum(static_cast<unsigned char>(next_ch)) != 0 || next_ch == '_');
    }

    switch (state.boundary_type) {
        case BoundaryType::kStart:
            if (multiline_) {
                // 多行模式：^ 匹配字符串开头或换行符之后
                return pos == 0 || (pos > 0 && (str[pos - 1] == '\n' || str[pos - 1] == '\r'));
            } else {
                return pos == 0;
            }

        case BoundaryType::kEnd:
            if (multiline_) {
                // 多行模式：$ 匹配字符串结尾或换行符之前
                return pos == str.size() || (pos < str.size() && (str[pos] == '\n' || str[pos] == '\r'));
            } else {
                return pos == str.size();
            }

        case BoundaryType::kWordBoundary:
            // 词边界：一侧是单词字符，另一侧不是
            return is_word_char_before != is_word_char_after;

        case BoundaryType::kNotWordBoundary:
            // 非词边界：两侧都是单词字符或都不是
            return is_word_char_before == is_word_char_after;

        default:
            return false;
    }
}

bool NFA::MatchLookahead(const NFAState& state, const std::string& str, size_t pos) const {
    // 从当前位置尝试匹配前瞻子NFA
    auto lookahead_start = state.lookahead.lookahead_start;
    auto lookahead_accept = state.lookahead.lookahead_accept;  // 前瞻子NFA的接受状态

    // 尝试从pos位置匹配前瞻子NFA
    auto current = EpsilonWithContext(lookahead_start, str, pos);

    // 初始检查：空匹配
    bool matched = (current.find(lookahead_accept) != current.end());

    // 逐字符匹配
    for (size_t i = pos; i < str.size(); ++i) {
        char ch = str[i];
        auto next = Move(current, ch);
        current = EpsilonWithContext(next, str, i + 1);

        if (current.empty()) {
            break;
        }

        if (current.find(lookahead_accept) != current.end()) {
            matched = true;
            break;
        }
    }

    // 正向前瞻要求匹配，负向前瞻要求不匹配
    return state.lookahead.positive ? matched : !matched;
}

bool NFA::MatchLookbehind(const NFAState& state, const std::string& str, size_t pos) const {
    // 从当前位置向前尝试匹配后瞻子NFA
    auto lookbehind_start = state.lookbehind.lookbehind_start;
    auto lookbehind_accept = state.lookbehind.lookbehind_accept;  // 后瞻子NFA的接受状态

    // 后瞻断言需要检查当前位置之前的文本是否匹配后瞻模式
    // 我们尝试从不同的起点开始匹配，看是否能精确匹配到当前位置

    for (size_t start = 0; start <= pos; ++start) {
        size_t lookbehind_len = pos - start;

        // 首先计算 epsilon 闭包（只处理纯 epsilon 转移，不包括断言）
        std::set<NFAStateId> current = EpsilonClosure(lookbehind_start);

        // 如果初始状态集合为空，跳过
        if (current.empty()) {
            continue;
        }

        // 逐字符匹配后瞻模式
        bool matched = true;

        for (size_t i = 0; i < lookbehind_len; ++i) {
            char ch = str[start + i];
            std::set<NFAStateId> next;

            // 对当前状态集合中的每个状态进行字符转移
            for (auto state_id : current) {
                if (state_id >= states_.size()) {
                    continue;
                }

                const auto& s = states_[state_id];

                // 检查字符匹配
                if (!MatchChar(s, ch)) {
                    continue;
                }

                // 字符匹配，添加转移后的状态
                if (s.next1 != kInvalidNFAStateId) {
                    next.insert(s.next1);
                }
            }

            // 如果没有下一个状态，匹配失败
            if (next.empty()) {
                matched = false;
                break;
            }

            // 计算下一个状态的 epsilon 闭包
            current.clear();
            for (auto next_id : next) {
                auto epsilon_closure = EpsilonClosure(next_id);
                current.insert(epsilon_closure.begin(), epsilon_closure.end());
            }

            // 如果 epsilon 闭包后状态为空，匹配失败
            if (current.empty()) {
                matched = false;
                break;
            }
        }

        // 检查是否到达接受状态（精确匹配了后瞻模式）
        if (matched && current.find(lookbehind_accept) != current.end()) {
            // 找到匹配
            return state.lookbehind.positive;
        }
    }

    // 没有找到匹配
    return !state.lookbehind.positive;
}

// ==================== NFABuilder ====================

NFA NFABuilder::Build(RegExpASTNode* root, bool ignore_case, bool multiline, bool dot_all, bool unicode, bool unicode_sets) {
    NFA nfa;

    if (!root) {
        // 创建空NFA
        auto start = nfa.CreateState(NFAStateType::kMatch);
        nfa.SetStartState(start);
        nfa.SetAcceptState(start);
        return nfa;
    }

    // Set flags
    nfa.SetFlags(multiline, dot_all, unicode, unicode_sets, ignore_case);

    // First pass: count capture groups
    uint32_t capture_count = CountCaptures(root);
    nfa.SetCaptureCount(capture_count);

    auto pair = Build(root, nfa, ignore_case);
    nfa.SetStartState(pair.start);
    nfa.SetAcceptState(pair.accept);

    return nfa;
}

uint32_t NFABuilder::CountCaptures(RegExpASTNode* node) {
    if (!node) {
        return 0;
    }

    switch (node->type()) {
        case RegExpASTNodeType::kGroup: {
            auto* group = static_cast<GroupNode*>(node);
            // Only count capturing groups (capture_index > 0)
            uint32_t count = (group->capture_index() > 0) ? 1 : 0;
            count += CountCaptures(group->child());
            return count;
        }

        case RegExpASTNodeType::kConcat: {
            auto* concat = static_cast<ConcatNode*>(node);
            uint32_t count = 0;
            for (const auto& child : concat->children()) {
                count += CountCaptures(child.get());
            }
            return count;
        }

        case RegExpASTNodeType::kAlternative: {
            auto* alt = static_cast<AlternativeNode*>(node);
            uint32_t count = 0;
            for (const auto& child : alt->alternatives()) {
                uint32_t child_count = CountCaptures(child.get());
                // All alternatives should have the same number of captures
                // We take the maximum to be safe
                count = std::max(count, child_count);
            }
            return count;
        }

        case RegExpASTNodeType::kStar:
        case RegExpASTNodeType::kPlus:
        case RegExpASTNodeType::kQuestion: {
            // These nodes have a single child
            RegExpASTNode* child = nullptr;
            if (node->type() == RegExpASTNodeType::kStar) {
                child = static_cast<StarNode*>(node)->child();
            } else if (node->type() == RegExpASTNodeType::kPlus) {
                child = static_cast<PlusNode*>(node)->child();
            } else {
                child = static_cast<QuestionNode*>(node)->child();
            }
            return CountCaptures(child);
        }

        case RegExpASTNodeType::kLookahead: {
            return CountCaptures(static_cast<LookaheadNode*>(node)->child());
        }

        case RegExpASTNodeType::kLookbehind: {
            return CountCaptures(static_cast<LookbehindNode*>(node)->child());
        }

        default:
            return 0;
    }
}

NFABuilder::NFAPair NFABuilder::Build(RegExpASTNode* node, NFA& nfa, bool ignore_case) {
    switch (node->type()) {
        case RegExpASTNodeType::kChar: {
            auto char_node = static_cast<CharNode*>(node);
            return BuildChar(nfa, char_node->ch(), ignore_case || char_node->ignore_case());
        }

        case RegExpASTNodeType::kDot:
            return BuildDot(nfa);

        case RegExpASTNodeType::kCharClass:
            return BuildCharClass(nfa, static_cast<CharClassNode*>(node));

        case RegExpASTNodeType::kEscape: {
            auto* escape_node = static_cast<EscapeNode*>(node);
            if (escape_node->escape_type() == EscapeNode::EscapeType::kBackref) {
                return BuildBackref(nfa, escape_node);
            }
            return BuildEscape(nfa, escape_node);
        }

        case RegExpASTNodeType::kConcat:
            return BuildConcat(nfa, static_cast<ConcatNode*>(node), ignore_case);

        case RegExpASTNodeType::kAlternative:
            return BuildAlternative(nfa, static_cast<AlternativeNode*>(node), ignore_case);

        case RegExpASTNodeType::kStar: {
            auto star_node = static_cast<StarNode*>(node);
            auto child = Build(star_node->child(), nfa, ignore_case);
            return BuildStar(nfa, child, star_node->greedy());
        }

        case RegExpASTNodeType::kPlus: {
            auto plus_node = static_cast<PlusNode*>(node);
            auto child = Build(plus_node->child(), nfa, ignore_case);
            return BuildPlus(nfa, child, plus_node->greedy());
        }

        case RegExpASTNodeType::kQuestion: {
            auto question_node = static_cast<QuestionNode*>(node);
            auto child = Build(question_node->child(), nfa, ignore_case);
            return BuildQuestion(nfa, child, question_node->greedy());
        }

        case RegExpASTNodeType::kGroup:
            return BuildGroup(nfa, static_cast<GroupNode*>(node), ignore_case);

        case RegExpASTNodeType::kBoundary:
            return BuildBoundary(nfa, static_cast<BoundaryNode*>(node));

        case RegExpASTNodeType::kLookahead:
            return BuildLookahead(nfa, static_cast<LookaheadNode*>(node), ignore_case);

        case RegExpASTNodeType::kLookbehind:
            return BuildLookbehind(nfa, static_cast<LookbehindNode*>(node), ignore_case);

        default:
            // 不应该到达这里
            auto state = nfa.CreateState(NFAStateType::kMatch);
            return {state, state};
    }
}

NFABuilder::NFAPair NFABuilder::BuildChar(NFA& nfa, char ch, bool ignore_case) {
    auto start = nfa.CreateState(NFAStateType::kChar);
    auto accept = nfa.CreateState(NFAStateType::kMatch);

    if (ignore_case) {
        // 忽略大小写，创建两个转移
        nfa.SetCharTransition(start, accept, ToLower(ch));
        auto alt_start = nfa.CreateState(NFAStateType::kChar);
        nfa.SetCharTransition(alt_start, accept, ToUpper(ch));

        // 创建新的起始状态，用epsilon连接
        auto new_start = nfa.CreateState(NFAStateType::kEpsilon);
        nfa.SetEpsilonTransition(new_start, start);
        nfa.SetEpsilonTransition(new_start, alt_start);

        return {new_start, accept};
    } else {
        nfa.SetCharTransition(start, accept, ch);
        return {start, accept};
    }
}

NFABuilder::NFAPair NFABuilder::BuildDot(NFA& nfa) {
    auto start = nfa.CreateState(NFAStateType::kDot);
    auto accept = nfa.CreateState(NFAStateType::kMatch);
    nfa.SetDotTransition(start, accept);
    return {start, accept};
}

NFABuilder::NFAPair NFABuilder::BuildCharClass(NFA& nfa, const CharClassNode* node) {
    auto start = nfa.CreateState(NFAStateType::kCharClass);
    auto accept = nfa.CreateState(NFAStateType::kMatch);

    std::vector<std::pair<char, char>> ranges;
    for (const auto& range : node->ranges()) {
        ranges.emplace_back(range.start, range.end);
    }

    nfa.SetCharClassTransition(start, accept, node->negated(), ranges);
    return {start, accept};
}

NFABuilder::NFAPair NFABuilder::BuildEscape(NFA& nfa, const EscapeNode* node) {
    auto start = nfa.CreateState(NFAStateType::kChar);
    auto accept = nfa.CreateState(NFAStateType::kMatch);

    switch (node->escape_type()) {
        case EscapeNode::EscapeType::kDigit:
            start = nfa.CreateState(NFAStateType::kDigit);
            nfa.SetTypeTransition(start, accept, NFAStateType::kDigit);
            break;

        case EscapeNode::EscapeType::kNotDigit:
            start = nfa.CreateState(NFAStateType::kNotDigit);
            nfa.SetTypeTransition(start, accept, NFAStateType::kNotDigit);
            break;

        case EscapeNode::EscapeType::kWord:
            start = nfa.CreateState(NFAStateType::kWord);
            nfa.SetTypeTransition(start, accept, NFAStateType::kWord);
            break;

        case EscapeNode::EscapeType::kNotWord:
            start = nfa.CreateState(NFAStateType::kNotWord);
            nfa.SetTypeTransition(start, accept, NFAStateType::kNotWord);
            break;

        case EscapeNode::EscapeType::kSpace:
            start = nfa.CreateState(NFAStateType::kSpace);
            nfa.SetTypeTransition(start, accept, NFAStateType::kSpace);
            break;

        case EscapeNode::EscapeType::kNotSpace:
            start = nfa.CreateState(NFAStateType::kNotSpace);
            nfa.SetTypeTransition(start, accept, NFAStateType::kNotSpace);
            break;

        case EscapeNode::EscapeType::kChar:
            nfa.SetCharTransition(start, accept, node->ch());
            break;

        case EscapeNode::EscapeType::kHex:
            // \xHH 十六进制转义，作为字符处理
            nfa.SetCharTransition(start, accept, node->ch());
            break;

        case EscapeNode::EscapeType::kUnicode:
            // \uHHHH Unicode转义，作为字符处理
            // 注意：目前只支持BMP字符（0x0000-0xFFFF）
            nfa.SetCharTransition(start, accept, node->ch());
            break;

        case EscapeNode::EscapeType::kOctal:
            // \0, \00, \000 八进制转义，作为字符处理
            nfa.SetCharTransition(start, accept, node->ch());
            break;

        case EscapeNode::EscapeType::kNamedBackref: {
            // 命名反向引用 \k<name>
            // 使用 ref_index 来引用捕获组
            auto ref_index = node->ref_index();
            auto backref_start = nfa.CreateState(NFAStateType::kBackref);
            auto backref_accept = nfa.CreateState(NFAStateType::kMatch);
            nfa.SetBackrefTransition(backref_start, backref_accept, ref_index);
            return {backref_start, backref_accept};
        }

        case EscapeNode::EscapeType::kUnicodeCodePoint: {
            // \u{HHHHHH} Unicode码点转义
            auto code_point = node->code_point();

            // 对于ASCII范围内的码点（0-127），直接使用字符转移
            if (code_point < 128) {
                char ch = static_cast<char>(code_point);
                auto char_start = nfa.CreateState(NFAStateType::kChar);
                auto char_accept = nfa.CreateState(NFAStateType::kMatch);
                nfa.SetCharTransition(char_start, char_accept, ch);
                return {char_start, char_accept};
            }

            // 对于非ASCII码点，转换为UTF-8字节序列并创建连接的状态链
            char utf8_buf[5];
            size_t utf8_len = 0;

            if (code_point <= 0x7FF) {
                // 2字节编码
                utf8_buf[0] = 0xC0 | (code_point >> 6);
                utf8_buf[1] = 0x80 | (code_point & 0x3F);
                utf8_len = 2;
            } else if (code_point <= 0xFFFF) {
                // 3字节编码
                utf8_buf[0] = 0xE0 | (code_point >> 12);
                utf8_buf[1] = 0x80 | ((code_point >> 6) & 0x3F);
                utf8_buf[2] = 0x80 | (code_point & 0x3F);
                utf8_len = 3;
            } else if (code_point <= 0x10FFFF) {
                // 4字节编码
                utf8_buf[0] = 0xF0 | (code_point >> 18);
                utf8_buf[1] = 0x80 | ((code_point >> 12) & 0x3F);
                utf8_buf[2] = 0x80 | ((code_point >> 6) & 0x3F);
                utf8_buf[3] = 0x80 | (code_point & 0x3F);
                utf8_len = 4;
            } else {
                // 无效的码点，创建一个永远不匹配的状态
                auto invalid_start = nfa.CreateState(NFAStateType::kChar);
                auto invalid_accept = nfa.CreateState(NFAStateType::kMatch);
                nfa.SetCharTransition(invalid_start, invalid_accept, '\0');
                return {invalid_start, invalid_accept};
            }

            // 创建UTF-8字节序列的NFA链
            NFAStateId current = nfa.CreateState(NFAStateType::kMatch);  // 最终接受状态
            NFAStateId accept_state = current;

            // 从后向前创建状态，这样第一个创建的状态就是起始状态
            for (int i = static_cast<int>(utf8_len) - 1; i >= 0; --i) {
                auto state = nfa.CreateState(NFAStateType::kChar);
                nfa.SetCharTransition(state, current, utf8_buf[i]);
                current = state;
            }

            return {current, accept_state};
        }

        case EscapeNode::EscapeType::kUnicodeProp:
        case EscapeNode::EscapeType::kNotUnicodeProp: {
            // \p{...} 或 \P{...} Unicode属性转义
            // 这里我们简化实现，支持一些常用的Unicode属性
            bool negated = (node->escape_type() == EscapeNode::EscapeType::kNotUnicodeProp);
            const auto& prop_name = node->prop_name();

            // 根据属性名称获取字符范围
            std::vector<std::pair<char, char>> ranges;
            bool supported = mjs::GetUnicodePropRanges(prop_name, ranges);

            if (!supported) {
                // Unsupported property，创建一个空匹配（永远不匹配）
                auto state = nfa.CreateState(NFAStateType::kMatch);
                return {state, state};
            }

            auto prop_start = nfa.CreateState(NFAStateType::kUnicodeProp);
            auto prop_accept = nfa.CreateState(NFAStateType::kMatch);
            nfa.SetUnicodePropTransition(prop_start, prop_accept, negated, ranges);
            return {prop_start, prop_accept};
        }
    }

    return {start, accept};
}

NFABuilder::NFAPair NFABuilder::BuildConcat(NFA& nfa, const ConcatNode* node, bool ignore_case) {
    auto& children = node->children();

    if (children.empty()) {
        // 空序列，接受空字符串
        auto state = nfa.CreateState(NFAStateType::kMatch);
        return {state, state};
    }


    // 构建第一个子节点
    auto first_pair = Build(children[0].get(), nfa, ignore_case);

    // 依次连接剩余子节点
    NFAStateId last_accept = first_pair.accept;
    for (size_t i = 1; i < children.size(); ++i) {
        auto next_pair = Build(children[i].get(), nfa, ignore_case);

        // 用epsilon连接前一个接受状态到下一个起始状态
        nfa.SetEpsilonTransition(last_accept, next_pair.start);

        last_accept = next_pair.accept;
    }

    return {first_pair.start, last_accept};
}

NFABuilder::NFAPair NFABuilder::BuildAlternative(NFA& nfa, const AlternativeNode* node, bool ignore_case) {
    auto& alternatives = node->alternatives();

    if (alternatives.empty()) {
        auto state = nfa.CreateState(NFAStateType::kMatch);
        return {state, state};
    }

    if (alternatives.size() == 1) {
        return Build(alternatives[0].get(), nfa, ignore_case);
    }

    // 创建新的起始和接受状态
    auto new_start = nfa.CreateState(NFAStateType::kEpsilon);
    auto new_accept = nfa.CreateState(NFAStateType::kMatch);

    // 为每个选择创建分支
    for (auto& alt : alternatives) {
        auto pair = Build(alt.get(), nfa, ignore_case);

        // epsilon从新起始状态到该分支起始状态
        nfa.SetEpsilonTransition(new_start, pair.start);

        // epsilon从该分支接受状态到新接受状态
        nfa.SetEpsilonTransition(pair.accept, new_accept);
    }

    return {new_start, new_accept};
}

NFABuilder::NFAPair NFABuilder::BuildStar(NFA& nfa, NFAPair child, bool greedy) {
    // 创建新的起始和接受状态
    auto new_start = nfa.CreateState(NFAStateType::kStar);
    auto new_accept = nfa.CreateState(NFAStateType::kMatch);

    // 设置greedy标志和循环体起始状态
    auto& start_state = nfa.GetState(new_start);
    start_state.repeat.greedy = greedy;
    start_state.repeat.loop_start = child.start;

    // 根据贪婪/非贪婪设置转移顺序
    if (greedy) {
        // 贪婪：优先进入循环体（next1），然后跳过（next2）
        start_state.next1 = child.start;      // 进入循环体
        start_state.next2 = new_accept;       // 跳过循环
    } else {
        // 非贪婪：优先跳过（next1），然后进入循环体（next2）
        start_state.next1 = new_accept;       // 跳过循环
        start_state.next2 = child.start;      // 进入循环体
    }

    // epsilon从子NFA接受状态回到子NFA起始状态（循环）
    nfa.SetEpsilonTransition(child.accept, child.start);

    // epsilon从子NFA接受状态到新接受状态（退出循环）
    nfa.SetEpsilonTransition(child.accept, new_accept);

    return {new_start, new_accept};
}

NFABuilder::NFAPair NFABuilder::BuildPlus(NFA& nfa, NFAPair child, bool greedy) {
    // + 等价于先匹配一次，再匹配*
    // 创建新的起始和接受状态
    auto new_start = nfa.CreateState(NFAStateType::kPlus);
    auto new_accept = nfa.CreateState(NFAStateType::kMatch);

    // 设置greedy标志和循环体起始状态
    auto& start_state = nfa.GetState(new_start);
    start_state.repeat.greedy = greedy;
    start_state.repeat.loop_start = child.start;
    start_state.next1 = child.start;      // 首先必须匹配一次

    // 从子NFA接受状态，可以选择继续循环或退出
    if (greedy) {
        // 贪婪：优先继续循环
        nfa.SetEpsilonTransition(child.accept, child.start);      // 继续循环
        nfa.SetEpsilonTransition(child.accept, new_accept);       // 退出循环
    } else {
        // 非贪婪：优先退出循环
        nfa.SetEpsilonTransition(child.accept, new_accept);       // 退出循环
        nfa.SetEpsilonTransition(child.accept, child.start);      // 继续循环
    }

    return {new_start, new_accept};
}

NFABuilder::NFAPair NFABuilder::BuildQuestion(NFA& nfa, NFAPair child, bool greedy) {
    // 创建新的起始和接受状态
    auto new_start = nfa.CreateState(NFAStateType::kQuestion);
    auto new_accept = nfa.CreateState(NFAStateType::kMatch);

    // 设置greedy标志和循环体起始状态
    auto& start_state = nfa.GetState(new_start);
    start_state.repeat.greedy = greedy;
    start_state.repeat.loop_start = child.start;

    // 根据贪婪/非贪婪设置转移顺序
    if (greedy) {
        // 贪婪：优先尝试匹配子NFA，然后跳过
        start_state.next1 = child.start;      // 尝试匹配
        start_state.next2 = new_accept;       // 跳过
    } else {
        // 非贪婪：优先跳过，然后尝试匹配
        start_state.next1 = new_accept;       // 跳过
        start_state.next2 = child.start;      // 尝试匹配
    }

    // epsilon从子NFA接受状态到新接受状态
    nfa.SetEpsilonTransition(child.accept, new_accept);

    return {new_start, new_accept};
}

NFABuilder::NFAPair NFABuilder::BuildGroup(NFA& nfa, const GroupNode* node, bool ignore_case) {
    uint32_t capture_index = node->capture_index();

    // 构建子节点的NFA
    auto child_pair = Build(node->child(), nfa, ignore_case);

    // 如果是捕获组（capture_index > 0），插入捕获开始/结束标记
    if (capture_index > 0) {
        // 创建捕获开始状态（在child之前）
        auto capture_start = nfa.CreateState(NFAStateType::kCaptureStart);

        // 检查child_pair.accept是否有效
        if (child_pair.accept >= nfa.StateCount()) {
            // 返回一个无效的pair
            auto error_state = nfa.CreateState(NFAStateType::kMatch);
            return {error_state, error_state};
        }

        // 检查child_pair.accept状态的epsilon_transitions
        // 注意：在创建新状态之前保存状态ID，避免引用失效
        NFAStateId child_accept_id = child_pair.accept;

        nfa.SetCaptureStartTransition(capture_start, child_pair.start, capture_index);

        // 创建捕获结束状态（在child之后）
        auto capture_end = nfa.CreateState(NFAStateType::kCaptureEnd);
        auto& capture_end_state = nfa.GetState(capture_end);
        capture_end_state.capture.capture_index = capture_index;

        // 重新获取child_accept状态的引用（在创建新状态后）
        // 因为states_可能重新分配了内存
        auto& child_accept_state = nfa.GetState(child_accept_id);

        // 从child的accept状态到capture_end状态设置epsilon转移
        // 直接设置next1，避免使用SetEpsilonTransition破坏状态类型
        if (child_accept_state.next1 == kInvalidNFAStateId) {
            child_accept_state.next1 = capture_end;
        } else if (child_accept_state.next2 == kInvalidNFAStateId) {
            child_accept_state.next2 = capture_end;
        } else {
            child_accept_state.epsilon_transitions.push_back(capture_end);
        }

        // 返回capture_end作为新的accept状态
        return {capture_start, capture_end};
    } else {
        // 非捕获组，直接返回子节点的NFA
        return child_pair;
    }
}

NFABuilder::NFAPair NFABuilder::BuildBoundary(NFA& nfa, const BoundaryNode* node) {
    auto start = nfa.CreateState(NFAStateType::kBoundary);
    auto accept = nfa.CreateState(NFAStateType::kMatch);

    BoundaryType boundary_type;
    switch (node->boundary_type()) {
        case BoundaryNode::BoundaryType::kStart:
            boundary_type = BoundaryType::kStart;
            break;
        case BoundaryNode::BoundaryType::kEnd:
            boundary_type = BoundaryType::kEnd;
            break;
        case BoundaryNode::BoundaryType::kWordBoundary:
            boundary_type = BoundaryType::kWordBoundary;
            break;
        case BoundaryNode::BoundaryType::kNotWordBoundary:
            boundary_type = BoundaryType::kNotWordBoundary;
            break;
    }

    nfa.SetBoundaryTransition(start, accept, boundary_type);
    return {start, accept};
}

NFABuilder::NFAPair NFABuilder::BuildLookahead(NFA& nfa, const LookaheadNode* node, bool ignore_case) {
    // 首先构建前瞻的子NFA
    auto lookahead_pair = Build(node->child(), nfa, ignore_case);

    // 创建边界状态
    auto start = nfa.CreateState(NFAStateType::kLookahead);
    auto accept = nfa.CreateState(NFAStateType::kMatch);

    // 设置前瞻转移，指向前瞻子NFA的起始和接受状态
    nfa.SetLookaheadTransition(start, accept, node->positive(),
                               lookahead_pair.start, lookahead_pair.accept);

    return {start, accept};
}

NFABuilder::NFAPair NFABuilder::BuildLookbehind(NFA& nfa, const LookbehindNode* node, bool ignore_case) {
    // 首先构建后瞻的子NFA
    auto lookbehind_pair = Build(node->child(), nfa, ignore_case);

    // 创建边界状态
    auto start = nfa.CreateState(NFAStateType::kLookbehind);

    // 创建一个特殊的占位符状态，用于后瞻子NFA的接受
    // 这个状态不会作为主NFA的接受状态，避免混淆
    auto lookbehind_accept = nfa.CreateState(NFAStateType::kMatch);

    // 将后瞻子NFA的接受状态连接到这个占位符
    nfa.SetEpsilonTransition(lookbehind_pair.accept, lookbehind_accept);

    // 创建后瞻断言的接受状态
    auto accept = nfa.CreateState(NFAStateType::kMatch);

    // 设置后瞻转移，指向后瞻子NFA的起始和占位符接受状态
    nfa.SetLookbehindTransition(start, accept, node->positive(), lookbehind_pair.start, lookbehind_accept);

    return {start, accept};
}

NFABuilder::NFAPair NFABuilder::BuildBackref(NFA& nfa, const EscapeNode* node) {
    uint32_t ref_index = node->ref_index();

    auto start = nfa.CreateState(NFAStateType::kBackref);
    auto accept = nfa.CreateState(NFAStateType::kMatch);

    nfa.SetBackrefTransition(start, accept, ref_index);

    return {start, accept};
}

char NFABuilder::ToLower(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return ch + 32;
    }
    return ch;
}

char NFABuilder::ToUpper(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        return ch - 32;
    }
    return ch;
}

bool NFABuilder::CharEqualsIgnoreCase(char a, char b) {
    return ToLower(a) == ToLower(b);
}

} // namespace mjs
