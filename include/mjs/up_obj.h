#pragma once

#include <mjs/object.h>

namespace mjs {

// upvalue设计
// 指向一个Value，这个Value可能在栈上，也可能在堆上
// 被引用变量的函数未退出时，则直接引用栈的Value
// 被引用变量的函数已退出时，说明需要延长其生命周期(b函数在a函数内被定义，返回了b函数给全局，a已经返回了)
// 这个时候需要将其复制到b的func ref中
// 调用该函数时，函数访问闭包变量，就会访问func ref中存储的

// 总结：
// func ref如果位于父函数的作用域中，则引用的是栈value
// func ref如果不在父函数的作用域中(被返回出去延长了)，则引用的是堆value


// 延长生命周期：



//class FunctionBodyObject;
//class UpValueObject : public Object {
//public:
//	UpValueObject() noexcept;
//
//public:
//	
//};

} // namespace mjs