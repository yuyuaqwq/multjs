/**
 * @file regexp_object.cpp
 * @brief 正则表达式对象实现
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <mjs/regexp/regexp_object.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/error.h>
#include <mjs/value/object/array_object.h>
#include <mjs/value/string.h>
#include <mjs/gc/handle.h>
#include <mjs/const_index_embedded.h>
#include <format>
#include <cstring>

namespace mjs {

namespace {
constexpr const char* kValidRegExpFlags = "dgimsuvy";

size_t AdvanceStringIndex(const std::string& str, size_t index, bool unicode_mode) {
    if (index >= str.size()) {
        return index + 1;
    }

    if (!unicode_mode) {
        return index + 1;
    }

    unsigned char lead = static_cast<unsigned char>(str[index]);
    if ((lead & 0x80) == 0x00) {
        return index + 1;
    }
    if ((lead & 0xE0) == 0xC0 && index + 1 <= str.size()) {
        return std::min(index + 2, str.size());
    }
    if ((lead & 0xF0) == 0xE0 && index + 2 <= str.size()) {
        return std::min(index + 3, str.size());
    }
    if ((lead & 0xF8) == 0xF0 && index + 3 <= str.size()) {
        return std::min(index + 4, str.size());
    }
    return index + 1;
}
}  // namespace

RegExpObject::RegExpObject(Context* context, const std::string& pattern, const std::string& flags)
    : Object(context, ClassId::kRegExpObject),
      pattern_(pattern),
      flags_(flags),
      global_(false),
      ignore_case_(false),
      multiline_(false),
      dot_all_(false),
      unicode_(false),
      sticky_(false),
      unicode_sets_(false),
      has_indices_(false),
      last_index_(0) {

    // 解析正则表达式标志
    for (char flag : flags) {
        switch (flag) {
            case 'g':
                global_ = true;
                break;
            case 'i':
                ignore_case_ = true;
                break;
            case 'm':
                multiline_ = true;
                break;
            case 's':
                dot_all_ = true;
                break;
            case 'u':
                unicode_ = true;
                break;
            case 'y':
                sticky_ = true;
                break;
            case 'v':
                unicode_sets_ = true;
                break;
            case 'd':
                has_indices_ = true;
                break;
            default:
                // 未知标志，忽略
                break;
        }
    }

    // 编译正则表达式
    Compile();
}

RegExpObject::~RegExpObject() = default;

std::string RegExpObject::ValidateFlags(const std::string& flags) {
    bool seen[256] = {false};

    for (char flag : flags) {
        unsigned char uflag = static_cast<unsigned char>(flag);
        if (std::strchr(kValidRegExpFlags, flag) == nullptr) {
            return std::format("Invalid regular expression flag '{}'", flag);
        }
        if (seen[uflag]) {
            return std::format("Duplicate regular expression flag '{}'", flag);
        }
        seen[uflag] = true;
    }

    if (flags.find('u') != std::string::npos && flags.find('v') != std::string::npos) {
        return "Invalid regular expression flags 'u' and 'v' cannot be used together";
    }

    return "";
}

bool RegExpObject::Compile() {
    compile_error_.clear();
    capture_group_names_.clear();
    nfa_.reset();

    if (auto error = ValidateFlags(flags_); !error.empty()) {
        compile_error_ = std::move(error);
        return false;
    }

    RegExpParser parser(pattern_, unicode_ || unicode_sets_, unicode_sets_);
    auto ast = parser.Parse();

    if (!ast) {
        compile_error_ = parser.GetError();
        return false;
    }

    capture_group_names_ = parser.capture_group_names();
    nfa_ = std::make_unique<NFA>(NFABuilder::Build(ast.get(), ignore_case_, multiline_, dot_all_, unicode_, unicode_sets_));

    return true;
}

bool RegExpObject::Test(Context* context, const std::string& str) {
    if (!nfa_) {
        return false;
    }

    // 根据 last_index 确定搜索起始位置
    size_t start_pos = (global_ || sticky_) ? last_index_ : 0;

    auto result = nfa_->MatchDetail(str, start_pos);

    if (result.has_value()) {
        // sticky 模式下匹配必须从 last_index 位置开始
        if (sticky_ && result->start_pos != start_pos) {
            // Sticky mode requires the match to start at exactly start_pos
            last_index_ = 0;
            return false;
        }

        // 更新 lastIndex
        if (global_ || sticky_) {
            size_t next_index = result->end_pos;
            if (next_index == start_pos) {
                next_index = AdvanceStringIndex(str, start_pos, unicode_ || unicode_sets_);
            }
            last_index_ = static_cast<uint32_t>(next_index);
        }
        return true;
    }

    // 匹配失败，重置 lastIndex
    if (global_ || sticky_) {
        last_index_ = 0;
    }

    return false;
}

Value RegExpObject::Exec(Context* context, const std::string& str) {
    if (!nfa_) {
        return Value(nullptr);
    }

    // 根据 last_index 确定搜索起始位置
    size_t start_pos = (global_ || sticky_) ? last_index_ : 0;

    auto result = nfa_->MatchDetail(str, start_pos);

    if (!result.has_value()) {
        // 没有匹配结果，重置 lastIndex 并返回 null
        if (global_ || sticky_) {
            last_index_ = 0;
        }
        return Value(nullptr);
    }

    // sticky 模式下匹配必须从 last_index 位置开始
    if (sticky_ && result->start_pos != start_pos) {
        last_index_ = 0;
        return Value(nullptr);
    }

    // 构建 exec 返回的数组结果
    // GCHandleScope 管理多个临时对象的生命周期
    // result_array + indices_array（匹配结果 + indices 数组）
    size_t scope_size = has_indices_ ? (3 + result->capture_indices.size() + 1) : 2;
    // 如果 scope 数量超过上限，限制为最大值
    if (scope_size > 20) scope_size = 20;

    if (has_indices_) {
        GCHandleScope<20> scope(context);
        auto result_array_handle = scope.New<ArrayObject>(static_cast<uint32_t>(0));
        auto& result_array = *result_array_handle;

        // index 0: 完整匹配的字符串
        result_array.Push(context, Value(String::New(result->matched_text)));

        // 填充捕获组字符串
        for (size_t i = 0; i < result->captures.size(); ++i) {
            if (i < result->capture_indices.size() && result->capture_indices[i].second == SIZE_MAX) {
                result_array.Push(context, Value());
                continue;
            }
            result_array.Push(context, Value(String::New(result->captures[i])));
        }

        // 设置 index 和 input 属性
        result_array.SetProperty(context, ConstIndexEmbedded::kIndex, Value(static_cast<int64_t>(result->start_pos)));
        result_array.SetProperty(context, ConstIndexEmbedded::kInput, Value(String::New(str)));

        // 构建 indices 数组（/d hasIndices 标志）
        auto indices_array_handle = scope.New<ArrayObject>(static_cast<uint32_t>(0));
        auto& indices_array = *indices_array_handle;

        // indices[0] 为完整匹配的起止位置范围
        {
            auto range_handle = scope.New<ArrayObject>(static_cast<uint32_t>(2));
            auto& range = *range_handle;
            range.Push(context, Value(static_cast<int64_t>(result->start_pos)));
            range.Push(context, Value(static_cast<int64_t>(result->end_pos)));
            indices_array.Push(context, scope.Close(range_handle));
        }

        // 为每个捕获组创建 indices 范围
        for (const auto& indices : result->capture_indices) {
            // 处理未匹配的捕获组（end == SIZE_MAX），设置为 undefined
            if (indices.second == SIZE_MAX) {
                // 未匹配的捕获组设置为 undefined（JavaScript 规范）
                indices_array.Push(context, Value());
                continue;
            }

            auto range_handle = scope.New<ArrayObject>(static_cast<uint32_t>(2));
            auto& range = *range_handle;
            range.Push(context, Value(static_cast<int64_t>(indices.first)));
            range.Push(context, Value(static_cast<int64_t>(indices.second)));
            indices_array.Push(context, scope.Close(range_handle));
        }

        result_array.SetProperty(context, ConstIndexEmbedded::kIndices, scope.Close(indices_array_handle));

        // 更新 lastIndex
        if (global_ || sticky_) {
            size_t next_index = result->end_pos;
            if (next_index == start_pos) {
                next_index = AdvanceStringIndex(str, start_pos, unicode_ || unicode_sets_);
            }
            last_index_ = static_cast<uint32_t>(next_index);
        }

        return scope.Close(result_array_handle);
    } else {
        GCHandleScope<1> scope(context);
        auto result_array_handle = scope.New<ArrayObject>(static_cast<uint32_t>(0));
        auto& result_array = *result_array_handle;

        // index 0: 完整匹配的字符串
        result_array.Push(context, Value(String::New(result->matched_text)));

        // 填充捕获组字符串
        for (size_t i = 0; i < result->captures.size(); ++i) {
            if (i < result->capture_indices.size() && result->capture_indices[i].second == SIZE_MAX) {
                result_array.Push(context, Value());
                continue;
            }
            result_array.Push(context, Value(String::New(result->captures[i])));
        }

        // 设置 index 和 input 属性
        result_array.SetProperty(context, ConstIndexEmbedded::kIndex, Value(static_cast<int64_t>(result->start_pos)));
        result_array.SetProperty(context, ConstIndexEmbedded::kInput, Value(String::New(str)));

        // 更新 lastIndex
        if (global_ || sticky_) {
            size_t next_index = result->end_pos;
            if (next_index == start_pos) {
                next_index = AdvanceStringIndex(str, start_pos, unicode_ || unicode_sets_);
            }
            last_index_ = static_cast<uint32_t>(next_index);
        }

        return scope.Close(result_array_handle);
    }
}

std::string RegExpObject::ToString() const {
    std::string result = "/";
    result += pattern_;
    result += "/";
    result += flags_;
    return result;
}

void RegExpObject::GCTraverse(Context* context, GCTraverseCallback callback) {
    Object::GCTraverse(context, callback);
    // RegExpObject 没有直接的 Value 成员需要 GC 追踪
}

} // namespace mjs

