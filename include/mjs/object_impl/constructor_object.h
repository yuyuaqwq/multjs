#pragma once

#include <mjs/object.h>

namespace mjs {

class ConstructorObject : public Object {
private:
	ConstructorObject(Context* context, ClassId target_class_id) 
		: Object(context, ClassId::kFunctionObject)
		, target_class_id_(target_class_id) {}

public:
	void Reference() {
		Object::Reference();
	}

	ClassId target_class_id() const { return target_class_id_; }

	static ConstructorObject* New(Context* context, ClassId target_class_id) {
		return new ConstructorObject(context, target_class_id);
	}

protected:
	ClassId target_class_id_;
};

} // namespace mjs