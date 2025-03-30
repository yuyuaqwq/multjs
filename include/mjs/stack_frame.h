#pragma once

#include <vector>
#include <string>
#include <memory>

#include <mjs/noncopyable.h>
#include <mjs/value.h>

namespace mjs {

// StackFrame：
// 使用tls，为每个线程指定一块内存(如1M)
// 当前StackFrame的增长都直接作用在这块内存中
// 调用时，因为旧的StackFrame不再增长
// 新的StackFrame的指针，直接指向该内存的未使用部分(底下是旧的StackFrame)，然后使用新的StackFrame即可
class Stack;
class StackFrame : public noncopyable {
public:
	StackFrame(Stack* stack)
		: stack_(stack){}

	void push(const Value& value);
	void push(Value&& value);
	Value pop();

	// 正数表示从栈帧底向上索引，0开始
	// 负数表示从栈帧顶向下索引，-1开始
	Value& get(ptrdiff_t index);
	void set(ptrdiff_t index, const Value& value);
	void set(ptrdiff_t index, Value&& value);

	size_t bottom() const { return bottom_; }
	void set_bottom(size_t bottom) { bottom_ = bottom; }

	const auto& this_val() const { return this_val_; }
	void set_this_val(Value&& this_val) { this_val_ = this_val; }

private:
	Stack* stack_;
	Value this_val_;
	size_t bottom_ = 0;	// 当前栈帧的栈底(在栈中的索引)
};

// 每个线程固定的栈
class Stack : public noncopyable {
public:
	Stack(size_t count) {
		vector_.reserve(count);
	}

	void push(const Value& value);
	void push(Value&& value);
	Value pop();

	Value& get(size_t index);
	void set(size_t index, const Value& value);
	void set(size_t index, Value&& value);

	void upgrade(size_t size);
	void reduce(size_t size);

	size_t size()  const noexcept;
	void resize(size_t size);

	auto& vector() { return vector_; }

private:
	std::vector<Value> vector_;
};


} // namespace mjs