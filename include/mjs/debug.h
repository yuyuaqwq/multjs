#pragma once

#include <mjs/opcode.h>
#include <mjs/source.h>

namespace mjs {

struct DebugEntry {
    // ����ҿ�
    Pc pc_start;
    Pc pc_end;
    SourcePos source_start;
    SourcePos source_end;
    SourceLine source_line;
};


// δ�����Ƕ����Ż�����Ҫ�ȶ�����
// ����ʱ�ȶ�λ����Ӧ�ķ�Χ����˳����ң���λ����С�ķ�Χ
class DebugTable {
public:
    void AddEntry(Pc pc_start, Pc pc_end, SourcePos source_start, SourcePos source_end, SourceLine source_line) {
        // assert(entries_.empty() || pc_start > entries_.back().pc_start);
        if (pc_start == pc_end) return;
        assert(pc_start < pc_end);
        entries_.emplace_back(DebugEntry{
            .pc_start = pc_start,
            .pc_end = pc_end,
            .source_start = source_start,
            .source_end = source_end,
            .source_line = source_line,
        });
    }

    void Sort() {
        //std::sort(entries_.begin(), entries_.end(),
        //    [](const DebugEntry& a, const DebugEntry& b) {
        //        return a.pc_start < b.pc_start;
        //});
    }

    const DebugEntry* FindEntry(Pc pc) const {
        if (entries_.empty()) return nullptr;

        //auto it = std::lower_bound(entries_.begin(), entries_.end(), pc,
        //    [](const DebugEntry& entry, Pc pc) {
        //        return entry.pc_start <= pc;
        //});

        //if (it != entries_.begin() && it->pc_start > pc) {
        //    --it;
        //}
        //
        //// ����Ƿ��ҵ�ƥ��ķ�Χ
        //if (pc >= it->pc_start && pc < it->pc_end) {
        //    return &(*it);
        //}

        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (pc >= it->pc_start && pc < it->pc_end) {
                return &(*it); // ����ƥ�����Ŀ
            }
        }

        return nullptr;
    }

private:
    std::vector<DebugEntry> entries_;
};

} // namespace mjs