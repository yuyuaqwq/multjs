/**
 * @file code_emitter.h
 * @brief C++代码发射器，负责代码格式化和缩进管理
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#pragma once

#include <string>
#include <sstream>
#include <vector>

namespace mjs {
namespace compiler {
namespace cpp_gen {

/**
 * @class CodeEmitter
 * @brief 代码发射器，辅助生成格式化的C++代码
 */
class CodeEmitter {
public:
    /**
     * @brief 构造函数
     * @param indent_size 缩进空格数，默认4个空格
     */
    explicit CodeEmitter(int indent_size = 4);

    /**
     * @brief 增加缩进级别
     */
    void Indent();

    /**
     * @brief 减少缩进级别
     */
    void Dedent();

    /**
     * @brief 发射一行代码（自动添加换行符）
     * @param code 代码内容
     */
    void EmitLine(const std::string& code);

    /**
     * @brief 发射原始代码（不添加缩进和换行）
     * @param code 代码内容
     */
    void EmitRaw(const std::string& code);

    /**
     * @brief 发射空行
     */
    void EmitBlankLine();

    /**
     * @brief 开始一个代码块（发射 "{" 并增加缩进）
     */
    void EmitBlockStart();

    /**
     * @brief 结束一个代码块（减少缩进并发射 "}"）
     */
    void EmitBlockEnd();

    /**
     * @brief 发射块开始（不自动增加缩进，用于如 if (cond) { 的场景）
     */
    void EmitBlockStartNoIndent();

    /**
     * @brief 发射块结束（不自动减少缩进）
     */
    void EmitBlockEndNoDedent();

    /**
     * @brief 获取当前缩进级别
     */
    int GetCurrentIndentLevel() const { return indent_level_; }

    /**
     * @brief 获取生成的完整代码
     */
    std::string ToString() const;

    /**
     * @brief 清空已生成的代码
     */
    void Clear();

    /**
     * @brief 获取当前缩进字符串
     */
    std::string GetCurrentIndent() const;

private:
    std::ostringstream stream_;
    int indent_level_;
    int indent_size_;
    bool at_line_start_;  // 当前是否在行首
};

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs
