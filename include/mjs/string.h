#pragma once

#include <string>
#include <optional>

namespace mjs {

// String不会有循环引用问题，仅使用引用计数管理
class String : public std::string {
public:
	using Base = std::string;
	using Base::Base;

	String(std::string&& str) 
		: Base(std::move(str))
	{
		hash_ = std::hash<std::string>()(*this);
	}

	void Reference() {
		++ref_count_;
	}

	void Dereference() {
		--ref_count_;
		if (ref_count_ == 0) {
			delete this;
		}
	}
	
	size_t hash() {
		return hash_;
	}

private:
	uint32_t ref_count_ = 0;
	size_t hash_;
};

} // namespace mjs