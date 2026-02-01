#include <mjs/gc/gc_allocator.h>

#include <mjs/context.h>
#include <mjs/gc/gc_heap.h>

namespace mjs {

GCAllocator::GCAllocator(Context* context)
    : context_(context) {}

GCAllocator::~GCAllocator() = default;

bool GCAllocator::Initialize() {
    heap_ = std::make_unique<GCHeap>(context_);
    return heap_->Initialize();
}

void* GCAllocator::Allocate(GCAllocType type, size_t size) {
    if (!heap_) {
        return nullptr;
    }
    
    // 转换为GCObjectType
    GCObjectType gc_type;
    switch (type) {
        case GCAllocType::kObject: gc_type = GCObjectType::kObject; break;
        case GCAllocType::kArray: gc_type = GCObjectType::kArray; break;
        case GCAllocType::kFunction: gc_type = GCObjectType::kFunction; break;
        case GCAllocType::kString: gc_type = GCObjectType::kString; break;
        case GCAllocType::kShape: gc_type = GCObjectType::kShape; break;
        case GCAllocType::kModuleDef: gc_type = GCObjectType::kModuleDef; break;
        case GCAllocType::kFunctionDef: gc_type = GCObjectType::kFunctionDef; break;
        case GCAllocType::kClosureVar: gc_type = GCObjectType::kClosureVar; break;
        default: gc_type = GCObjectType::kOther; break;
    }
    
    GCObject* obj = heap_->AllocateObject(gc_type, size);
    if (obj) {
        return obj->data();
    }
    return nullptr;
}

void GCAllocator::Free(void* ptr) {
    // 新GC系统中，内存由GC自动管理
    // 此方法仅在特殊情况下使用
}

bool GCAllocator::Collect(bool full_gc) {
    if (heap_) {
        return heap_->CollectGarbage(full_gc);
    }
    return false;
}

void GCAllocator::ForceFullCollection() {
    if (heap_) {
        heap_->ForceFullGC();
    }
}

void GCAllocator::GetHeapStats(size_t& used, size_t& capacity) const {
    if (heap_) {
        // TODO: 通过GCHeap暴露这些信息
        used = 0;
        capacity = kNewSpaceSize + kOldSpaceInitialSize;
    } else {
        used = capacity = 0;
    }
}

void GCAllocator::GetStats(size_t& allocated, size_t& collected, uint32_t& gc_count) const {
    if (heap_) {
        heap_->GetStats(allocated, collected, gc_count);
    } else {
        allocated = collected = 0;
        gc_count = 0;
    }
}

void GCAllocator::SetThreshold(uint8_t threshold) {
    if (heap_) {
        heap_->set_gc_threshold(threshold);
    }
}

// ========== 全局函数 ==========

void* GCAllocate(Context* context, GCAllocType type, size_t size) {
    if (!context) {
        return nullptr;
    }
    // 使用GCManager的分配器
    return context->gc_manager().heap()->AllocateObject(
        static_cast<GCObjectType>(type), size)->data();
}

void GCFree(void* ptr) {
    std::free(ptr);
}

bool GCCollect(Context* context, bool full_gc) {
    if (context) {
        return context->gc_manager().CollectGarbage(full_gc);
    }
    return false;
}

} // namespace mjs
