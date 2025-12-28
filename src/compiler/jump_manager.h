#include <vector>

#include <mjs/noncopyable.h>

#include "repair_def.h"

namespace mjs {

namespace compiler {

/**
 * @brief 标签信息
 */
struct LabelInfo {
    Pc current_loop_start_pc = kInvalidPc;      ///< 当前循环开始PC
    std::vector<RepairEntry> entries;           ///< 需要修复的条目
};

class JumpManager : public noncopyable {
public:
    std::vector<RepairEntry>* current_loop_repair_entries() { return current_loop_repair_entries_; }

    void set_current_loop_repair_entries(std::vector<RepairEntry>* current_loop_repair_entries) { 
        current_loop_repair_entries_ = current_loop_repair_entries;
    }

    std::unordered_map<std::string, LabelInfo>& label_map() { return label_map_; }

    std::optional<Pc>& current_label_reloop_pc() { return current_label_reloop_pc_; }

    void set_current_label_reloop_pc(std::optional<Pc> current_label_reloop_pc) {
        current_label_reloop_pc_ = current_label_reloop_pc;
    }

private:
    std::vector<RepairEntry>* current_loop_repair_entries_ = nullptr; ///< 当前循环需要修复的条目

    std::unordered_map<std::string, LabelInfo> label_map_; ///< 标签映射
    std::optional<Pc> current_label_reloop_pc_;     ///< 当前标签重新循环PC
};

} // namespace compiler

} // namespace mjs