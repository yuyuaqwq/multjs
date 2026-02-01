#pragma once

#include <mjs/value/object/object.h>

namespace mjs {

class ConstructorObject : public Object {
private:
	ConstructorObject(Context* context, ClassId target_class_id, GCObjectType gc_type = GCObjectType::kFunction)
		: Object(context, ClassId::kFunctionObject, gc_type)
		, target_class_id_(target_class_id) {}

public:
	ClassId target_class_id() const { return target_class_id_; }

	static ConstructorObject* New(Context* context, ClassId target_class_id) {
		// 使用 GCHeap 分配内存
		GCHeap* heap = context->gc_manager().heap();

		// 计算需要分配的总大小
		size_t total_size = sizeof(ConstructorObject);

		// 分配原始内存，不构造 GCObject
		void* mem = heap->AllocateRaw(GCObjectType::kFunction, total_size);
		if (!mem) {
			return nullptr;
		}

		// 使用 placement new 在分配的内存中构造 ConstructorObject
		// 这会先构造 GCObject 基类，然后构造 ConstructorObject 派生类
		return new (mem) ConstructorObject(context, target_class_id);
	}

protected:
	ClassId target_class_id_;
};

} // namespace mjs