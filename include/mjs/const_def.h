#pragma once

#include <cstdint>
#include <cassert>

namespace mjs {

// 0表示无效
// 正数，从1开始，表示全局常量池索引
// 负数，从-1开始，表示局部常量池索引
class ConstIndex {
public:
    ConstIndex()
        : value_(0) {}

    explicit ConstIndex(int32_t value)
        : value_(value) {}

    bool operator==(const ConstIndex& rhs) const {
        return value_ == rhs.value_;
    }

    operator size_t() const {
        assert(value_ >= 0 && "ConstIndex must be non-negative for vector indexing");
        return static_cast<size_t>(value_);
    }

    // 前置 ++（返回自增后的引用）
    ConstIndex& operator++() {
        ++value_;
        return *this;
    }

    bool is_invalid() const { return value_ == 0; }

    bool is_global_index() const { return value_ > 0; }

    bool is_local_index() const { return value_ < 0; }

    ConstIndex to_global_index() const { return ConstIndex(value_); }

    ConstIndex to_local_index() const { return ConstIndex(-value_); }

    void from_global_index() { assert(is_global_index()); }
    
    void from_local_index() { assert(is_local_index()); value_ = -value_; }

    int32_t value() const { return value_; }

    bool is_same_pool(ConstIndex rhs) const { 
        return (value_ & 0x80000000) == (rhs.value_ & 0x80000000);
    }

private:
    int32_t value_;
};

} // namespace mjs 

namespace std {
    template<>
    struct hash<mjs::ConstIndex> {
        size_t operator()(const mjs::ConstIndex& idx) const {
            return std::hash<uint32_t>()(idx.value());
        }
    };
}