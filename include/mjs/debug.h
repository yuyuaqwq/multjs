#pragma once

#include <mjs/opcode.h>
#include <mjs/source.h>

namespace mjs {

struct DebugEntry {
    // 左闭右开
    Pc pc_start;
    Pc pc_end;
    SourcePos source_start;
    SourcePos source_end;
    SourceLine source_line;
};


// 未来考虑二分优化，需要稳定排序
// 查找时先定位到对应的范围，再顺序查找，定位到最小的范围
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
        //// 检查是否找到匹配的范围
        //if (pc >= it->pc_start && pc < it->pc_end) {
        //    return &(*it);
        //}

        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (pc >= it->pc_start && pc < it->pc_end) {
                return &(*it); // 返回匹配的条目
            }
        }

        return nullptr;
    }

private:
    std::vector<DebugEntry> entries_;
};

} // namespace mjs