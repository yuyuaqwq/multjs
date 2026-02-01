#include <mjs/gc/gc_object.h>

#include <functional>

#include <mjs/context.h>
#include <mjs/value/value.h>

namespace mjs {

GCObject::GCObject(GCObjectType type, size_t size) {
    header_.set_type(type);
    header_.size_ = static_cast<uint32_t>(size);
    header_.set_generation(GCGeneration::kNew);
    header_.SetMarked(false);
    header_.SetForwarded(false);
    header_.SetPinned(false);
    header_.ClearAge();
}

void GCObject::GCTraverse(Context* context, std::function<void(Context* ctx, Value& value)> callback) {
    // 基类默认不遍历任何内容
    // 子类需要重写此方法
}

} // namespace mjs
