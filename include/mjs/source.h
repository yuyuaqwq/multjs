#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <utility>
#include <stdexcept>

namespace mjs {

using SourcePos = uint32_t;
using SourceLine = uint32_t;
using SourceColumn = uint32_t;

constexpr SourcePos kInvalidSourcePos = 0xffffffff;
constexpr SourceLine kInvalidSourceLine = 0;

using Source = std::string;

class LineTable {
public:
    // 预处理源码，构建行号映射表
    void Build(std::string_view source) {
        line_offsets_.clear();
        line_offsets_.push_back(0); // 第1行从0开始

        for (SourcePos pos = 0; pos < source.size(); ++pos) {
            if (source[pos] == '\n') {
                line_offsets_.push_back(pos + 1); // 下一行起始位置（跳过\n）
            }
        }
    }

    // 将字节位置pos转换为行号和列号（1-based）
    std::pair<SourceLine, SourceColumn> PosToLineAndColumn(SourcePos pos) const {
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
    std::vector<SourcePos> line_offsets_; // 存储每行的起始偏移量（0-based）

};

} // namespace mjs