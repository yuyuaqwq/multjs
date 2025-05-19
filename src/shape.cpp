#include <mjs/shape.h>
#include <mjs/context.h>

namespace mjs {

ShapeProperty::ShapeProperty(uint32_t flags, ConstIndex const_index)
    : flags_(flags)
    , const_index_(const_index) {}


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


bool TransitionTable::Has() const {
    if (type_ == Type::kNone) {
        return false;
    }
    else if (type_ == Type::kOne) {
        return true;
    }
    else {
        assert(type_ == Type::kMap);
        return !map_->empty();
    }
}

Shape* TransitionTable::Find(ConstIndex key) const {
    if (type_ == Type::kNone) {
        return nullptr;
    }
    else if (type_ == Type::kOne) {
        return key_ == key ? shape_ : nullptr;
    }
    else {
        assert(type_ == Type::kMap);
        auto iter = map_->find(key);
        if (iter == map_->end()) {
            return nullptr;
        }
        return iter->second;
    }
}

void TransitionTable::Add(ConstIndex key, Shape* shape) {
    if (type_ == Type::kNone) {
        key_ = key;
        shape_ = shape;
        type_ = Type::kOne;
    }
    else if (type_ == Type::kOne) {
        auto map = new ankerl::unordered_dense::map<ConstIndex, Shape*>;
        map->emplace(key_, shape_);
        map->emplace(key, shape);
        map_ = map;
        type_ = Type::kMap;
    }
    else {
        assert(type_ == Type::kMap);
        map_->emplace(key, shape);
    }
}

bool TransitionTable::Delete(ConstIndex key) {
    if (type_ == Type::kNone) {
        return false;
    }
    else if (type_ == Type::kOne) {
        if (key_ == key) {
            type_ = Type::kNone;
            return true;
        }
        return false;
    }
    else {
        assert(type_ == Type::kMap);
        auto del_count = map_->erase(key);
        return del_count > 0;
    }
}


Shape::Shape(ShapeManager* shape_manager)
    : ReferenceCounter()
    , shape_manager_(shape_manager)
    , property_size_(0)
    , property_map_(nullptr)
    , parent_shape_(nullptr) {}

Shape::Shape(Shape* parent_shape, uint32_t property_size)
    : ReferenceCounter()
    , parent_shape_(parent_shape)
    , shape_manager_(parent_shape->shape_manager_)
    , property_size_(property_size)
    , property_map_(parent_shape->property_map_)
{
    if (!property_map_) {
        property_map_ = new ShapePropertyHashTable;
    }
}

Shape::~Shape() {
    if (parent_shape_) {
        // 从父节点的过渡表中移除
        // base_shape->parent_shape()->transition_table().erase(base_shape->parent_transition_table_iter());
        // 因为只有add才会导致创建新的shape，上次add的一定在末尾
        auto success = parent_shape_->transtion_table_.Delete(GetProperty(property_size_ - 1).const_index());
        assert(success);
        
        // 释放property_map_
        if (property_map_ != parent_shape_->property_map_) {
            assert(property_map_);
            property_map_->DereferenceConstValue(&shape_manager_->context());
            delete property_map_;
        }

        parent_shape_->Dereference();
    }
    else {
        assert(!property_map_);
    }
}

const int Shape::Find(ConstIndex const_index) const {
    return property_map_->Find(const_index, property_size_);
}

void Shape::Add(ShapeProperty&& prop) {
    shape_manager_->context().ReferenceConstValue(prop.const_index());
    return property_map_->Add(std::move(prop));
}

const ShapeProperty& Shape::GetProperty(int32_t idx) const {
    return property_map_->GetProperty(idx);
}


ShapeManager::ShapeManager(Context* context) 
    : context_(context)
{
    empty_shape_ = new Shape(this);
    empty_shape_->Reference();
}

ShapeManager::~ShapeManager() {
    empty_shape_->Dereference();
}

int ShapeManager::AddProperty(Shape** base_shape_ptr, ShapeProperty&& property) {
    auto base_shape = *base_shape_ptr;
    if (base_shape != empty_shape_) {
        auto index = base_shape->Find(property.const_index());
        if (index != -1) {
            // 已经存在的属性，返回
            return index;
        }
    }

    // 查找过渡表
    auto transition_shape = base_shape->transtion_table().Find(property.const_index());
    if (transition_shape) {
        base_shape->Dereference();
        *base_shape_ptr = transition_shape;
        (*base_shape_ptr)->Reference();
        return AddProperty(base_shape_ptr, std::move(property));
    }

    // 创建新的shape
    Shape* new_shape = new Shape(base_shape, base_shape->property_size() + 1);

    if (base_shape->transtion_table().Has()) {
        // 如果过渡表不为空，需要创建一个分支
        if (new_shape->property_map() == base_shape->property_map()) {
            new_shape->set_property_map(new ShapePropertyHashTable);
        }
        for (int32_t i = 0; i < base_shape->property_size(); ++i) {
            ShapeProperty property = base_shape->GetProperty(i);
            new_shape->Add(std::move(property));
        }
    }

    // 放到过渡表
    base_shape->transtion_table().Add(property.const_index(), new_shape);

    // 子节点引用父节点
    // base_shape->Dereference();
    *base_shape_ptr = new_shape;
    (*base_shape_ptr)->Reference();

    new_shape->Add(std::move(property));
    return new_shape->property_size() - 1;
}

} // namespace mjs 