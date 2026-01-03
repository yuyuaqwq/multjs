/**
 * @file code_emitter.cpp
 * @brief CodeEmitter实现
 */

#include <algorithm>

#include "cpp_gen/code_emitter.h"

namespace mjs {
namespace compiler {
namespace cpp_gen {

CodeEmitter::CodeEmitter(int indent_size)
    : indent_level_(0), indent_size_(indent_size), at_line_start_(true) {}

void CodeEmitter::Indent() {
    ++indent_level_;
}

void CodeEmitter::Dedent() {
    if (indent_level_ > 0) {
        --indent_level_;
    }
}

void CodeEmitter::EmitLine(const std::string& code) {
    if (at_line_start_) {
        stream_ << GetCurrentIndent();
        at_line_start_ = false;
    }
    stream_ << code << "\n";
    at_line_start_ = true;
}

void CodeEmitter::EmitRaw(const std::string& code) {
    stream_ << code;
    if (!code.empty() && code.back() == '\n') {
        at_line_start_ = true;
    } else {
        at_line_start_ = false;
    }
}

void CodeEmitter::EmitBlankLine() {
    stream_ << "\n";
    at_line_start_ = true;
}

void CodeEmitter::EmitBlockStart() {
    EmitLine("{");
    Indent();
}

void CodeEmitter::EmitBlockEnd() {
    Dedent();
    EmitLine("}");
}

void CodeEmitter::EmitBlockStartNoIndent() {
    if (at_line_start_) {
        stream_ << GetCurrentIndent();
        at_line_start_ = false;
    }
    stream_ << " {\n";
    at_line_start_ = true;
}

void CodeEmitter::EmitBlockEndNoDedent() {
    Dedent();
    if (at_line_start_) {
        stream_ << GetCurrentIndent();
        at_line_start_ = false;
    }
    stream_ << "}\n";
    at_line_start_ = true;
}

std::string CodeEmitter::ToString() const {
    return stream_.str();
}

void CodeEmitter::Clear() {
    stream_.str("");
    stream_.clear();
    indent_level_ = 0;
    at_line_start_ = true;
}

std::string CodeEmitter::GetCurrentIndent() const {
    return std::string(indent_level_ * indent_size_, ' ');
}

} // namespace cpp_gen
} // namespace compiler
} // namespace mjs
