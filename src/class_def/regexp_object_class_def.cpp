/**
 * @file regexp_object_class_def.cpp
 * @brief 正则表达式对象类定义实现
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <mjs/class_def/regexp_object_class_def.h>
#include <mjs/stack_frame.h>
#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/gc/handle.h>
#include <mjs/regexp/regexp_object.h>
#include <mjs/value/string.h>
#include <mjs/error.h>
#include <format>

namespace mjs {

RegExpObjectClassDef::RegExpObjectClassDef(Runtime* runtime)
    : ClassDef(runtime, ClassId::kRegExpObject, "RegExp") {
    prototype_.object().SetPrototype(&runtime->default_context(),
                                     runtime->class_def_table()[ClassId::kObject].prototype());

    constructor_.object().SetPrototype(&runtime->default_context(),
                                       runtime->class_def_table()[ClassId::kFunctionObject].prototype());

    prototype_.object().SetProperty(&runtime->default_context(),
                                    ConstIndexEmbedded::kTest,
                                    Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
        return RegExpObjectClassDef::Test(context, par_count, stack);
    }));

    prototype_.object().SetProperty(&runtime->default_context(),
                                    ConstIndexEmbedded::kExec,
                                    Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
        return RegExpObjectClassDef::Exec(context, par_count, stack);
    }));

    prototype_.object().SetProperty(&runtime->default_context(),
                                    ConstIndexEmbedded::kToString,
                                    Value([](Context* context, uint32_t par_count, const StackFrame& stack) -> Value {
        return RegExpObjectClassDef::ToString(context, par_count, stack);
    }));
}

Value RegExpObjectClassDef::NewConstructor(Context* context, uint32_t par_count, const StackFrame& stack) const {
    return RegExpConstructor(context, par_count, stack);
}

Value RegExpObjectClassDef::RegExpConstructor(Context* context, uint32_t par_count, const StackFrame& stack) {
    std::string pattern;
    std::string flags;

    if (par_count >= 1) {
        auto& pattern_val = stack.get(0);

        if (pattern_val.IsRegExpObject()) {
            auto& regexp = pattern_val.regexp();
            if (par_count < 2) {
                GCHandleScope<1> scope(context);
                auto clone = scope.New<RegExpObject>(regexp.pattern(), regexp.flags());
                return scope.Close(clone);
            }
            pattern = regexp.pattern();
        } else if (pattern_val.IsString()) {
            pattern = pattern_val.string_view();
        } else {
            return TypeError::Throw(context, "RegExp pattern must be a string");
        }
    }

    if (par_count >= 2) {
        auto& flags_val = stack.get(1);
        if (flags_val.IsString()) {
            flags = flags_val.string_view();
        } else if (!flags_val.IsUndefined()) {
            return TypeError::Throw(context, "RegExp flags must be a string");
        }
    }

    if (!flags.empty()) {
        std::string error = RegExpObject::ValidateFlags(flags);
        if (!error.empty()) {
            return SyntaxError::Throw(context, "{}", error);
        }
    }

    GCHandleScope<1> scope(context);
    auto regexp = scope.New<RegExpObject>(pattern, flags);
    return scope.Close(regexp);
}

Value RegExpObjectClassDef::Test(Context* context, uint32_t par_count, const StackFrame& stack) {
    if (par_count < 1) {
        return TypeError::Throw(context, "RegExp.test requires a string argument");
    }

    auto& regexp = stack.this_val().regexp();
    auto& str_val = stack.get(0);

    if (!str_val.IsString()) {
        auto str = str_val.ToString(context);
        if (str.IsException()) {
            return str;
        }
        return Value(regexp.Test(context, str.string_view()));
    }

    return Value(regexp.Test(context, str_val.string_view()));
}

Value RegExpObjectClassDef::Exec(Context* context, uint32_t par_count, const StackFrame& stack) {
    if (par_count < 1) {
        return TypeError::Throw(context, "RegExp.exec requires a string argument");
    }

    auto& regexp = stack.this_val().regexp();
    auto& str_val = stack.get(0);

    if (!str_val.IsString()) {
        auto str = str_val.ToString(context);
        if (str.IsException()) {
            return str;
        }
        return regexp.Exec(context, str.string_view());
    }

    return regexp.Exec(context, str_val.string_view());
}

Value RegExpObjectClassDef::ToString(Context* context, uint32_t par_count, const StackFrame& stack) {
    auto& regexp = stack.this_val().regexp();
    auto str = regexp.ToString();
    return Value(String::New(str));
}

} // namespace mjs