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
	Value& Get(int32_t index);
	void Set(int32_t index, const Value& value);
	void Set(int32_t index, Value&& value);

	uint32_t bottom() const { return bottom_; }
	void set_bottom(uint32_t bottom) { bottom_ = bottom; }

private:
	Stack* stack_;
	uint32_t bottom_ = 0;	// 当前栈帧的栈底
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

	Value& Get(int32_t index);
	void Set(int32_t index, const Value& value);
	void Set(int32_t index, Value&& value);

	void Upgrade(uint32_t size);
	void Reduce(uint32_t size);

	size_t size()  const noexcept;
	void resize(size_t size);

private:
	std::vector<Value> stack_;
};


} // namespace mjs