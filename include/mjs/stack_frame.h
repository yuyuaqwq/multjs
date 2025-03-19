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
class StackFrame {
public:
	StackFrame(Stack* stack)
		: stack_(stack){}

	void Push(const Value& value);
	void Push(Value&& value);
	Value Pop();

	// 正数表示从栈帧底向上索引，0开始
	// 负数表示从栈帧顶向下索引，-1开始
	Value& Get(ptrdiff_t index);
	void Set(ptrdiff_t index, const Value& value);
	void Set(ptrdiff_t index, Value&& value);

	size_t bottom() const { return bottom_; }
	void set_bottom(size_t bottom) { bottom_ = bottom; }

private:
	Stack* stack_;
	size_t bottom_ = 0;	// 当前栈帧的栈底
};

// 每个线程固定的栈
class Stack : noncopyable {
public:
	Stack(size_t count) {
		stack_.reserve(count);
	}

	void Push(const Value& value);
	void Push(Value&& value);
	Value Pop();

	Value& Get(size_t index);
	void Set(size_t index, const Value& value);
	void Set(size_t index, Value&& value);

	void Upgrade(size_t size);
	void Reduce(size_t size);

	size_t Size()  const noexcept;
	void Resize(size_t size);

private:
	std::vector<Value> stack_;
};


} // namespace mjs