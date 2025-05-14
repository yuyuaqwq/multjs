#pragma once

#include <cstdint>

#include <mjs/noncopyable.h>

namespace mjs {

template <typename T>
class ReferenceCounter : public noncopyable {
public:
	ReferenceCounter() = default;
	~ReferenceCounter() = default;

	void Reference() {
		++ref_count_;
	}

	void Dereference() {
		--ref_count_;
		if (ref_count_ == 0) {
			delete static_cast<T*>(this);
		}
	}
	
	uint32_t ref_count() { return ref_count_; }

private:
	uint32_t ref_count_ = 0;
};

} // namespace mjs