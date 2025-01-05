#pragma once

#include <vector>
#include <string>
#include <memory>

#include "value.h"

#include <cstdlib>

namespace mjs {

template <class T>
class StackAllocator {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    template <typename U> struct rebind {
        using other = StackAllocator<U>;
    };

    StackAllocator() noexcept {}
    constexpr ~StackAllocator() {}
    template <class U> StackAllocator(const StackAllocator<U>& pool) noexcept :
        StackAllocator() {}

    [[nodiscard]] T* allocate(std::size_t n) {
        return static_cast<T*>(alloca(n * sizeof(T)));
    }

    void deallocate(T* free_ptr, std::size_t n) {
        
    }
};


template< class T1, class T2 >
constexpr bool operator==(const StackAllocator<T1>& lhs, const StackAllocator<T2>& rhs) noexcept {
    return true;
}

class StackFrame {
public:
	void Push(const Value& value);
	void Push(Value&& value);
	Value Pop();

	// 负数表示从尾部索引起
	Value& Get(int32_t index);
	void Set(int32_t index, const Value& value);
	void Set(int32_t index, Value&& value);

	size_t Size()  const noexcept;
	void ReSize(size_t s);

private:
	std::vector<Value> stack_; // , StackAllocator<Value>
};

} // namespace mjs