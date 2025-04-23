#pragma once

#include <string>
#include <optional>

#include <mjs/reference_counter.h>

namespace mjs {

// ������ѭ���������⣬��ʹ�����ü�������
class String
	: public std::string
	, public ReferenceCounter
{
public:
	using Base = std::string;
	using Base::Base;

	String(std::string&& str) 
		: Base(std::move(str))
	{
		hash_ = std::hash<std::string>()(*this);
	}
	
	size_t hash() {
		return hash_;
	}

private:
	size_t hash_;
};

} // namespace mjs