/**
 * @file regexp_object.cpp
 * @brief 婵繐绲介崹顖滄偘閵娿劍褰х€殿喖绻愰顔炬寬閳ュ磭鏉介柣?
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

    // 閻熸瑱绲鹃悗浠嬪冀閸パ呯
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
                // 闊洨鏅弳鎰板籍閻樿櫕娅忛柡宥呮搐缁?
                break;
        }
    }

    // 缂傚倹鐗為惁褍顫㈤敐鍛仧閻炴稏鍔忛幓顏勵嚕?
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

    // 濞寸姴绐卆st_index鐎殿喒鍋撳┑顔碱儏鐏忣噣鏌?
    size_t start_pos = (global_ || sticky_) ? last_index_ : 0;

    auto result = nfa_->MatchDetail(str, start_pos);

    if (result.has_value()) {
        // 闁革腹鏆瀟icky婵☆垪鈧磭纭€濞戞挸顑戠槐婵婄疀閸涙番鈧繑绂掑姊゛rt_pos鐎殿喒鍋撳┑顔碱儏鐏忣噣鏌?
        if (sticky_ && result->start_pos != start_pos) {
            // Sticky mode requires the match to start at exactly start_pos
            last_index_ = 0;
            return false;
        }

        // 闁哄洤鐡ㄩ弻濡塧stIndex
        if (global_ || sticky_) {
            size_t next_index = result->end_pos;
            if (next_index == start_pos) {
                next_index = AdvanceStringIndex(str, start_pos, unicode_ || unicode_sets_);
            }
            last_index_ = static_cast<uint32_t>(next_index);
        }
        return true;
    }

    // 闂佹彃绉堕悿鍞媋stIndex
    if (global_ || sticky_) {
        last_index_ = 0;
    }

    return false;
}

Value RegExpObject::Exec(Context* context, const std::string& str) {
    if (!nfa_) {
        return Value(nullptr);
    }

    // 濞寸姴绐卆st_index鐎殿喒鍋撳┑顔碱儏鐏忣噣鏌?
    size_t start_pos = (global_ || sticky_) ? last_index_ : 0;

    auto result = nfa_->MatchDetail(str, start_pos);

    if (!result.has_value()) {
        // 闁告牕缍婇崢銈嗗緞鏉堫偉袝闁挎稑鐭傞崳鍝ョ磾閻㈢禈stIndex
        if (global_ || sticky_) {
            last_index_ = 0;
        }
        return Value(nullptr);
    }

    // 闁革腹鏆瀟icky婵☆垪鈧磭纭€濞戞挸顑戠槐婵婄疀閸涙番鈧繑绂掑姊゛rt_pos鐎殿喒鍋撳┑顔碱儏鐏忣噣鏌?
    if (sticky_ && result->start_pos != start_pos) {
        last_index_ = 0;
        return Value(nullptr);
    }

    // 闁告帗绋戠紓鎾剁磼閹惧浜柡浣瑰缁?
    // GCHandleScope闂傚洠鍋撻悷鏇氭祰閸愮粯寰勯悢鏋海濞寸姰鍎遍鎰棯閾忣偄顣查柡鍫濐槷婢跺秹寮粔鍢簄dle
    // result_array + indices_array (闁告瑯鍨堕埀? + 濞戞挸鐡ㄥ淇絘nge objects
    size_t scope_size = has_indices_ ? (3 + result->capture_indices.size() + 1) : 2;
    // 闂傚嫭鍔曢崺妤呭嫉閳ь剚寰勯—绛﹐pe濠㈠爢鍐瘓濞寸姰鍎垫导鈺呭礂瀹ュ棛鍨芥繝褋鍨归崵?
    if (scope_size > 20) scope_size = 20;

    if (has_indices_) {
        GCHandleScope<20> scope(context);
        auto result_array_handle = scope.New<ArrayObject>(static_cast<uint32_t>(0));
        auto& result_array = *result_array_handle;

        // index 0: 閻庣懓鏈弳锝夊礌瑜版帒甯抽柣銊ュ閺嬪啴寮?
        result_array.Push(context, Value(String::New(result->matched_text)));

        // 婵烇綀顕ф慨鐐哄箲閺団€崇缂?
        for (size_t i = 0; i < result->captures.size(); ++i) {
            if (i < result->capture_indices.size() && result->capture_indices[i].second == SIZE_MAX) {
                result_array.Push(context, Value());
                continue;
            }
            result_array.Push(context, Value(String::New(result->captures[i])));
        }

        // 閻犱礁澧介悿鍡涘极閹殿喚鐭嬮柣銊ュ閻﹢骞€?
        result_array.SetProperty(context, ConstIndexEmbedded::kIndex, Value(static_cast<int64_t>(result->start_pos)));
        result_array.SetProperty(context, ConstIndexEmbedded::kInput, Value(String::New(str)));

        // 濠碘€冲€归悘澶愬触椤栨粍鏆忓ù婊冩敡asIndices (/d闁哄秴娲ょ换?闁挎稑鏈崸濠囧礉閻栧埖dices闁轰焦澹嗙划?
        auto indices_array_handle = scope.New<ArrayObject>(static_cast<uint32_t>(0));
        auto& indices_array = *indices_array_handle;

        // indices[0] 闁哄嫷鍨遍弳锝嗘媴閹惧啿鐖遍梺鏉跨Ф濞堟垿鎳犻崘銊︾函
        {
            auto range_handle = scope.New<ArrayObject>(static_cast<uint32_t>(2));
            auto& range = *range_handle;
            range.Push(context, Value(static_cast<int64_t>(result->start_pos)));
            range.Push(context, Value(static_cast<int64_t>(result->end_pos)));
            indices_array.Push(context, scope.Close(range_handle));
        }

        // 濞戞挾鍎ら惁鈩冪▔椤忓懎绀嬮柤楣冾棑缁秴菐鐠囨彃顫ｇ紒渚垮灩缁扁晠鎳犻崘銊︾函
        for (const auto& indices : result->capture_indices) {
            // 濠碘€冲€归悘澶愬箲閺団€崇缂備礁瀚﹢顓㈠礌瑜版帒甯抽柨娑樼暥nd濞村吋纰嶅Σ绐糏ZE_MAX闁挎稑鏈婵嬪籍閺堥潧鈻忛柣鈶╂殸ndefined
            if (indices.second == SIZE_MAX) {
                // 闁哄牜浜濆畷鐔兼嚔閸戙倗绀夊ù锝堟硶閺侇槢ndefined (JavaScript闁哄秴娲ら崳顖滄偘鐏炶壈绀?
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

        // 闁哄洤鐡ㄩ弻濡塧stIndex
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

        // index 0: 閻庣懓鏈弳锝夊礌瑜版帒甯抽柣銊ュ閺嬪啴寮?
        result_array.Push(context, Value(String::New(result->matched_text)));

        // 婵烇綀顕ф慨鐐哄箲閺団€崇缂?
        for (size_t i = 0; i < result->captures.size(); ++i) {
            if (i < result->capture_indices.size() && result->capture_indices[i].second == SIZE_MAX) {
                result_array.Push(context, Value());
                continue;
            }
            result_array.Push(context, Value(String::New(result->captures[i])));
        }

        // 閻犱礁澧介悿鍡涘极閹殿喚鐭嬮柣銊ュ閻﹢骞€?
        result_array.SetProperty(context, ConstIndexEmbedded::kIndex, Value(static_cast<int64_t>(result->start_pos)));
        result_array.SetProperty(context, ConstIndexEmbedded::kInput, Value(String::New(str)));

        // 闁哄洤鐡ㄩ弻濡塧stIndex
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
    // RegExpObject闁烩晩鍠栨晶鐘测柦閳╁啯绠扸alue闁瑰瓨鍔曢幉鎶芥閳ь剛鎲版笟鈧禍鍫曞储?
}

} // namespace mjs

