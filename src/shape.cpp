#include <mjs/shape.h>
#include <mjs/context.h>

namespace mjs {

Shape::Shape(Context* context, uint32_t property_count)
    : ReferenceCounter()
    , hash_(0)
    , prop_size_(0)
    , prop_array_(new ShapeProperty[property_count]) {}

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
        auto iter = prop_hash_.find(const_index);
        if (iter == prop_hash_.end()) {
            return nullptr;
        }
        return &prop_array_[*iter];
    }
}

void Shape::add(ShapeProperty&& prop) {
    prop_array_[prop_size_] = std::move(prop);

    if (prop_size_ > kPropArrayMaxSize) {
        // 同时插入到哈希表
        prop_hash_[prop.const_index()] = prop_size_;
    }

    prop_size_++;
}


ShapeManager::ShapeManager(Context* context) : context_(context) {}
ShapeManager::~ShapeManager() {
    for (auto& shape : shape_cache_) {
        // shape.Dereference();
    }
}

Shape* ShapeManager::new_shape() {
    auto shape = new Shape(context_);
    shape->Reference();
    return shape;
}

Shape* ShapeManager::find_or_create_shape(Shape* base_shape, const ShapeProperty& property) {

    auto new_hash = Shape::calc_new_shape_hash(base_shape, property);

    // 通过新的哈希查找表中是否存在相同哈希的shape




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