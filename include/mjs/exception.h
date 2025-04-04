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

    // �������������PC�Ƿ���try�鷶Χ��
    bool Contains(Pc pc) const {
        return pc >= try_start_pc && pc < finally_end_pc;  // ���ұ�
    }

    // ����Ƿ���catch�������
    bool HasCatch() const {
        return catch_start_pc != kInvalidPc;
    }

    // ����Ƿ���finally�������
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
        // ������ң�ȷ�����Ƕ�׵�try������ƥ��
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->Contains(throw_pc)) {
                return &(*it); // ����ƥ�����Ŀ
            }
        }
        return nullptr; // δ�ҵ�ƥ���try��
    }

#undef max
    // �����ض�PC��Χ�ڵ����ڲ���Ŀ
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

    // ��ȡ������Ŀ��ֻ����
    const std::vector<ExceptionEntry>& GetEntries() const {
        return entries_;
    }

    // ����쳣��
    void Clear() {
        entries_.clear();
    }

private:
    std::vector<ExceptionEntry> entries_;
};

} // namespace mjs