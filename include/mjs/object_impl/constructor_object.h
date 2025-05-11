#pragma once

#include <mjs/object.h>

namespace mjs {

class ConstructorObject : public Object {
public:
	ConstructorObject(Runtime* runtime, ClassId target_class_id) 
		: Object(runtime, ClassId::kConstructorObject)
		, target_class_id_(target_class_id) {}

	ClassId target_class_id() const { return target_class_id_; }

protected:
	ClassId target_class_id_;
};

} // namespace mjs