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
    private ::ankerl::unordered_dense::map<ConstIndex, Value, ConstIndexHasher, ConstIndexHashKeyEqual>,
    public noncopyable {
public:
    using Base = ankerl::unordered_dense::map<ConstIndex, Value, ConstIndexHasher, ConstIndexHashKeyEqual>;
    using iterator = Base::iterator;
    using const_iterator = Base::const_iterator;

    PropertyMap(Context* context);
    PropertyMap(Runtime* runtime);

	~PropertyMap();

    std::pair<iterator, bool> insert(Runtime* runtime, std::string_view name, Value&& value) {
        ConstIndex index = InsertConst(runtime, name);
        return Base::emplace(index, std::move(value));
    }

    std::pair<iterator, bool> insert(Context* context, std::string_view name, Value&& value) {
        ConstIndex index = InsertConst(context, name);
        ReferenceConst(context, index);
        return Base::emplace(index, std::move(value));
    }

    void set(Runtime* runtime, ConstIndex index, Value&& value) {
        assert(!index.is_invalid());
        assert(index.is_global_index());
        Base::operator[](index) = std::move(value);
    }

    iterator set(Context* context, ConstIndex index, Value&& value) {
        assert(!index.is_invalid());
        auto res = Base::emplace(index, value);
        if (!res.second) {
            res.first->second = std::move(value);
        }
        else if (index.is_local_index()) {
            ReferenceConst(context, index);
        }
        return res.first;
    }

    size_t erase(Runtime* runtime, ConstIndex index) {
        assert(!index.is_invalid());
        assert(index.is_global_index());
        return Base::erase(index);
    }

    size_t erase(Context* context, ConstIndex index);


    iterator find(ConstIndex index) {
        assert(!index.is_invalid());
        return Base::find(index);
    }

    const_iterator find(ConstIndex index) const {
        assert(!index.is_invalid());
        return Base::find(index);
    }

    // Iteration support
    iterator begin() noexcept { return Base::begin(); }
    const_iterator begin() const noexcept { return Base::begin(); }
    const_iterator cbegin() const noexcept { return Base::cbegin(); }

    iterator end() noexcept { return Base::end(); }
    const_iterator end() const noexcept { return Base::end(); }
    const_iterator cend() const noexcept { return Base::cend(); }


    Runtime& runtime() const { return *runtime_; }
    Context& context() const { return *context_; }

private:
    ConstIndex InsertConst(Runtime* runtime, std::string_view name);
    ConstIndex InsertConst(Context* context, std::string_view name);

    void ReferenceConst(Context* context, ConstIndex index);

private:
    friend class ConstIndexHasher;
    friend class ConstIndexHashKeyEqual;
    Runtime* runtime_;
    Context* context_;
};

} // namespace mjs