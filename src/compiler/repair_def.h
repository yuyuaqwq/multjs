#pragma once

#include <mjs/opcode.h>

namespace mjs {

namespace compiler {

/**
    * @brief 需要修复的跳转指令条目
    */
struct RepairEntry {
    enum class Type {
        kBreak,    ///< break语句
        kContinue, ///< continue语句
    };

    Type type;     ///< 类型
    Pc repair_pc;  ///< 需要修复的PC
};




} // namespace compiler

} // namespace mjs