#pragma once

#include <mjs/value/object/object.h>
#include <mjs/class_def/array_object_class_def.h>

namespace mjs {

class ArrayObject : public Object {
private:
    ArrayObject(Context* context);

    ArrayObject(Context* context, size_t count);

    ArrayObject(Context* context, std::initializer_list<Value> values);

    // 设置 length 属性值（内部使用）
    void SetLengthValue(size_t new_length);

    // 检查是否是有效的数组索引（符合 JS 规范：32位无符号整数）
    static bool IsValidArrayIndex(uint64_t index) {
        return index < 4294967296ULL; // 2^32
    }

    // 将值转换为数组索引
    static bool ToArrayIndex(const Value& key, uint64_t* out_index);

    // 缓存 length 属性的 slot 索引（用于快速访问）
    mutable PropertySlotIndex length_slot_index_ = kPropertySlotIndexInvalid;

public:
    bool GetProperty(Context* context, ConstIndex key, Value* value) override;

    void SetProperty(Context* context, ConstIndex key, Value&& value) override;

    void Push(Context* context, Value val);

    Value Pop(Context* context);

    void ForEach(Context* context, Value callback);

    // 获取数组长度
    size_t GetLength() const;

    // 数组元素访问
    Value& At(Context* context, size_t index);

private:
    friend class GCManager;
};

} // namespace mjs