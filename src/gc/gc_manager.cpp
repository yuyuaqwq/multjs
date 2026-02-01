#include <mjs/gc/gc_manager.h>

#include <iostream>
#include <format>

#include <mjs/context.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>

namespace mjs {

GCManager::GCManager(Context* context)
    : context_(context) {}

GCManager::~GCManager() = default;

bool GCManager::Initialize() {
    heap_ = std::make_unique<GCHeap>(context_);
    return heap_->Initialize();
}

GCObject* GCManager::Allocate(GCObjectType gc_type, size_t size) {
    if (!heap_) {
        return nullptr;
    }
    return heap_->Allocate(gc_type, size);
}

bool GCManager::CollectGarbage(bool full_gc) {
    if (!heap_) {
        return false;
    }
    return heap_->CollectGarbage(full_gc);
}

void GCManager::ForceFullGC() {
    if (heap_) {
        heap_->ForceFullGC();
    }
}

void GCManager::GetHeapStats(size_t& new_space_used, size_t& new_space_capacity,
                             size_t& old_space_used, size_t& old_space_capacity) const {
    if (!heap_) {
        new_space_used = old_space_used = 0;
        new_space_capacity = old_space_capacity = 0;
        return;
    }
    
    // 通过GCHeap获取统计信息
    // 这些需要通过新增API暴露
    new_space_capacity = kNewSpaceSize;
    old_space_capacity = kOldSpaceInitialSize;
    new_space_used = old_space_used = 0;  // 需要GCHeap暴露
}

void GCManager::GetGCStats(size_t& total_allocated, size_t& total_collected, uint32_t& gc_count) const {
    if (heap_) {
        heap_->GetStats(total_allocated, total_collected, gc_count);
    } else {
        total_allocated = total_collected = 0;
        gc_count = 0;
    }
}

void GCManager::PrintStats() const {
    size_t total_allocated, total_collected;
    uint32_t gc_count;
    GetGCStats(total_allocated, total_collected, gc_count);
    
    std::cout << std::format("GC Statistics:\n");
    std::cout << std::format("  Total allocated: {} bytes\n", total_allocated);
    std::cout << std::format("  Total collected: {} bytes\n", total_collected);
    std::cout << std::format("  GC count: {}\n", gc_count);
}

void GCManager::SetGCThreshold(uint8_t threshold) {
    if (heap_) {
        heap_->set_gc_threshold(std::max<uint8_t>(10, std::min<uint8_t>(90, threshold)));
    }
}

void GCManager::AddRoot(Value* value) {
    if (heap_) {
        heap_->AddRoot(value);
    }
}

void GCManager::RemoveRoot(Value* value) {
    if (heap_) {
        heap_->RemoveRoot(value);
    }
}

void GCManager::PrintObjectTree(Context* context) {
    std::cout << "GC Object Tree:\n";
    // 可以通过GCHeap遍历所有对象
    // 具体实现取决于需要展示的信息
}

} // namespace mjs
