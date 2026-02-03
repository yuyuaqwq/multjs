#pragma once

#include <mjs/value/object/object.h>

namespace mjs {

class ConstructorObject : public Object {
private:
	ConstructorObject(Context* context, ClassId target_class_id)
		: Object(context, ClassId::kFunctionObject)
		, target_class_id_(target_class_id) {}

public:
	ClassId target_class_id() const { return target_class_id_; }

protected:
	friend class GCManager;

	ClassId target_class_id_;
};

} // namespace mjs