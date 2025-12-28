#include <mjs/shape_property_hash_table.h>
#include <mjs/context.h>

#include <cassert>
#include <cstring>

namespace mjs {

ShapePropertyHashTable::~ShapePropertyHashTable() {
    if (properties_) {
        delete[] properties_;
    }
    if (slot_indices_) {
        delete[] slot_indices_;
    }
}

const int ShapePropertyHashTable::Find(ConstIndex const_index, uint32_t property_size) const {
    if (property_size_ <= kPropertiesMaxSize) {
        // 当属性数量较少时，线性查找更优
        for (uint32_t i = 0; i < property_size; i++) {
            if (properties_[i].const_index() == const_index) {
                return i;
            }
        }
        return -1;
    }
    else {
        // 使用哈希表查找，带线性探测
        uint32_t index = std::hash<int32_t>()(const_index) & hash_mask_;
        uint32_t original_index = index;

        do {
            int32_t slot_index = slot_indices_[index];

            if (slot_index == -1) {
                // 找到空槽位，说明元素不存在
                return -1;
            }

            const auto& prop = properties_[slot_index];
            if (prop.const_index() == const_index) {
                if (slot_index >= property_size) {
                    return -1;
                }
                return slot_index;
            }

            // 线性探测下一个位置
            index = (index + 1) & hash_mask_;
        } while (index != original_index); // 避免无限循环

        return -1;
    }
}

void ShapePropertyHashTable::Add(ShapeProperty&& prop) {
    auto index = property_size_++;
    if (index >= property_capacity_) {
        if (property_capacity_ < 4) {
            property_capacity_ = 4;
        }
        else {
            property_capacity_ = property_size_ * 1.5;
        }
        assert(property_capacity_ > property_size_);
        auto* old_properties = properties_;
        properties_ = new ShapeProperty[property_capacity_];
        std::memcpy(properties_, old_properties, sizeof(*properties_) * (property_size_ - 1));
    }
    properties_[index] = std::move(prop);

    if (property_size_ > kPropertiesMaxSize) {
        if (hash_capacity_ == 0) {
            auto hash_capacity = GetPower2(property_size_);
            auto loading_factor = CalcLoadingFactor();
            if (loading_factor >= kLoadingFactor) {
                hash_capacity *= 2;
            }
            // 提升为哈希表
            Rehash(hash_capacity);
            return;
        }

        // 检查是否需要rehash
        auto loading_factor = CalcLoadingFactor();
        if (loading_factor >= kLoadingFactor) {
            Rehash(hash_capacity_ * 2);
            return;
        }

        // 使用线性探测法插入
        uint32_t hash_index = std::hash<int32_t>()(prop.const_index()) & hash_mask_;

        while (slot_indices_[hash_index] != -1) {
            // 如果已经存在相同的key，直接更新
            if (properties_[slot_indices_[hash_index]].const_index() == prop.const_index()) {
                slot_indices_[hash_index] = index;
                return;
            }
            // 否则继续探测下一个位置
            hash_index = (hash_index + 1) & hash_mask_;
        }

        // 找到空槽位，插入
        slot_indices_[hash_index] = index;
    }
}

const ShapeProperty& ShapePropertyHashTable::GetProperty(int32_t idx) {
    return properties_[idx];
}

void ShapePropertyHashTable::DereferenceConstValue(Context* context) {
    for (uint32_t i = 0; i < property_size_; i++) {
        context->DereferenceConstValue(properties_[i].const_index());
    }
}

uint32_t ShapePropertyHashTable::GetPower2(uint32_t n) {
    if (n <= 1) {
        return 1;
    }
    uint32_t p = 1;
    while (p < n) {
        p <<= 1;
    }
    return p;
}

double ShapePropertyHashTable::CalcLoadingFactor() const {
    return static_cast<double>(property_size_) / hash_capacity_;
}

void ShapePropertyHashTable::Rehash(uint32_t new_capacity) {
    // 保存旧的数据
    int32_t* old_indices = slot_indices_;

    // 分配新的空间
    hash_capacity_ = new_capacity;
    hash_mask_ = hash_capacity_ - 1;
    slot_indices_ = new int32_t[hash_capacity_];

    // 初始化新的slot_indices_数组
    for (uint32_t i = 0; i < hash_capacity_; i++) {
        slot_indices_[i] = -1;
    }

    // 重新插入所有元素
    for (uint32_t i = 0; i < property_size_; i++) {
        const auto& prop = properties_[i];
        uint32_t hash_index = std::hash<int32_t>()(prop.const_index()) & hash_mask_;

        // 使用线性探测找到新的位置
        while (slot_indices_[hash_index] != -1) {
            hash_index = (hash_index + 1) & hash_mask_;
        }

        slot_indices_[hash_index] = i;
    }

    // 删除旧的数据
    if (old_indices) {
        delete[] old_indices;
    }
}

} // namespace mjs