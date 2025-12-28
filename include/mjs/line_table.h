/**
 * @file line_table.h
 * @brief JavaScript 行号映射表定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的行号映射表类，用于源代码位置转换和
 * 调试信息处理，支持将字节位置转换为行号和列号（1-based）。
 */

#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <utility>
#include <stdexcept>

#include <mjs/source_define.h>

namespace mjs {

/**
 * @class LineTable
 * @brief 行号映射表类
 *
 * 提供源代码行号映射功能，支持源代码预处理和位置转换。
 * 用于将字节位置转换为行号和列号（1-based）。
 */
class LineTable {
public:
    /**
     * @brief 预处理源码，构建行号映射表
     * @param source 源代码字符串视图
     */
    void Build(std::string_view source) {
        line_offsets_.clear();
        line_offsets_.push_back(0); // 第1行从0开始

        for (SourcePosition pos = 0; pos < source.size(); ++pos) {
            if (source[pos] == '\n') {
                line_offsets_.push_back(pos + 1); // 下一行起始位置（跳过\n）
            }
        }
    }

    /**
     * @brief 将字节位置转换为行号和列号（1-based）
     * @param pos 字节位置
     * @return 行号和列号对
     * @throw std::out_of_range 当位置超出范围时抛出
     */
    std::pair<SourceLine, SourceColumn> PosToLineAndColumn(SourcePosition pos) const {
        if (line_offsets_.empty()) {
            throw std::out_of_range("LineTable is not initialized");
        }

        // 查找第一个大于等于pos的行起始位置
        auto it = std::lower_bound(line_offsets_.begin(), line_offsets_.end(), pos);

        if (it == line_offsets_.end()) {
            --it;
        }

        if (*it > pos) {
            if (it == line_offsets_.begin()) {
                throw std::out_of_range("Position is beyond the last line");
            }
            --it;
        }

        SourceLine line = static_cast<SourceLine>(std::distance(line_offsets_.begin(), it)) + 1;
        SourceColumn column = pos - *it; // 转为1-based列号
        return { line, column };
    }

private:
    std::vector<SourcePosition> line_offsets_; ///< 存储每行的起始偏移量（0-based）
};

} // namespace mjs