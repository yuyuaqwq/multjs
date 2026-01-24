#include <mjs/object_impl/array_object.h>

#include <mjs/context.h>
#include <mjs/error.h>

namespace mjs {

ArrayObject::ArrayObject(Context* context)
    : Object(context, ClassId::kArrayObject)
    , length_slot_index_(kPropertySlotIndexInvalid) {
    InitLengthProperty(context);
}

void ArrayObject::InitLengthProperty(Context* context) {
    // 数组创建时，length 属性初始化为 0
    SetLengthValue(context, 0);
}

void ArrayObject::SetLengthValue(Context* context, size_t new_length) {
    uint32_t flags = ShapeProperty::kWritable | ShapeProperty::kConfigurable; // length 不可枚举

    // 检查 length 属性是否已存在
    auto index = shape_->Find(ConstIndexEmbedded::kLength);
    if (index == kPropertySlotIndexInvalid) {
        // 不存在，添加新属性
        index = shape_->shape_manager()->AddProperty(&shape_, ShapeProperty(ConstIndexEmbedded::kLength));
        AddPropertySlot(index, Value(static_cast<int64_t>(new_length)), flags);
    } else {
        // 已存在，更新值
        SetPropertyValue(index, Value(static_cast<int64_t>(new_length)));
    }

    // 缓存 length 属性的 slot 索引
    length_slot_index_ = index;
}

bool ArrayObject::ToArrayIndex(const Value& key, uint64_t* out_index) {
    if (key.IsInt64()) {
        int64_t i64_val = key.i64();
        if (i64_val >= 0) {
            *out_index = static_cast<uint64_t>(i64_val);
            return true;
        }
    }
    return false;
}

bool ArrayObject::GetProperty(Context* context, ConstIndex key, Value* value) {
    // 检查是否访问 length 属性
    if (key == ConstIndexEmbedded::kLength) {
        *value = Value(static_cast<int64_t>(length()));
        return true;
    }
    // 其他属性通过父类处理
    return Object::GetProperty(context, key, value);
}

void ArrayObject::SetProperty(Context* context, ConstIndex key, Value&& value) {
    // 特殊处理 length 属性的设置
    if (key == ConstIndexEmbedded::kLength) {
        // 设置 length 的特殊行为
        if (!value.IsInt64() || value.i64() < 0) {
            // 无效的 length 值，忽略或抛出错误
            return;
        }

        size_t new_length = static_cast<size_t>(value.i64());
        size_t current_length = length();

        if (new_length < current_length) {
            // 删除超出新 length 的元素
            for (size_t i = new_length; i < current_length; i++) {
                auto index_key = context->FindConstOrInsertToLocal(Value(static_cast<int64_t>(i)));
                DelProperty(context, index_key);
            }
        }

        // 更新 length 值
        SetLengthValue(context, new_length);
        return;
    }

    // 检查是否是数组索引属性
    // 如果键是 "0", "1", "2" 等数字字符串
    // 需要检查是否需要更新 length
    // 这里简化处理：假设数字字符串键会通过 SetComputedProperty 处理

    // 其他属性通过父类处理
    Object::SetProperty(context, key, std::move(value));
}

void ArrayObject::DelProperty(Context* context, ConstIndex key) {
    // 删除属性不会改变 length（JS 标准行为）
    Object::DelProperty(context, key);
}

bool ArrayObject::GetComputedProperty(Context* context, const Value& key, Value* value) {
    // 统一转换为 ConstIndex 并调用 GetProperty
    auto const_key = context->FindConstOrInsertToLocal(key);
    return GetProperty(context, const_key, value);
}

void ArrayObject::SetComputedProperty(Context* context, const Value& key, Value&& value) {
    // 尝试转换为数组索引
    uint64_t array_index;
    if (ToArrayIndex(key, &array_index) && IsValidArrayIndex(array_index)) {
        // 检查是否需要更新 length
        size_t current_length = length();
        if (array_index >= current_length) {
            // 设置元素时，如果索引 >= length，需要更新 length
            SetLengthValue(context, array_index + 1);
        }
    }

    // 统一转换为 ConstIndex 并调用 SetProperty
    auto const_key = context->FindConstOrInsertToLocal(key);
    SetProperty(context, const_key, std::move(value));
}

void ArrayObject::DelComputedProperty(Context* context, const Value& key) {
    // 统一转换为 ConstIndex 并调用 DelProperty
    auto const_key = context->FindConstOrInsertToLocal(key);
    DelProperty(context, const_key);
}

void ArrayObject::Push(Context* context, Value val) {
    size_t len = length();
    auto index_key = context->FindConstOrInsertToLocal(Value(static_cast<int64_t>(len)));
    SetProperty(context, index_key, std::move(val));
    SetLengthValue(context, len + 1);
}

Value ArrayObject::Pop(Context* context) {
    size_t len = length();
    if (len == 0) {
        return Value(); // 返回 undefined
    }

    // 获取最后一个元素
    size_t last_index = len - 1;
    auto index_key = context->FindConstOrInsertToLocal(Value(static_cast<int64_t>(last_index)));
    Value result;
    GetProperty(context, index_key, &result);

    // 删除最后一个元素并更新 length
    DelProperty(context, index_key);
    SetLengthValue(context, last_index);

    return result;
}

void ArrayObject::ForEach(Context* context, Value callback) {
    // TODO: 实现 forEach 逻辑
    // 需要遍历 0 到 length-1 的所有索引
}

size_t ArrayObject::length() const {
    // 使用缓存的 slot 索引快速访问 length 属性
    if (length_slot_index_ != kPropertySlotIndexInvalid &&
        length_slot_index_ < static_cast<PropertySlotIndex>(properties_.size())) {
        const auto& value = GetPropertyValue(length_slot_index_);
        if (value.IsInt64()) {
            return static_cast<size_t>(value.i64());
        }
    }

    // 如果 length 属性还没有缓存或不存在，返回 0
    return 0;
}

Value& ArrayObject::At(Context* context, size_t index) {
    auto index_key = context->FindConstOrInsertToLocal(Value(static_cast<int64_t>(index)));

    auto slot_index = shape_->Find(index_key);
    if (slot_index != kPropertySlotIndexInvalid) {
        return GetPropertyValue(slot_index);
    }

    // 如果属性不存在，创建它
    if (index >= length()) {
        SetLengthValue(context, index + 1);
    }

    SetProperty(context, index_key, Value());
    slot_index = shape_->Find(index_key);
    return GetPropertyValue(slot_index);
}

ArrayObject* ArrayObject::New(Context* context, std::initializer_list<Value> values) {
    auto arr_obj = new ArrayObject(context);
    arr_obj->SetLengthValue(context, values.size());

    size_t i = 0;
    for (auto& value : values) {
        auto index_key = context->FindConstOrInsertToLocal(Value(static_cast<int64_t>(i++)));
        arr_obj->SetProperty(context, index_key, Value(value));
    }
    return arr_obj;
}

ArrayObject* ArrayObject::New(Context* context, size_t count) {
    auto arr_obj = new ArrayObject(context);
    arr_obj->SetLengthValue(context, count);

    // 注意：稀疏数组，不创建中间元素，只设置 length
    // JS 标准：new Array(10) 创建一个长度为 10 但没有元素的数组
    return arr_obj;
}

} // namespace mjs
