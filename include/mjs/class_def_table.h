#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>

#include <mjs/noncopyable.h>
#include <mjs/value/value.h>
#include <mjs/class_def/class_def.h>
#include <mjs/segmented_array.h>

namespace mjs {

class Runtime;
class ClassDefTable : public noncopyable {
public:
	ClassDefTable(Runtime* runtime);

	void Initialize(Runtime* runtime);

	void Register(ClassDefUnique class_def);

	void Clear();

	ClassDef& At(ClassId class_id) {
		return *class_def_arr_.at(static_cast<uint32_t>(class_id));
	}

	ClassDef& operator[](ClassId class_id) {
		return *class_def_arr_[(static_cast<uint32_t>(class_id))];
	}

private:
	uint32_t Insert(ClassDefUnique class_def) {
		auto lock = std::lock_guard(mutex_);
		return class_def_arr_.insert(std::move(class_def));
	}

private:
	std::mutex mutex_;
	SegmentedArray<ClassDefUnique, uint32_t, 1024> class_def_arr_;
};

} // namespace mjs