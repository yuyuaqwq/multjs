#pragma once

#include <mjs/opcode.h>

namespace mjs {

using ExceptionIdx = uint32_t;
constexpr ExceptionIdx kExceptionInvalidIdx = 0xffffffff;

struct ExceptionEntry{
    Pc try_start_pc = kInvalidPc;
    Pc try_end_pc = kInvalidPc;
    Pc catch_start_pc = kInvalidPc;
    Pc catch_end_pc = kInvalidPc;
    VarIndex catch_err_var_idx = kVarInvaildIndex;
    Pc finally_start_pc = kInvalidPc;
    Pc finally_end_pc = kInvalidPc;

    // 辅助方法：检查PC是否在try块范围内
    bool Contains(Pc pc) const {
        return pc >= try_start_pc && pc < finally_end_pc;  // 左开右闭
    }

    // 检查是否有catch处理程序
    bool HasCatch() const {
        return catch_start_pc != kInvalidPc;
    }

    // 检查是否有finally处理程序
    bool HasFinally() const {
        return finally_start_pc != kInvalidPc;
    }

    bool LocatedInTry(Pc pc) const {
        return pc >= try_start_pc && pc < try_end_pc;
    }

    bool LocatedInCatch(Pc pc) const {
        return pc >= catch_start_pc && pc < catch_end_pc;
    }

    bool LocatedInFinally(Pc pc) const {
        return pc >= finally_start_pc && pc < finally_end_pc;
    }
};

class ExceptionTable {
public:
    ExceptionIdx AddEntry(ExceptionEntry&& entry) {
        entries_.emplace_back(std::move(entry));
        return entries_.size() - 1;
    }

    ExceptionEntry& GetEntry(ExceptionIdx idx) {
        return entries_.at(idx);
    }

    ExceptionEntry* FindEntry(Pc throw_pc) {
        // 反向查找，确保最近嵌套的try块优先匹配
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->Contains(throw_pc)) {
                return &(*it); // 返回匹配的条目
            }
        }
        return nullptr; // 未找到匹配的try块
    }

#undef max
    // 查找特定PC范围内的最内层条目
    ExceptionEntry* FindInnermostEntry(Pc start_pc, Pc end_pc) {
        ExceptionEntry* innermost = nullptr;
        size_t min_range = std::numeric_limits<size_t>::max();

        for (auto& entry : entries_) {
            if (entry.try_start_pc >= start_pc && entry.try_end_pc <= end_pc) {
                size_t range = entry.try_end_pc - entry.try_start_pc;
                if (range < min_range) {
                    min_range = range;
                    innermost = &entry;
                }
            }
        }
        return innermost;
    }

    // 获取所有条目（只读）
    const std::vector<ExceptionEntry>& GetEntries() const {
        return entries_;
    }

    // 清空异常表
    void Clear() {
        entries_.clear();
    }

private:
    std::vector<ExceptionEntry> entries_;
};

} // namespace mjs