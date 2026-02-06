#pragma once

#include <mjs/value/object/object.h>
#include <mjs/class_def/array_object_class_def.h>
#include <vector>

namespace mjs {

class ArrayObject : public Object {
private:
    ArrayObject(Context* context);

    ArrayObject(Context* context, size_t count);

    ArrayObject(Context* context, std::initializer_list<Value> values);

    // 数组元素在 properties_ 中的偏移量
    // 前4个位置预留给额外属性（如 __proto__ 等）
    static constexpr size_t kArrayElementOffset = 4;

    // 检查是否是有效的数组索引（符合 JS 规范：32位无符号整数）
    static bool IsValidArrayIndex(uint64_t index) {
        return index < 4294967296ULL; // 2^32
    }

    // 尝试将字符串转换为数组索引
    // 返回 true 表示成功转换为有效索引，false 表示不是有效的数组索引字符串
    static bool TryStringToArrayIndex(std::string_view str, uint64_t* out_index);

    uint32_t length_ = 0;             // 数组长度
    struct {
        uint32_t is_fast_array_ : 1;

    };

    // 检查索引是否在连续数组范围内
    bool IsInArrayRange(size_t index) const {
        return index < length_;
    }

    // 退化到哈希表模式（当额外属性超过限制时）
    void TransitionToSlowMode(Context* context);

    // 确保数组有足够的额外属性空间
    bool EnsureExtraPropertySpace(Context* context);

public:
    bool GetProperty(Context* context, ConstIndex key, Value* value) override;

    void SetProperty(Context* context, ConstIndex key, Value&& value) override;

    bool HasProperty(Context* context, ConstIndex key) override;

    bool GetComputedProperty(Context* context, const Value& key, Value* value) override;

    void SetComputedProperty(Context* context, const Value& key, Value&& val) override;

    bool DelComputedProperty(Context* context, const Value& key, Value* value) override;

    void Push(Context* context, Value val);

    Value Pop(Context* context);

    void ForEach(Context* context, Value callback);

    // 获取数组长度
    size_t GetLength() const;

private:
    friend class GCManager;
};

} // namespace mjs