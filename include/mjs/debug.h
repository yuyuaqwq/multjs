#pragma once

#include <mjs/opcode.h>
#include <mjs/source.h>

namespace mjs {

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
    void AddEntry(Pc pc_start, Pc pc_end, SourcePos source_start, SourcePos source_end, SourceLine source_line) {
        assert(entries_.empty() || pc_start >= entries_.back().pc_start);

        entries_.emplace_back(DebugEntry{
            .pc_start = pc_start,
            .pc_end = pc_end,
            .source_start = source_start,
            .source_end = source_end,
            .source_line = source_line,
        });
    }

    const DebugEntry* FindEntry(Pc pc) const {
        if (entries_.empty()) return nullptr;

        auto it = std::lower_bound(entries_.begin(), entries_.end(), pc,
            [](const DebugEntry& entry, Pc pc) {
                return entry.pc_start <= pc;
        });

        if (it != entries_.begin() && it->pc_start > pc) {
            --it;
        }
        
        // ¼ì²éÊÇ·ñÕÒµ½Æ¥ÅäµÄ·¶Î§
        if (pc >= it->pc_start && pc < it->pc_end) {
            return &(*it);
        }

        return nullptr;
    }

private:
    std::vector<DebugEntry> entries_;
};

} // namespace mjs