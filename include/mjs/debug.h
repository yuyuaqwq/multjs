/**
 * @file debug.h
 * @brief JavaScript 引擎调试信息管理
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎的调试信息管理系统，包括调试条目结构
 * 和调试信息表类，用于源代码位置与字节码位置的映射。
 */

#pragma once

#include <mjs/opcode.h>
#include <mjs/source.h>

namespace mjs {

/**
 * @struct DebugEntry
 * @brief 调试条目结构体
 *
 * 存储源代码位置与字节码位置的映射关系，用于调试和错误报告。
 * 每个条目表示一个字节码范围对应的源代码范围。
 */
struct DebugEntry {
    Pc pc_start;                    ///< 字节码起始位置（包含）
    Pc pc_end;                      ///< 字节码结束位置（不包含）
    SourcePos source_start;         ///< 源代码起始位置
    SourcePos source_end;           ///< 源代码结束位置
    SourceLine source_line;         ///< 源代码行号
};

/**
 * @class DebugTable
 * @brief 调试信息表类
 *
 * 管理所有调试条目，提供调试信息的添加、查找和排序功能。
 * 用于将字节码位置映射回源代码位置，支持调试器和错误报告。
 *
 * @note 未来考虑二分优化，需要稳定排序
 * @note 查找时先定位到对应的范围，再顺序查找，定位到最小的范围
 */
class DebugTable {
public:
    /**
     * @brief 添加调试条目
     * @param pc_start 字节码起始位置
     * @param pc_end 字节码结束位置
     * @param source_start 源代码起始位置
     * @param source_end 源代码结束位置
     * @param source_line 源代码行号
     */
    void AddEntry(Pc pc_start, Pc pc_end, SourcePos source_start, SourcePos source_end, SourceLine source_line) {
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

    /**
     * @brief 对调试条目进行排序
     * @note 目前为空实现，未来需要实现稳定排序
     */
    void Sort() {
        // TODO: 实现稳定排序
        // std::sort(entries_.begin(), entries_.end(),
        //     [](const DebugEntry& a, const DebugEntry& b) {
        //         return a.pc_start < b.pc_start;
        // });
    }

    /**
     * @brief 查找指定字节码位置对应的调试条目
     * @param pc 字节码位置
     * @return 调试条目指针，如果未找到则返回 nullptr
     */
    const DebugEntry* FindEntry(Pc pc) const {
        if (entries_.empty()) return nullptr;

        // TODO: 实现二分查找优化
        // auto it = std::lower_bound(entries_.begin(), entries_.end(), pc,
        //     [](const DebugEntry& entry, Pc pc) {
        //         return entry.pc_start <= pc;
        // });
        //
        // if (it != entries_.begin() && it->pc_start > pc) {
        //     --it;
        // }
        //
        // // 检查是否找到匹配的范围
        // if (pc >= it->pc_start && pc < it->pc_end) {
        //     return &(*it);
        // }

        // 当前使用顺序查找
        for (auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (pc >= it->pc_start && pc < it->pc_end) {
                return &(*it); // 返回匹配的条目
            }
        }

        return nullptr;
    }

private:
    std::vector<DebugEntry> entries_; ///< 调试条目向量
};

} // namespace mjs