#pragma once

#include <vector>
#include <string>
#include <memory>

#include <mjs/noncopyable.h>
#include <mjs/value.h>
#include <mjs/opcode.h>

namespace mjs {

// StackFrame：
// 使用tls，为每个线程指定一块内存(如1M)
// 当前StackFrame的增长都直接作用在这块内存中
// 调用时，因为旧的StackFrame不再增长
// 新的StackFrame的指针，直接指向该内存的未使用部分(底下是旧的StackFrame)，然后使用新的StackFrame即可
class Stack;
class StackFrame : public noncopyable {
public:
	StackFrame(Stack* stack);

	StackFrame(const StackFrame* upper_stack_frame);

	void push(const Value& value);
	void push(Value&& value);
	Value pop();

	void reduce(size_t count);
	void upgrade(size_t count);

	// 正数表示从栈帧底向上索引，0开始
	// 负数表示从栈帧顶向下索引，-1开始
	Value& get(ptrdiff_t index) const;
	void set(ptrdiff_t index, const Value& value);
	void set(ptrdiff_t index, Value&& value);

	const auto* upper_stack_frame() const { return upper_stack_frame_; }

	size_t bottom() const { return bottom_; }
	void set_bottom(size_t bottom) { bottom_ = bottom; }

	const auto& function_val() const { return function_val_; }
	void set_function_val(Value&& function_val) { function_val_ = std::move(function_val); }

	const auto* function_def() const { return function_def_; }
	void set_function_def(FunctionDefBase* function_def) { function_def_ = function_def; }

	const auto& this_val() const { return this_val_; }
	void set_this_val(Value&& this_val) { this_val_ = std::move(this_val); }

	auto pc() const { return pc_; }
	void set_pc(Pc pc) { pc_ = pc; }

private:
	Stack* stack_;
	const StackFrame* upper_stack_frame_ = nullptr;
	size_t bottom_;	// 当前栈帧的栈底(在栈中的索引)

	Value function_val_;
	FunctionDefBase* function_def_ = nullptr;
	Value this_val_;
	Pc pc_ = 0;
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

	void clear();

	auto& vector() { return vector_; }

private:
	std::vector<Value> vector_;
};

} // namespace mjs