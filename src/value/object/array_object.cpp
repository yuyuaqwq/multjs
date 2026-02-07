#include <mjs/value/object/array_object.h>

#include <array>
#include <algorithm>
#include <string>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/error.h>
#include <mjs/const_index_embedded.h>

namespace mjs {

// 设计：
// 混合模式结构：[哈希表元素0...N-1] [快速数组元素0...M-1]
// - slow_property_count_：哈希表元素数量（N）
// - length_：快速数组元素数量（M）
// 哈希表元素从4开始，需要时翻倍增长
// 快速数组元素直接通过索引访问

ArrayObject::ArrayObject(Context* context)
    : Object(context, ClassId::kArrayObject)
{
    // 预留初始哈希表空间
    properties_.reserve(kInitialHashTableCapacity);
    length_ = 0;
    slow_property_count_ = 0;
}

ArrayObject::ArrayObject(Context* context, size_t count)
    : Object(context, ClassId::kArrayObject)
{
    // 预留空间：哈希表 + 数组元素
    properties_.reserve(kInitialHashTableCapacity + count);

    length_ = count;
    slow_property_count_ = 0;

    // 初始化数组元素为 undefined
    for (size_t i = 0; i < count; ++i) {
        properties_.emplace_back(PropertySlot(Value()));
    }
}

ArrayObject::ArrayObject(Context* context, std::initializer_list<Value> values)
    : Object(context, ClassId::kArrayObject)
{
    // 预留空间：哈希表 + 数组元素
    properties_.reserve(kInitialHashTableCapacity + values.size());

    length_ = values.size();
    slow_property_count_ = 0;

    // 初始化数组元素
    for (auto& value : values) {
        properties_.emplace_back(PropertySlot(Value(value)));
    }
}

bool ArrayObject::GetProperty(Context* context, ConstIndex key, Value* value) {
    // 检查是否访问 length 属性
    if (key == ConstIndexEmbedded::kLength) {
        *value = Value(static_cast<int64_t>(length_));
        return true;
    }
    // 不存在 a.1 这种语法，所以通过.是访问不到数组元素的
    return Object::GetProperty(context, key, value);
}

bool ArrayObject::GetComputedProperty(Context* context, const Value& key, Value* value) {
    // 优化：直接处理数字索引
    if (key.IsInt64()) {
        int64_t i64_val = key.i64();
        if (i64_val >= 0) {
            size_t idx = static_cast<size_t>(i64_val);
            if (IsValidArrayIndex(static_cast<uint64_t>(idx))) {
                // 访问快速数组部分
                if (idx < length_) {
                    *value = properties_[slow_property_count_ + idx].value;
                    // 如果元素是 undefined，返回 false（表示"空洞"）
                    return !value->IsUndefined();
                }
                // 索引超出范围，返回 false（属性不存在）
                return false;
            }
        }
    }

    // 可能是"1"、"2"此类的字符串，需要转成数字索引
    if (key.IsString()) {
        uint64_t array_index = 0;
        if (TryStringToArrayIndex(key.string_view(), &array_index)) {
            size_t idx = static_cast<size_t>(array_index);
            // 访问快速数组部分
            if (idx < length_) {
                *value = properties_[slow_property_count_ + idx].value;
                return !value->IsUndefined();
            }
            return false;
        }
    }

    // 其他情况（如非数字字符串）从哈希表获取
    return Object::GetComputedProperty(context, key, value);
}

bool ArrayObject::HasProperty(Context* context, ConstIndex key) {
    // 检查是否访问 length 属性
    if (key == ConstIndexEmbedded::kLength) {
        return true;
    }

    // 检查是否是数组索引
    const Value& key_value = context->GetConstValue(key);
    if (key_value.IsString()) {
        uint64_t array_index = 0;
        if (TryStringToArrayIndex(key_value.string_view(), &array_index)) {
            // 是有效的数组索引
            size_t idx = static_cast<size_t>(array_index);
            return idx < length_;
        }
    }

    // 其他属性调用父类方法
    return Object::HasProperty(context, key);
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

        if (new_length < length_) {
            // 缩短快速数组
            properties_.resize(slow_property_count_ + new_length);
        }

        // 更新 length 值
        length_ = new_length;
        return;
    }

    // 确保哈希表有足够空间
    EnsureHashTableCapacity(context);

    // 其他属性通过父类处理（存储在哈希表中）
    Object::SetProperty(context, key, std::move(value));
}

void ArrayObject::SetComputedProperty(Context* context, const Value& key, Value&& value) {
    // 优化：直接处理数字索引
    if (key.IsInt64()) {
        int64_t i64_val = key.i64();
        if (i64_val >= 0) {
            uint64_t array_index = static_cast<uint64_t>(i64_val);
            if (IsValidArrayIndex(array_index)) {
                size_t idx = static_cast<size_t>(array_index);

                // 更新 length
                if (idx >= length_) {
                    length_ = idx + 1;
                }

                // 在快速数组中存储
                size_t required_size = slow_property_count_ + length_;
                if (properties_.size() < required_size) {
                    properties_.resize(required_size);
                }
                properties_[slow_property_count_ + idx].value = std::move(value);
                return;
            }
        }
    }

    // 可能是"1"、"2"此类的字符串，需要转成数字索引
    if (key.IsString()) {
        uint64_t array_index = 0;
        if (TryStringToArrayIndex(key.string_view(), &array_index)) {
            size_t idx = static_cast<size_t>(array_index);

            // 更新 length
            if (idx >= length_) {
                length_ = idx + 1;
            }

            // 在快速数组中存储
            size_t required_size = slow_property_count_ + length_;
            if (properties_.size() < required_size) {
                properties_.resize(required_size);
            }
            properties_[slow_property_count_ + idx].value = std::move(value);
            return;
        }
    }

    // 其他情况调用父类方法（哈希表存储）
    Object::SetComputedProperty(context, key, std::move(value));
}

bool ArrayObject::DelComputedProperty(Context* context, const Value& key, Value* value) {
    // 优化：直接处理数字索引
    if (key.IsInt64()) {
        int64_t i64_val = key.i64();
        if (i64_val >= 0) {
            size_t idx = static_cast<size_t>(i64_val);
            if (IsValidArrayIndex(static_cast<uint64_t>(idx))) {
                // 从快速数组中删除
                if (idx < length_) {
                    *value = std::move(properties_[slow_property_count_ + idx].value);
                    // 设置为 undefined
                    properties_[slow_property_count_ + idx].value = Value();
                    return true;
                }
                // 索引超出范围，返回 false
                *value = Value();
                return false;
            }
        }
    }

    // 可能是"1"、"2"此类的字符串，需要转成数字索引
    if (key.IsString()) {
        uint64_t array_index = 0;
        if (TryStringToArrayIndex(key.string_view(), &array_index)) {
            size_t idx = static_cast<size_t>(array_index);
            // 从快速数组中删除
            if (idx < length_) {
                *value = std::move(properties_[slow_property_count_ + idx].value);
                // 设置为 undefined
                properties_[slow_property_count_ + idx].value = Value();
                return true;
            }
            // 索引超出范围，返回 false
            *value = Value();
            return false;
        }
    }

    // 其他情况调用父类方法（从哈希表删除）
    return Object::DelComputedProperty(context, key, value);
}

void ArrayObject::Push(Context* context, Value val) {
    // 直接添加到快速数组末尾
    properties_.emplace_back(PropertySlot(std::move(val)));
    ++length_;
}

Value ArrayObject::Pop(Context* context) {
    size_t len = length_;
    if (len == 0) {
        return Value(); // 返回 undefined
    }

    size_t last_index = len - 1;
    Value result = std::move(properties_[slow_property_count_ + last_index].value);
    properties_.pop_back();
    --length_;
    return result;
}

void ArrayObject::ForEach(Context* context, Value callback) {
    // 遍历数组元素，调用回调函数
    for (size_t i = 0; i < length_; ++i) {
        Value element_value = properties_[slow_property_count_ + i].value;

        // 调用回调函数：callback(element, index, array)
        Value argv[] = { element_value, Value(static_cast<int64_t>(i)), Value(this) };
        context->CallFunction(&callback, Value(), std::begin(argv), std::end(argv));
    }
}

bool ArrayObject::TryStringToArrayIndex(std::string_view str, uint64_t* out_index) {
    if (str.empty()) {
        return false;
    }

    // 手动解析为无符号整数，避免异常开销
    uint64_t index = 0;

    for (char c : str) {
        if (c < '0' || c > '9') {
            return false; // 非数字字符
        }

        // 检查乘法溢出
        if (index > UINT64_MAX / 10) {
            return false; // 溢出
        }
        index *= 10;

        uint64_t digit = static_cast<uint64_t>(c - '0');
        // 检查加法溢出
        if (index > UINT64_MAX - digit) {
            return false; // 溢出
        }
        index += digit;
    }

    // 确保是有效的数组索引
    if (IsValidArrayIndex(index)) {
        *out_index = index;
        return true;
    }

    return false;
}

void ArrayObject::EnsureHashTableCapacity(Context* context) {
    size_t current_capacity = properties_.size() - length_;
    if (slow_property_count_ < current_capacity) {
        return; // 还有空间
    }

    // 计算新容量：从4开始翻倍增长
    size_t new_capacity = (current_capacity == 0) ? kInitialHashTableCapacity : current_capacity * 2;

    // 需要复制整个数组，因为快速数组元素在后面
    std::vector<PropertySlot> new_properties;
    new_properties.reserve(new_capacity + length_);

    // 复制哈希表元素
    for (size_t i = 0; i < slow_property_count_; ++i) {
        new_properties.push_back(std::move(properties_[i]));
    }

    // 填充剩余哈希表空间为空
    for (size_t i = slow_property_count_; i < new_capacity; ++i) {
        new_properties.emplace_back(PropertySlot(Value()));
    }

    // 复制快速数组元素
    for (size_t i = 0; i < length_; ++i) {
        new_properties.push_back(std::move(properties_[slow_property_count_ + i]));
    }

    properties_ = std::move(new_properties);
}

size_t ArrayObject::GetLength() const {
    return length_;
}

} // namespace mjs
