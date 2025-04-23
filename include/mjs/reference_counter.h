#pragma once

#include <cstdint>

#include <mjs/noncopyable.h>

namespace mjs {

class ReferenceCounter : public noncopyable {
public:
	ReferenceCounter() = default;
	virtual ~ReferenceCounter() = default;

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