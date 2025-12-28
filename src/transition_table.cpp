#include <mjs/transition_table.h>
#include <mjs/shape.h>

#include <cassert>

namespace mjs {

TransitionTable::~TransitionTable() {
    assert(!Has());
    if (type_ == Type::kMap) {
        delete map_;
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

} // namespace mjs