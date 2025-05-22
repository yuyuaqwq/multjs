#pragma once

#include <mjs/opcode.h>
#include <mjs/source_def.h>

namespace mjs {

class FunctionDef;
struct DebugEntry {
    // ×ó±ÕÓÒ¿ª
    Pc pc_start;
    Pc pc_end;
    SourcePos source_start;
    SourcePos source_end;
    SourceLine source_line;
};

class DebugTable {
public:
    void AddEntry(Pc pc_start, Pc pc_end, SourcePos source_start, SourcePos source_end) {
        entries_.emplace_back(DebugEntry{
            .pc_start = pc_start,
            .pc_end = pc_end,
            .source_start = source_start,
            .source_end = source_end,
            .source_line = 0,
        });
    }

    const DebugEntry& FindEntry(Pc pc) const {
        for (auto& entity : entries_) {
            if (pc >= entity.pc_start && pc < entity.pc_end) {
                return entity;
            }
        }
    }

private:
    std::vector<DebugEntry> entries_;
};

} // namespace mjs