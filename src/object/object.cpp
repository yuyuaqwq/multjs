#include <mjs/object/object.h>

#include <mjs/context.h>
#include <mjs/runtime.h>

namespace mjs {

Object::Object(Context* context) {
	// ����context obj����
	if (context) {
		context->AddObject(this);
	}
	else {
		tag_.is_const_ = true;
	}
}

Object::~Object() {
	assert(gc_mark() || tag_.ref_count_ == 0);
	if (property_map_) {
		delete property_map_;
	}
	unlink();
}

void Object::SetProperty(Context* context, ConstIndex key, Value&& val) {
	assert(!context || !tag_.is_const_);
	if (!property_map_) property_map_ = new PropertyMap();
	(*property_map_)[key] = std::move(val);
}

Value* Object::GetProperty(Context* context, ConstIndex key) {
	// 1. ������������
	if (property_map_) {
		auto iter = property_map_->find(key);
		if (iter != property_map_->end()) {
			return &iter->second;
		}
	}
		
	// 2. class def����
	auto& class_def = context->runtime().class_def_table().get(class_id());
	auto val = class_def.GetProperty(&context->runtime(), key);
	if (val) return val;

	// 3. ԭ��������
	if (prototype_.IsObject()) {
		return prototype_.object().GetProperty(context, key);
	}
	return nullptr;
}

void Object::DelProperty(Context* context, ConstIndex key) {
	assert(!tag_.is_const_);
	if (!property_map_) return;
	property_map_->erase(key);
}

void Object::SetIndexed(Context* context, const Value& key, Value&& val) {
	if (!key.IsString() || key.const_index() == kConstInvaildIndex) {
		throw std::runtime_error("todo");
	}
	return SetProperty(context, key.const_index(), std::move(val));
}

Value* Object::GetIndexed(Context* context, const Value& key) {
	if (!key.IsString() || key.const_index() == kConstInvaildIndex) {
		throw std::runtime_error("todo");
	}
	return GetProperty(context, key.const_index());
}

void Object::DelIndexed(Context* context, const Value& key) {
	if (!key.IsString() || key.const_index() == kConstInvaildIndex) {
		throw std::runtime_error("todo");
	}
	return DelProperty(context, key.const_index());
}


void Object::Reference() {
	++tag_.ref_count_;
}

void Object::Dereference() {
	// ��������gc�����У��������п��ܰ�����������ͨ��gc_mark�����ظ�����
	if (!gc_mark()) {
		WeakDereference();
		if (tag_.ref_count_ == 0) {
			delete this;
		}
	}
}

void Object::WeakDereference() {
	assert(tag_.ref_count_ > 0);
	--tag_.ref_count_;
}

} // namespace mjs