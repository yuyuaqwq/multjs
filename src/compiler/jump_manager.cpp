#include "src/compiler/jump_manager.h"

#include <mjs/error.h>
#include <mjs/value/function_def.h>

namespace mjs {
namespace compiler {

void JumpManager::RepairEntries(FunctionDefBase* function_def_base, const std::vector<RepairEntry>& entries, Pc end_pc, Pc reloop_pc) {
    for (auto& repair_info : entries) {
        switch (repair_info.type) {
        case RepairEntry::Type::kBreak: {
            function_def_base->bytecode_table().RepairPc(repair_info.repair_pc, end_pc);
            break;
        }
        case RepairEntry::Type::kContinue: {
            assert(reloop_pc != kInvalidPc);
            function_def_base->bytecode_table().RepairPc(repair_info.repair_pc, reloop_pc);
            break;
        }
        default:
            throw SyntaxError("Incorrect type.");
            break;
        }
    }
}


} // namespace compiler
} // namespace mjs