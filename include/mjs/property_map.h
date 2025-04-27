#pragma once

#include <unordered_map>

#include <mjs/value.h>
#include <mjs/unordered_dense.h>

namespace mjs {

class Runtime;
class Context;

class PropertyMap;

// Custom hasher that can access the PropertyMap's context
class ConstIndexHasher {
public:
    using is_avalanching = void; // Mark as high-quality hash if appropriate

    // explicit Hasher(PropertyMap* property_map) : property_map_(property_map) {}

    auto operator()(const ConstIndex& key) const noexcept -> uint64_t;

private:
    //PropertyMap* property_map_;
};

// Custom equality comparator
class ConstIndexHashKeyEqual {
public:
    // explicit KeyEqual(PropertyMap* property_map) : property_map_(property_map) {}

    bool operator()(const ConstIndex& lhs, const ConstIndex& rhs) const;

private:
    //PropertyMap* property_map_;
};



class PropertyMap : 
    public ::ankerl::unordered_dense::map<ConstIndex, Value, ConstIndexHasher, ConstIndexHashKeyEqual>,
    public noncopyable {
public:
    using Base = ankerl::unordered_dense::map<ConstIndex, Value, ConstIndexHasher, ConstIndexHashKeyEqual>;

    PropertyMap(Context* context);
    PropertyMap(Runtime* runtime);

	~PropertyMap();

    std::pair<iterator, bool> emplace(Runtime* runtime, std::string name, Value&& value) {
        ConstIndex key = InsertConst(runtime, std::move(name));
        return Base::emplace(key, std::move(value));
    }

    std::pair<iterator, bool> emplace(Context* context, std::string name, Value&& value) {
        ConstIndex key = InsertConst(context, std::move(name));
        ReferenceConst(context, key);
        return Base::emplace(key, std::move(value));
    }

    void set(Runtime* runtime, ConstIndex index, Value&& value) {
        assert(index.is_global_index());
        Base::operator[](index) = std::move(value);
    }

    void set(Context* context, ConstIndex index, Value&& value) {
        auto res = Base::emplace(index, value);
        if (res.second && index.is_local_index()) {
            ReferenceConst(context, index);
        }
        else {
            res.first->second = std::move(value);
        }
    }


    size_t erase(Runtime* runtime, ConstIndex index) {
        assert(index.is_global_index());
        return Base::erase(index);
    }

    size_t erase(Context* context, ConstIndex index);

    PropertyMap* copy(Context* context) {
        auto map = new PropertyMap(context);
        map->Base::operator=(*this);
        return map;
    }

    Runtime& runtime() const { return *runtime_; }
    Context& context() const { return *context_; }

private:
    ConstIndex InsertConst(Runtime* runtime, std::string&& name);
    ConstIndex InsertConst(Context* context, std::string&& name);

    void ReferenceConst(Context* context, ConstIndex index);

private:
    friend class ConstIndexHasher;
    friend class ConstIndexHashKeyEqual;
    Runtime* runtime_;
    Context* context_;
};

} // namespace mjs