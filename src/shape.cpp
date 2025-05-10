#include <mjs/shape.h>
#include <mjs/context.h>

namespace mjs {

Shape::Shape(Context* context)
    : ReferenceCounter()
    , hash_(0)
    , prop_size_(0)
    , prop_capacity_(4)
    , prop_array_(new ShapeProperty[prop_capacity_]()) {}

Shape::~Shape() {
    if (prop_array_) {
        delete[] prop_array_;
    }
}


const ShapeProperty* Shape::find(ConstIndex const_index) const {
    if (prop_size_ <= kPropArrayMaxSize) {
        // 当属性数量较少时，线性查找更优
        for (uint32_t i = 0; i < prop_size_; i++) {
            // 后面需要改掉直接比较为ConstIndexHashKeyEqual
            if (prop_array_[i].const_index() == const_index) {
                return &prop_array_[i];
            }
        }
        return nullptr;
    }
    else {
        auto index = std::hash<uint32_t>()(const_index.value()) & prop_hash_mask_;

        while (prop_array_[index].const_index() != const_index) {
            if (prop_array_[index].const_index().is_invalid()) {
                return nullptr;
            }
            index = (++index) & prop_hash_mask_;
        }

        return &prop_array_[index];
    }
}

void Shape::add(ShapeProperty&& prop) {
    assert(prop_capacity_ >= kPropArrayMaxSize);

    if (prop_size_ < kPropArrayMaxSize && prop_hash_mask_ == 0) {
        prop_array_[prop_size_++] = std::move(prop);
        return;
    }

    if (prop_hash_mask_ == 0) {
        // 转换为哈希表
        rehash();
    }
    else if (prop_size_ == prop_capacity_) {
        rehash();
    }

    auto index = std::hash<uint32_t>()(prop.const_index().value()) & prop_hash_mask_;

    while (!prop_array_[index].const_index().is_invalid()) {
        index = (++index) & prop_hash_mask_;
    }

    prop_array_[index] = std::move(prop);

    ++prop_size_;
}

void Shape::rehash() {
    auto old_prop_capacity = prop_capacity_;
    auto* old_array = prop_array_;

    prop_capacity_ *= 2;
    prop_hash_mask_ = get_power2(prop_capacity_) - 1;
    prop_array_ = new ShapeProperty[prop_capacity_];
    prop_size_ = 0;

    for (uint32_t i = 0; i < old_prop_capacity; ++i) {
        if (old_array[i].const_index().is_invalid()) {
            continue;
        }
        add(std::move(old_array[i]));
    }

    delete[] old_array;
}

void Shape::remove_property(ConstIndex const_index) {
    
}

ShapeManager::ShapeManager(Context* context)
    : context_(context) {
}

ShapeManager::~ShapeManager() {
    for (auto& pair : shape_cache_) {
        pair.second->Dereference();
    }
}

Shape* ShapeManager::new_shape() {
    auto shape = new Shape(context_);
    shape->Reference();
    return shape;
}

Shape* ShapeManager::find_or_create_shape(Shape* base_shape, ConstIndex const_index, uint32_t flags) {
    // 计算新shape的哈希值
    size_t new_hash = base_shape->hash() * 31 + std::hash<int32_t>()(const_index.value());
    
    // 查找缓存
    auto it = shape_cache_.find(new_hash);
    if (it != shape_cache_.end()) {
        return it->second;
    }
    
    // 创建新的shape
    auto new_shape = new Shape(context_);
    new_shape->Reference();
    
    // 复制基础属性
    if (base_shape->prop_size() > 0) {
        ShapeProperty* new_array = new ShapeProperty[base_shape->prop_size()];
        std::memcpy(new_array, base_shape->prop_array(), 
            base_shape->prop_size() * sizeof(ShapeProperty));

    }
    
    // 添加新属性
    new_shape->add(ShapeProperty(flags, const_index, new_shape->prop_size()));
    
    // 缓存新shape
    shape_cache_[new_hash] = new_shape;
    
    return new_shape;
}

} // namespace mjs 