#include <mjs/gc/gc_object.h>

#include <functional>

#include <mjs/context.h>
#include <mjs/value/value.h>

namespace mjs {

GCObject::GCObject() {
    header_.set_generation(GCGeneration::kNew);
    header_.SetMarked(false);
    header_.SetPinned(false);
    header_.SetDestructed(false);
    header_.ClearAge();
}

void GCObject::GCTraverse(Context* context, GCTraverseCallback callback) {
    // 基类默认不遍历任何内容
    // 子类需要重写此方法
}

} // namespace mjs
