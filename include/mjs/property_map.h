#pragma once

#include <unordered_map>

#include <mjs/value.h>
#include <mjs/unordered_dense.h>

namespace mjs {

class Runtime;
class Context;

class PropertyMap;
// Custom hasher that can access the PropertyMap's context
class Hasher {
public:
    using is_avalanching = void; // Mark as high-quality hash if appropriate

    explicit Hasher(PropertyMap* property_map) : property_map_(property_map) {}

    auto operator()(const ConstIndex& key) const noexcept -> uint64_t;

private:
    PropertyMap* property_map_;
};

// Custom equality comparator
class KeyEqual {
public:
    explicit KeyEqual(PropertyMap* property_map) : property_map_(property_map) {}

    bool operator()(const ConstIndex& lhs, const ConstIndex& rhs) const;

private:
    PropertyMap* property_map_;
};



class PropertyMap : 
    public ::ankerl::unordered_dense::map<ConstIndex, Value, Hasher, KeyEqual>,
    public noncopyable {
public:
    using Base = ankerl::unordered_dense::map<ConstIndex, Value, Hasher, KeyEqual>;

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

    iterator erase(const_iterator pos) {
        return Base::erase(pos);
    }

    size_t erase(Runtime* runtime, ConstIndex index) {
        return Base::erase(index);
    }

    size_t erase(Context* context, ConstIndex index) {
        return Base::erase(index);
    }

    Runtime& runtime() const { return *runtime_; }
    Context& context() const { return *context_; }

    PropertyMap* Copy(Context* context) {
        auto map = new PropertyMap(context);

        map->deallocate_buckets();
        map->m_values = m_values;
        map->m_max_load_factor = m_max_load_factor;
        map->copy_buckets(*this);

        return map;
    }

private:
    ConstIndex InsertConst(Runtime* runtime, std::string&& name);
    ConstIndex InsertConst(Context* context, std::string&& name);

    void ReferenceConst(Context* context, ConstIndex index);

private:
    Runtime* runtime_;
    Context* context_;
};

} // namespace mjs