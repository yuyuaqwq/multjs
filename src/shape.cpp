#include <mjs/shape.h>
#include <mjs/context.h>

namespace mjs {


Shape::Shape(ShapeManager* shape_manager, uint32_t property_count)
    : ReferenceCounter()
    , shape_manager_(shape_manager)
    , hash_(0)
    , property_size_(0)
    , property_capacity_(property_count)
    , hash_capacity_(0)
    , slot_indices_(nullptr)
    , properties_(nullptr)
{
    if (property_capacity_ > 0) {
        properties_ = new ShapeProperty[property_capacity_];
    }
}

Shape::~Shape() {
    if (properties_) {
        delete[] properties_;
    }
    if (slot_indices_) {
        delete[] slot_indices_;
    }
}


const int Shape::find(ConstIndex const_index) const {
    if (property_size_ <= kPropertiesMaxSize) {
        // 当属性数量较少时，线性查找更优
        for (uint32_t i = 0; i < property_size_; i++) {
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
            if (prop.const_index() ==  const_index) {
                return slot_index;
            }
            
            // 线性探测下一个位置
            index = (index + 1) & hash_mask_;
        } while (index != original_index); // 避免无限循环
        
        return -1;
    }
}

// 如果Shape只被一个位置引用了，那么可以原地增加属性
void Shape::add(ShapeProperty&& prop) {
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
            auto hash_capacity = get_power2(property_size_);
            auto loading_factor = calc_loading_factor();
            if (loading_factor >= kLoadingFactor) {
                hash_capacity *= 2;
            }
            // 提升为哈希表
            rehash(hash_capacity);
            return;
        }

        // 检查是否需要rehash
        auto loading_factor = calc_loading_factor();
        if (loading_factor >= kLoadingFactor) {
            rehash(hash_capacity_ * 2);
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

void Shape::rehash(uint32_t new_capacity) {
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


ShapeManager::ShapeManager()  {
    empty_shape_ = new Shape(this, 0);
    empty_shape_->Reference();
}

ShapeManager::~ShapeManager() {
    //for (auto& shape : shape_cache_) {
    //    shape.Dereference();
    //}
}

int ShapeManager::add_property(Shape** base_shape_ptr, const ShapeProperty& property) {
    auto base_shape = *base_shape_ptr;
    auto index = base_shape->find(property.const_index());
    if (index != -1) {
        // 已经存在的属性，返回
        return index;
    }

    // 查找过渡表
    auto table = base_shape->transition_table();
    auto iter = table.find(property.const_index());
    if (iter != table.end()) {
        base_shape->Dereference();
        *base_shape_ptr = iter->second;
        (*base_shape_ptr)->Reference();
        return add_property(base_shape_ptr, property);
    }

    // 不存在，如果Shape只被一个位置引用，则直接add，返回原base_shape
    // 如果是empty_shape_，也不能直接add
    if (base_shape != empty_shape_
        && base_shape->ref_count() == 1)
    {
        auto tmp = property;
        base_shape->add(std::move(tmp));
        return base_shape->property_size() - 1;
    }

    // 创建新的shape
    Shape* new_shape = new Shape(this, base_shape->property_size() + 1);

    for (int32_t i = 0; i < base_shape->property_size(); ++i) {
        ShapeProperty property = base_shape->properties()[i];
        new_shape->add(std::move(property));
    }

    // 放到过渡表
    table.emplace(property.const_index(), new_shape);

    base_shape->Dereference();
    *base_shape_ptr = new_shape;
    (*base_shape_ptr)->Reference();

    ShapeProperty tmp = property;
    new_shape->add(std::move(tmp));
    return new_shape->property_size() - 1;
}


} // namespace mjs 