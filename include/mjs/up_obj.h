#pragma once

#include <mjs/object.h>
#include <mjs/func_obj.h>

namespace mjs {

class UpValueObject : public Object {
public:
	UpValueObject(FunctionBodyObject* func_body, uint32_t var_idx) noexcept
		: func_body(func_body)
		, var_idx(var_idx) {}

public:
	FunctionBodyObject* func_body;
	uint32_t var_idx;
};

// 闭包值引用
// 加载UpValue时，实际生成ClosureValueRef入栈
// 查找路径：
// func ref -> closure_values_[closure_value_idxs_[closure_value_idx]]

// 怎么拿到这个func ref？
// func ref也有一个parent链表
// 通过当前func ref，循环向上找func ref，每找一个就比较一下func body，匹配则表示就是这个func ref

// func ref怎么维护？
// 每次函数调用就把当前func ref赋值为parent吗？

class ClosureValueRefObject : public Object {
private:

public:
	Value* value_ref;
};


} // namespace mjs