#pragma once

#include <string>

namespace mjs {

// String������ѭ���������⣬��ʹ�����ü�������
class String : public std::string {
public:
	using Base = std::string;
	using Base::Base;

	String(std::string&& str) : Base(std::move(str)) {}  // ���һ������ std::string �Ĺ��캯��

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