#pragma once

#include <string>

namespace mjs {

// String不会有循环引用问题，仅使用引用计数管理
class String : public std::string {
public:
	using Base = std::string;
	using Base::Base;

	String(std::string&& str) : Base(std::move(str)) {}  // 添加一个接受 std::string 的构造函数

	void Reference() {
		++ref_count_;
	}

	void Dereference() {
		--ref_count_;
		if (ref_count_ == 0) {
			delete this;
		}
	}

private:
	uint32_t ref_count_ = 0;
};

} // namespace mjs