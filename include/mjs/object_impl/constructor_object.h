#pragma once

#include <mjs/object.h>

namespace mjs {

class ConstructorObject : public Object {
private:
	ConstructorObject(Runtime* runtime, ClassId target_class_id) 
		: Object(runtime, ClassId::kFunctionObject)
		, target_class_id_(target_class_id) {}

public:
	void Reference() {
		Object::Reference();
	}

	ClassId target_class_id() const { return target_class_id_; }

	static ConstructorObject* New(Runtime* runtime, ClassId target_class_id) {
		return new ConstructorObject(runtime, target_class_id);
	}

protected:
	ClassId target_class_id_;
};

} // namespace mjs