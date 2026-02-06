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
// 快速路径场景：
// 2 个额外属性 + count 个数组元素
// 访问数组元素，索引会加上4来访问properties_
// 前2个则是保存属性，比如__proto__
// 如果属性超过了2，则退化成基于哈希表的数组

ArrayObject::ArrayObject(Context* context)
    : Object(context, ClassId::kArrayObject)
{
    // 预留前4个位置给额外属性
    properties_.resize(kArrayElementOffset);
    length_ = 0;
    is_fast_array_ = true;
}

ArrayObject::ArrayObject(Context* context, size_t count)
    : Object(context, ClassId::kArrayObject)
{
    // 预留空间：前4个给额外属性，后面给数组元素
    properties_.reserve(count + kArrayElementOffset);
    properties_.resize(kArrayElementOffset);

    is_fast_array_ = true;

    length_ = count;
    if (count > 0) {
        properties_.resize(count + kArrayElementOffset);
    }
}

ArrayObject::ArrayObject(Context* context, std::initializer_list<Value> values)
    : Object(context, ClassId::kArrayObject)
{
    // 预留空间：前4个给额外属性，后面给数组元素
    properties_.reserve(values.size() + kArrayElementOffset);
    properties_.resize(kArrayElementOffset);

    is_fast_array_ = true;

    length_ = values.size();
    for (auto& value : values) {
        properties_.emplace_back(Value(value));
    }
}

bool ArrayObject::GetProperty(Context* context, ConstIndex key, Value* value) {
    // 检查是否访问 length 属性
    if (key == ConstIndexEmbedded::kLength) {
        *value = Value(static_cast<int64_t>(length_));
        return true;
    }

    // 检查是否是数组元素的字符串索引（如 "0", "1" 等）
    // 根据 ConstIndex 的正负判断使用哪个常量池
    const Value* key_value_ptr = nullptr;
    if (key >= 0) {
        // 全局常量池
        key_value_ptr = &context->runtime().global_const_pool()[key];
    } else {
        // 本地常量池
        key_value_ptr = &context->local_const_pool()[key];
    }

    if (key_value_ptr->IsString()) {
        uint64_t array_index = 0;
        if (TryStringToArrayIndex(key_value_ptr->string_view(), &array_index)) {
            size_t idx = static_cast<size_t>(array_index);
            if (idx < length_) {
                if (is_fast_array_) {
                    if (idx + kArrayElementOffset < properties_.size()) {
                        *value = properties_[idx + kArrayElementOffset].value;
                    } else {
                        *value = Value();
                    }
                    return true;
                } else {
                    return GetComputedProperty(context, Value(static_cast<int64_t>(idx)), value);
                }
            } else {
                // 索引超出 length，返回 undefined
                *value = Value();
                return true;
            }
        }
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
                if (is_fast_array_) {
                    // 快速路径：检查索引是否在有效范围内
                    if (idx < length_ && idx + kArrayElementOffset < properties_.size()) {
                        *value = properties_[idx + kArrayElementOffset].value;
                        // 如果元素是 undefined，返回 false（表示"空洞"）
                        return !value->IsUndefined();
                    }
                    // 索引超出范围，返回 false（属性不存在）
                    return false;
                } else {
                    // 慢速路径：从哈希表获取
                    return Object::GetComputedProperty(context, key, value);
                }
            }
        }
    }

    // 可能是"1"、"2"此类的字符串，需要转成数字索引
    if (key.IsString()) {
        uint64_t array_index = 0;
        if (TryStringToArrayIndex(key.string_view(), &array_index)) {
            size_t idx = static_cast<size_t>(array_index);
            if (is_fast_array_) {
                // 快速路径：检查索引是否在有效范围内
                if (idx < length_ && idx + kArrayElementOffset < properties_.size()) {
                    *value = properties_[idx + kArrayElementOffset].value;
                    // 如果元素是 undefined，返回 false（表示"空洞"）
                    return !value->IsUndefined();
                }
                // 索引超出范围，返回 false（属性不存在）
                return false;
            } else {
                // 慢速路径：从哈希表获取
                return Object::GetComputedProperty(context, key, value);
            }
        }
    }

    // 其他情况（如非数字字符串）调用父类方法
    return Object::GetComputedProperty(context, key, value);
}

bool ArrayObject::HasProperty(Context* context, ConstIndex key) {
    // 检查是否访问 length 属性
    if (key == ConstIndexEmbedded::kLength) {
        return true;
    }

    // 检查是否是数组元素的字符串索引（如 "0", "1" 等）
    // 根据 ConstIndex 的正负判断使用哪个常量池
    const Value* key_value_ptr = nullptr;
    if (key >= 0) {
        // 全局常量池
        key_value_ptr = &context->runtime().global_const_pool()[key];
    } else {
        // 本地常量池
        key_value_ptr = &context->local_const_pool()[key];
    }

    if (key_value_ptr->IsString()) {
        uint64_t array_index = 0;
        if (TryStringToArrayIndex(key_value_ptr->string_view(), &array_index)) {
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
        size_t current_length = GetLength();

        if (new_length < current_length) {
            // 如果是快速路径，直接缩短连续数组
            if (is_fast_array_ && new_length < length_) {
                properties_.resize(new_length + kArrayElementOffset);
            }
            // 稀疏场景则只能从哈希表删除
        }

        // 更新 length 值
        length_ = new_length;
        return;
    }

    // 检查是否需要退化到慢速模式
    if (is_fast_array_ && !EnsureExtraPropertySpace(context)) {
        TransitionToSlowMode(context);
    }

    // 其他属性通过父类处理
    return Object::SetProperty(context, key, std::move(value));
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

                // 检查是否应该在连续数组中存储
                if (is_fast_array_ && idx < length_ * 2 + 1024) {
                    // 在连续数组中存储
                    size_t required_size = idx + kArrayElementOffset + 1;
                    if (properties_.size() < required_size) {
                        properties_.resize(required_size);
                    }
                    properties_[idx + kArrayElementOffset].value = std::move(value);
                    return;
                }
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

            // 检查是否应该在连续数组中存储
            if (is_fast_array_ && idx < length_ * 2 + 1024) {
                // 在连续数组中存储
                size_t required_size = idx + kArrayElementOffset + 1;
                if (properties_.size() < required_size) {
                    properties_.resize(required_size);
                }
                properties_[idx + kArrayElementOffset].value = std::move(value);
                return;
            }
        }
    }

    // 其他情况调用父类方法
    Object::SetComputedProperty(context, key, std::move(value));
}

bool ArrayObject::DelComputedProperty(Context* context, const Value& key, Value* value) {
    // 优化：直接处理数字索引
    if (key.IsInt64()) {
        int64_t i64_val = key.i64();
        if (i64_val >= 0) {
            size_t idx = static_cast<size_t>(i64_val);
            if (IsValidArrayIndex(static_cast<uint64_t>(idx))) {
                if (is_fast_array_) {
                    // 快速路径：检查索引是否在有效范围内
                    if (idx < length_ && idx + kArrayElementOffset < properties_.size()) {
                        *value = std::move(properties_[idx + kArrayElementOffset].value);
                        // 设置为 undefined
                        properties_[idx + kArrayElementOffset].value = Value();
                        return true;
                    }
                    // 索引超出范围，返回 false
                    *value = Value();
                    return false;
                } else {
                    // 慢速路径：从哈希表删除
                    return Object::DelComputedProperty(context, key, value);
                }
            }
        }
    }

    // 可能是"1"、"2"此类的字符串，需要转成数字索引
    if (key.IsString()) {
        uint64_t array_index = 0;
        if (TryStringToArrayIndex(key.string_view(), &array_index)) {
            size_t idx = static_cast<size_t>(array_index);
            if (is_fast_array_) {
                // 快速路径：检查索引是否在有效范围内
                if (idx < length_ && idx + kArrayElementOffset < properties_.size()) {
                    *value = std::move(properties_[idx + kArrayElementOffset].value);
                    // 设置为 undefined
                    properties_[idx + kArrayElementOffset].value = Value();
                    return true;
                }
                // 索引超出范围，返回 false
                *value = Value();
                return false;
            } else {
                // 慢速路径：从哈希表删除
                return Object::DelComputedProperty(context, key, value);
            }
        }
    }

    // 其他情况调用父类方法
    return Object::DelComputedProperty(context, key, value);
}

void ArrayObject::Push(Context* context, Value val) {
    // 如果是快速路径
    if (is_fast_array_) {
        properties_.emplace_back(std::move(val));
        ++length_;
    } else {
        // 插入到哈希表
        SetComputedProperty(context, Value(static_cast<int64_t>(length_)), std::move(val));
        ++length_;
    }
}

Value ArrayObject::Pop(Context* context) {
    size_t len = length_;
    if (len == 0) {
        return Value(); // 返回 undefined
    }

    size_t last_index = len - 1;
    Value result;

    // 如果是快速路径
    if (is_fast_array_) {
        result = std::move(properties_[last_index + kArrayElementOffset].value);
        properties_.pop_back();
    } else {
        // 从哈希表中获取并删除
        GetComputedProperty(context, Value(static_cast<int64_t>(last_index)), &result);
        DelComputedProperty(context, Value(static_cast<int64_t>(last_index)), &result);
    }

    length_ = last_index;
    return result;
}

void ArrayObject::ForEach(Context* context, Value callback) {
    // 遍历数组元素，调用回调函数
    for (size_t i = 0; i < length_; ++i) {
        Value element_value;
        if (is_fast_array_) {
            if (i + kArrayElementOffset < properties_.size()) {
                element_value = properties_[i + kArrayElementOffset].value;
            } else {
                element_value = Value(); // undefined
            }
        } else {
            GetComputedProperty(context, Value(static_cast<int64_t>(i)), &element_value);
        }

        // 调用回调函数：callback(element, index, array)
        Value argv[] = { element_value, Value(static_cast<int64_t>(i)), Value(this) };
        context->CallFunction(&callback, Value(), std::begin(argv), std::end(argv));
    }
}

bool ArrayObject::TryStringToArrayIndex(std::string_view str, uint64_t* out_index) {
    if (str.empty()) {
        return false;
    }

    // 尝试解析为无符号整数
    try {
        size_t pos = 0;
        uint64_t index = std::stoull(std::string(str), &pos, 10);

        // 确保整个字符串都被解析，且是有效的数组索引
        if (pos == str.length() && IsValidArrayIndex(index)) {
            *out_index = index;
            return true;
        }
    } catch (...) {
        // 解析失败，不是有效的数字字符串
    }

    return false;
}

void ArrayObject::TransitionToSlowMode(Context* context) {
    if (!is_fast_array_) {
        return; // 已经是慢速模式
    }

    // 将所有数组元素迁移到哈希表
    std::vector<Value> array_elements;
    array_elements.reserve(length_);

    for (size_t i = 0; i < length_; ++i) {
        if (i + kArrayElementOffset < properties_.size()) {
            array_elements.push_back(properties_[i + kArrayElementOffset].value);
        } else {
            array_elements.push_back(Value());
        }
    }

    // 清空数组元素，保留前kArrayElementOffset个额外属性
    properties_.resize(kArrayElementOffset);

    // 将数组元素作为哈希表属性存储
    for (size_t i = 0; i < array_elements.size(); ++i) {
        if (!array_elements[i].IsUndefined()) {
            SetComputedProperty(context, Value(static_cast<int64_t>(i)), std::move(array_elements[i]));
        }
    }

    // 标记为慢速模式
    is_fast_array_ = false;
}

bool ArrayObject::EnsureExtraPropertySpace(Context* context) {
    // 检查额外属性数量是否超过限制
    size_t extra_property_count = 0;

    // 计算当前额外属性数量（除了数组元素之外的属性）
    // 简单的判断方式：检查是否数组元素起始位置前有过多属性
    for (size_t i = 0; i < kArrayElementOffset && i < properties_.size(); ++i) {
        // 检查这个位置是否有有效属性
        // 这里简化处理，实际需要通过shape来判断
        if (i < kArrayElementOffset) {
            extra_property_count++;
        }
    }

    // 如果额外属性超过限制，需要退化
    // 这里简化判断：如果有额外的非数组元素属性
    return extra_property_count <= kArrayElementOffset;
}

size_t ArrayObject::GetLength() const {
    return length_;
}

} // namespace mjs
