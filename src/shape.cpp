#include <mjs/shape.h>
#include <mjs/context.h>

namespace mjs {

bool ShapeProperty::const_index_equal(Context* context, const ConstIndex& rhs) const {
    if (const_index_.is_same_pool(rhs)) {
        return const_index_ == rhs;
    }
    auto& lval = GetPoolValue(context, const_index_);
    auto& rval = GetPoolValue(context, rhs);
    return lval.Comparer(rval) == 0;
}

uint64_t ShapeProperty::const_index_hash(Context* context, const ConstIndex& const_index) {
    auto& val = GetPoolValue(context, const_index);
    return val.hash();
}

const Value& ShapeProperty::GetPoolValue(Context* context, const ConstIndex& key) {
    assert(!key.is_invalid());
    if (key.is_global_index()) {
        auto& val = context->runtime().const_pool().at(key);
        return val;
    }
    else {
        assert(key.is_local_index());
        auto key_ = key;
        key_.from_local_index();
        auto& val = context->const_pool().at(key);
        return val;
    }
}



Shape::Shape(ShapeManager* shape_manager, uint32_t property_count)
    : ReferenceCounter()
    , shape_manager_(shape_manager)
    , hash_(0)
    , property_size_(0)
    , hash_capacity_(0)
    , slot_indices_(nullptr)
    , properties_(nullptr) {}

Shape::~Shape() {
    if (properties_) {
        delete[] properties_;
    }
    if (slot_indices_) {
        delete[] slot_indices_;
    }
}


const ShapeProperty* Shape::find(ConstIndex const_index) const {
    if (property_size_ <= kPropertiesMaxSize) {
        // 当属性数量较少时，线性查找更优
        for (uint32_t i = 0; i < property_size_; i++) {
            if (properties_[i].const_index_equal(&shape_manager_->context(), const_index)) {
                return &properties_[i];
            }
        }
        return nullptr;
    }
    else {
        // 使用哈希表查找，带线性探测
        uint32_t index = ShapeProperty::const_index_hash(&shape_manager_->context(), const_index) & hash_mask_;
        uint32_t original_index = index;
        
        do {
            int32_t slot_index = slot_indices_[index];
            if (slot_index == -1) {
                // 找到空槽位，说明元素不存在
                return nullptr;
            }
            
            const auto& prop = properties_[slot_index];
            if (prop.const_index_equal(&shape_manager_->context(), const_index)) {
                return &prop;
            }
            
            // 线性探测下一个位置
            index = (index + 1) & hash_mask_;
        } while (index != original_index); // 避免无限循环
        
        return nullptr;
    }
}

// 如果Shape只被一个位置引用了，那么可以原地增加属性
void Shape::add( ShapeProperty&& prop) {
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
        std::memcpy(old_properties, properties_, sizeof(*properties_) * (property_size_ - 1));
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
        uint32_t hash_index = ShapeProperty::const_index_hash(&shape_manager_->context(), prop.const_index()) & hash_mask_;
        
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
        uint32_t hash_index = ShapeProperty::const_index_hash(&shape_manager_->context(), prop.const_index()) & hash_mask_;
        
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


ShapeManager::ShapeManager(Context* context) 
    : context_(context)
    , empty_shape_(this, 0) {}

ShapeManager::~ShapeManager() {
    //for (auto& shape : shape_cache_) {
    //    shape.Dereference();
    //}
}

int ShapeManager::add_property(Shape** base_shape_ptr, const ShapeProperty& property) {
    auto base_shape = *base_shape_ptr;
    base_shape->find(property.const_index());

    auto new_hash = Shape::calc_new_shape_hash(base_shape, property);


    // 通过新的哈希查找表中是否存在相同哈希的shape

    // 存在，返回

    // 不存在，如果Shape只被一个位置引用，则直接add，返回原base_shape
    if (base_shape == &empty_shape_) {
        // 如果是empty_shape_，也不能直接add
    }


    // 否则创建


    return 0;

    // 创建临时shape用于查找
    // auto temp_shape = new Shape(context_);
    // temp_shape->Reference();
        
    // 复制基础属性
    //if (base_shape->prop_size() > 0) {
    //    for (uint32_t i = 0; i < base_shape->prop_size(); ++i) {
    //        const auto* prop = base_shape->find(base_shape->prop_array()[i].const_index());
    //        if (prop) {
    //            temp_shape->add(ShapeProperty(prop->flags(), prop->const_index(), prop->slot_index()));
    //        }
    //    }
    //}
    //    
    //// 添加新属性
    //temp_shape->add(ShapeProperty(flags, const_index, temp_shape->prop_size()));
    //    
    //// 创建查找键
    //ShapeKey key{temp_shape->hash(), temp_shape};
    //    
    //// 查找缓存
    //auto it = shape_cache_.find(key);
    //if (it != shape_cache_.end()) {
    //    temp_shape->Dereference();
    //    return it->second;
    //}
    //    
    //// 缓存新shape
    //shape_cache_[key] = temp_shape;
    //return temp_shape;
}


} // namespace mjs 