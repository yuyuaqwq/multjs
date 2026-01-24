#pragma once

#include <mjs/constant.h>

namespace mjs {

class ConstIndexEmbedded {
public:
    enum : ConstIndex {
        kNone = 0,

        kProto,         // __proto__
        kPrototype,     // prototype
        kConstructor,   // constructor

        kValue,         // value
        kDone,          // done
        kNext,          // next

        kThen,          // then
        kResolve,       // resolve
        kReject,        // reject

        kSplit,         // split
        kSubString,     // substring
        kIndexOf,       // indexOf
        kToLowerCase,   // toLowerCase
        kToUpperCase,   // toUpperCase
        kTrim,          // trim
        kReplace,       // replace

        kFor,           // for

        kFreeze,        // freeze
        kSeal,          // seal
        kPreventExtensions, // preventExtensions
        kDefineProperty, // defineProperty

        kLength,        // length
        kOf,            // of
        kPush,          // push
        kPop,           // pop
        kForEach,       // forEach
        kMap,           // map
        kFilter,        // filter
        kReduce,        // reduce

        kEnd,
    };
};

} // namespace mjs