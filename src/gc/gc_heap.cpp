#include <mjs/gc/gc_heap.h>

#include <cstring>
#include <algorithm>
#include <unordered_map>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include <mjs/shape/shape.h>
#include <mjs/stack_frame.h>
#include <mjs/job_queue.h>
#include <mjs/local_const_pool.h>

namespace mjs {

// ==================== NewSpace Implementation ====================

NewSpace::NewSpace() = default;

NewSpace::~NewSpace() {
    if (from_space_) {
        delete[] from_space_;
    }
    if (to_space_) {
        delete[] to_space_;
    }
}

bool NewSpace::Initialize() {
    // 分配两个半区
    from_space_ = new uint8_t[kNewSpaceSemiSize];
    to_space_ = new uint8_t[kNewSpaceSemiSize];

    if (!from_space_ || !to_space_) {
        return false;
    }

    top_ = from_space_;
    to_space_top_ = to_space_;  // 初始化To空间分配指针
    return true;
}

void* NewSpace::Allocate(size_t size) {
    // 对齐大小
    size = AlignGCObjectSize(size);

    if (!HasSpace(size)) {
        return nullptr;
    }

    void* result = top_;
    top_ += size;

    // 清零内存
    std::memset(result, 0, size);
    return result;
}

void* NewSpace::AllocateInToSpace(size_t size) {
    // 对齐大小
    size = AlignGCObjectSize(size);

    // 检查To空间是否有足够空间
    if (to_space_top_ + size > to_space_ + kNewSpaceSemiSize) {
        return nullptr;
    }

    void* result = to_space_top_;
    to_space_top_ += size;

    // 清零内存
    std::memset(result, 0, size);
    return result;
}

bool NewSpace::HasSpace(size_t size) const {
    return top_ + AlignGCObjectSize(size) <= from_space_ + kNewSpaceSemiSize;
}

void NewSpace::SwapSpaces() {
    std::swap(from_space_, to_space_);
    std::swap(top_, to_space_top_);
}

void NewSpace::Reset(uint8_t* new_top) {
    top_ = new_top;
}

void NewSpace::IterateObjects(std::function<void(GCObject*)> callback) {
    uint8_t* current = from_space_;
    while (current < top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        callback(obj);
        current += obj->gc_size();
    }
}

// ==================== OldSpace Implementation ====================

OldSpace::OldSpace() = default;

OldSpace::~OldSpace() {
    if (space_start_) {
        delete[] space_start_;
    }
}

bool OldSpace::Initialize(size_t initial_size) {
    space_start_ = new uint8_t[initial_size];
    if (!space_start_) {
        return false;
    }
    
    capacity_ = initial_size;
    top_ = space_start_;
    used_size_ = 0;
    return true;
}

void* OldSpace::Allocate(size_t size) {
    size = AlignGCObjectSize(size);
    
    if (top_ + size > space_start_ + capacity_) {
        // 尝试扩展
        if (!Expand(size)) {
            return nullptr;
        }
    }
    
    void* result = top_;
    top_ += size;
    used_size_ += size;
    
    // 清零内存
    std::memset(result, 0, size);
    return result;
}

bool OldSpace::Expand(size_t min_size) {
    size_t new_capacity = capacity_ * 2;
    if (new_capacity < capacity_ + min_size) {
        new_capacity = capacity_ + min_size + kOldSpaceInitialSize;
    }
    
    uint8_t* new_space = new uint8_t[new_capacity];
    if (!new_space) {
        return false;
    }
    
    // 复制现有数据
    if (used_size_ > 0) {
        std::memcpy(new_space, space_start_, used_size_);
    }
    
    delete[] space_start_;
    space_start_ = new_space;
    top_ = space_start_ + used_size_;
    capacity_ = new_capacity;
    
    return true;
}

void OldSpace::IterateObjects(std::function<void(GCObject*)> callback) {
    uint8_t* current = space_start_;
    while (current < top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        callback(obj);
        current += obj->gc_size();
    }
}

void OldSpace::IterateLiveObjects(std::function<void(GCObject*)> callback) {
    uint8_t* current = space_start_;
    while (current < top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        if (obj->header()->IsMarked()) {
            callback(obj);
        }
        current += obj->gc_size();
    }
}

uint8_t* OldSpace::ComputeCompactTop() {
    uint8_t* new_top = space_start_;
    uint8_t* current = space_start_;
    
    while (current < top_) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        if (obj->header()->IsMarked()) {
            new_top += obj->gc_size();
        }
        current += obj->gc_size();
    }
    
    return new_top;
}

// ==================== GCHeap Implementation ====================

GCHeap::GCHeap(Context* context) 
    : context_(context) {}

GCHeap::~GCHeap() = default;

bool GCHeap::Initialize() {
    new_space_ = std::make_unique<NewSpace>();
    old_space_ = std::make_unique<OldSpace>();
    
    if (!new_space_->Initialize()) {
        return false;
    }
    
    if (!old_space_->Initialize()) {
        return false;
    }
    
    return true;
}

GCObject* GCHeap::AllocateObject(GCObjectType type, size_t data_size) {
    size_t total_size = GCObjectTotalSize(data_size);
    return Allocate(type, total_size);
}

GCObject* GCHeap::Allocate(GCObjectType type, size_t total_size) {
    // 检查是否需要GC
    if (!in_gc_ && new_space_->used_size() > kNewSpaceSemiSize * gc_threshold_ / 100) {
        CollectGarbage(false);
    }

    GCObject* obj = nullptr;

    // 大对象直接在老年代分配
    if (total_size >= kLargeObjectThreshold) {
        void* mem = old_space_->Allocate(total_size);
        if (!mem) {
            // 尝试完整GC
            if (!in_gc_ && CollectGarbage(true)) {
                mem = old_space_->Allocate(total_size);
            }
        }
        if (mem) {
            obj = new (mem) GCObject(type, total_size);
            obj->header()->set_generation(GCGeneration::kOld);
        }
    } else {
        // 在新生代分配
        void* mem = new_space_->Allocate(total_size);
        if (!mem) {
            // 新生代满了，触发Scavenge
            if (!in_gc_ && Scavenge()) {
                mem = new_space_->Allocate(total_size);
            }
        }
        if (mem) {
            obj = new (mem) GCObject(type, total_size);
            obj->header()->set_generation(GCGeneration::kNew);
        }
    }

    if (obj) {
        total_allocated_ += total_size;
    }

    return obj;
}

void* GCHeap::AllocateRaw(GCObjectType type, size_t total_size) {
    // 检查是否需要GC
    if (!in_gc_ && new_space_->used_size() > kNewSpaceSemiSize * gc_threshold_ / 100) {
        CollectGarbage(false);
    }

    void* mem = nullptr;

    // 大对象直接在老年代分配
    if (total_size >= kLargeObjectThreshold) {
        mem = old_space_->Allocate(total_size);
        if (!mem) {
            // 尝试完整GC
            if (!in_gc_ && CollectGarbage(true)) {
                mem = old_space_->Allocate(total_size);
            }
        }
    } else {
        // 在新生代分配
        mem = new_space_->Allocate(total_size);
        if (!mem) {
            // 新生代满了，触发Scavenge
            if (!in_gc_ && Scavenge()) {
                mem = new_space_->Allocate(total_size);
            }
        }
    }

    if (mem) {
        total_allocated_ += total_size;
    }

    return mem;
}

bool GCHeap::CollectGarbage(bool full_gc) {
    if (in_gc_) {
        return false;
    }
    
    in_gc_ = true;
    bool result = true;
    
    // 首先执行新生代GC
    result = Scavenge();
    
    // 如果需要完整GC，执行老年代GC
    if (full_gc && result) {
        result = MarkCompact();
    }
    
    in_gc_ = false;
    return result;
}

void GCHeap::ForceFullGC() {
    CollectGarbage(true);
}

void GCHeap::GetStats(size_t& total_allocated, size_t& total_collected, uint32_t& gc_count) const {
    total_allocated = total_allocated_;
    total_collected = total_collected_;
    gc_count = gc_count_;
}

// ==================== Scavenge (Copying GC) ====================

bool GCHeap::Scavenge() {
    ++gc_count_;

    // 重置To空间分配指针
    new_space_->ResetToSpace();

    // 收集根
    CollectRoots();
    
    // 转发指针（用于处理对象移动）
    std::unordered_map<GCObject*, GCObject*> forward_map;
    
    // 处理根中的新生代对象
    auto process_root = [&](Value* value) {
        if (!value || !value->IsObject()) return;

        Object* obj = &value->object();
        GCObject* gc_obj = static_cast<GCObject*>(obj);

        if (gc_obj->header()->generation() == GCGeneration::kNew) {
            GCObject* new_obj = nullptr;

            if (gc_obj->header()->IsForwarded()) {
                // 已经复制过，更新引用
                new_obj = forward_map[gc_obj];
            } else if (gc_obj->header()->age() >= kTenureAgeThreshold) {
                // 晋升到老年代
                new_obj = PromoteObject(gc_obj);
            } else {
                // 复制到To空间
                new_obj = CopyObject(gc_obj);
            }

            if (new_obj) {
                forward_map[gc_obj] = new_obj;
                // 更新Value中的引用
                *value = Value(reinterpret_cast<Object*>(new_obj->data()));
            }
        }
    };
    
    // 处理所有根
    for (auto* root : root_set_.stack_roots) {
        process_root(root);
    }
    for (auto* root : root_set_.global_roots) {
        process_root(root);
    }
    for (auto* root : root_set_.const_pool_roots) {
        process_root(root);
    }
    
    // 遍历To空间，复制其引用的对象
    // 注意：此时存活对象已经被复制到To空间，所以需要扫描To空间
    uint8_t* scan = new_space_->to_space();
    uint8_t* scan_end = new_space_->to_space_top();  // to_space_top指向To空间中下一个可用位置
    while (scan < scan_end) {
        GCObject* obj = reinterpret_cast<GCObject*>(scan);
        
        // 遍历对象的子引用
        obj->GCTraverse(context_, [&](Context* ctx, Value& value) {
            if (!value.IsObject()) return;

            Object* child_obj = &value.object();
            GCObject* child_gc_obj = static_cast<GCObject*>(child_obj);

            if (child_gc_obj->header()->generation() == GCGeneration::kNew) {
                GCObject* new_child = nullptr;

                if (child_gc_obj->header()->IsForwarded()) {
                    new_child = forward_map[child_gc_obj];
                } else if (child_gc_obj->header()->age() >= kTenureAgeThreshold) {
                    new_child = PromoteObject(child_gc_obj);
                } else {
                    new_child = CopyObject(child_gc_obj);
                }

                if (new_child) {
                    forward_map[child_gc_obj] = new_child;
                    value = Value(reinterpret_cast<Object*>(new_child->data()));
                }
            }
        });
        
        scan += obj->gc_size();
    }
    
    // 交换From和To空间
    size_t survived = static_cast<size_t>(new_space_->top() - new_space_->from_space());
    new_space_->SwapSpaces();
    
    // 计算回收的内存
    size_t collected = kNewSpaceSemiSize - survived;
    total_collected_ += collected;
    
    root_set_.Clear();
    return true;
}

GCObject* GCHeap::CopyObject(GCObject* obj) {
    size_t size = obj->gc_size();

    // 在To空间分配
    void* mem = new_space_->AllocateInToSpace(size);
    if (!mem) {
        // To空间不足，尝试晋升
        return PromoteObject(obj);
    }

    // 复制对象
    std::memcpy(mem, obj, size);
    GCObject* new_obj = reinterpret_cast<GCObject*>(mem);

    // 增加年龄
    new_obj->header()->IncrementAge();

    // 标记原对象为已转发
    obj->header()->SetForwarded(true);

    return new_obj;
}

GCObject* GCHeap::PromoteObject(GCObject* obj) {
    size_t size = obj->gc_size();
    
    void* mem = old_space_->Allocate(size);
    if (!mem) {
        return nullptr;
    }
    
    // 复制对象
    std::memcpy(mem, obj, size);
    GCObject* new_obj = reinterpret_cast<GCObject*>(mem);
    
    // 设置为老年代
    new_obj->header()->set_generation(GCGeneration::kOld);
    new_obj->header()->ClearAge();
    
    // 标记原对象为已转发
    obj->header()->SetForwarded(true);
    
    return new_obj;
}

// ==================== Mark-Compact ====================

bool GCHeap::MarkCompact() {
    ++full_gc_count_;
    
    // 收集根
    CollectRoots();
    
    // 标记阶段
    MarkPhase();
    
    // 压缩阶段
    CompactPhase();
    
    root_set_.Clear();
    return true;
}

void GCHeap::MarkPhase() {
    // 清除所有标记
    old_space_->IterateObjects([](GCObject* obj) {
        obj->header()->SetMarked(false);
    });
    
    // 从根开始标记
    MarkFromRoots();
}

void GCHeap::MarkFromRoots() {
    // 标记根引用的对象
    auto mark_root = [&](Value* value) {
        if (!value || !value->IsObject()) return;

        Object* obj = &value->object();
        GCObject* gc_obj = static_cast<GCObject*>(obj);

        MarkObject(gc_obj);
    };

    // 处理所有根
    for (auto* root : root_set_.stack_roots) {
        mark_root(root);
    }
    for (auto* root : root_set_.global_roots) {
        mark_root(root);
    }
    for (auto* root : root_set_.const_pool_roots) {
        mark_root(root);
    }
}

void GCHeap::MarkObject(GCObject* obj) {
    if (!obj || obj->header()->IsMarked()) {
        return;
    }
    
    // 标记对象
    obj->header()->SetMarked(true);
    
    // 递归标记子对象
    obj->GCTraverse(context_, [&](Context* ctx, Value& value) {
        if (value.IsObject()) {
            Object* child_obj = &value.object();
            GCObject* child_gc_obj = static_cast<GCObject*>(child_obj);
            MarkObject(child_gc_obj);
        }
    });
}

void GCHeap::CompactPhase() {
    // 计算新的位置
    uint8_t* new_top = old_space_->ComputeCompactTop();
    
    // 第一遍：计算转发地址
    std::unordered_map<GCObject*, GCObject*> forward_map;
    uint8_t* new_pos = old_space_->space_start();
    
    old_space_->IterateObjects([&](GCObject* obj) {
        if (obj->header()->IsMarked()) {
            forward_map[obj] = reinterpret_cast<GCObject*>(new_pos);
            new_pos += obj->gc_size();
        }
    });
    
    // 第二遍：移动对象
    old_space_->IterateLiveObjects([&](GCObject* obj) {
        GCObject* new_addr = forward_map[obj];
        if (new_addr != obj) {
            size_t size = obj->gc_size();
            std::memmove(new_addr, obj, size);
            
            // 调用移动回调
            new_addr->GCMoved(obj);
        }
    });
    
    // 更新引用
    auto update_refs = [&](Value* value) {
        if (!value || !value->IsObject()) return;

        Object* obj = &value->object();
        GCObject* gc_obj = static_cast<GCObject*>(obj);

        if (gc_obj->header()->generation() == GCGeneration::kOld) {
            auto it = forward_map.find(gc_obj);
            if (it != forward_map.end() && it->second != gc_obj) {
                *value = Value(reinterpret_cast<Object*>(it->second->data()));
            }
        }
    };
    
    // 更新根引用
    for (auto* root : root_set_.stack_roots) {
        update_refs(root);
    }
    for (auto* root : root_set_.global_roots) {
        update_refs(root);
    }
    for (auto* root : root_set_.const_pool_roots) {
        update_refs(root);
    }
    
    // 更新对象内部引用
    uint8_t* current = old_space_->space_start();
    while (current < new_pos) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        obj->GCTraverse(context_, [&](Context* ctx, Value& value) {
            update_refs(&value);
        });
        current += obj->gc_size();
    }
    
    // 更新top
    old_space_->set_top(new_pos);
}

void GCHeap::CollectRoots() {
    root_set_.Clear();
    
    if (!context_) {
        return;
    }
    
    // 获取Runtime和栈
    Runtime& runtime = const_cast<Runtime&>(context_->runtime());
    Stack& stack = runtime.stack();
    
    // 收集栈上的所有值
    for (size_t i = 0; i < stack.size(); ++i) {
        Value& val = stack.get(i);
        if (val.IsObject()) {
            root_set_.stack_roots.push_back(&val);
        }
    }
    
    // 收集本地常量池中的所有值
    LocalConstPool& local_pool = context_->local_const_pool();
    // 注意：LocalConstPool不直接暴露其内部存储，我们需要通过其他方式获取
    // 这里我们暂时不收集，因为在LocalConstPool的实现中，Value被存储在pool_中
    
    // 收集微任务队列中的所有值
    JobQueue& job_queue = context_->microtask_queue();
    for (auto& job : job_queue) {
        // Job包含func_, this_val_, argv_
        Value* func_ptr = const_cast<Value*>(&job.func());
        Value* this_ptr = const_cast<Value*>(&job.this_val());
        if (func_ptr->IsObject()) {
            root_set_.stack_roots.push_back(func_ptr);
        }
        if (this_ptr->IsObject()) {
            root_set_.stack_roots.push_back(this_ptr);
        }
        for (auto& arg : job.argv()) {
            Value* arg_ptr = const_cast<Value*>(&arg);
            if (arg_ptr->IsObject()) {
                root_set_.stack_roots.push_back(arg_ptr);
            }
        }
    }
    
    // 收集全局this对象
    Value& global_this = runtime.global_this();
    if (global_this.IsObject()) {
        root_set_.global_roots.push_back(&global_this);
    }
}

void GCHeap::AddRoot(Value* value) {
    // 可以添加到永久根集合
}

void GCHeap::RemoveRoot(Value* value) {
    // 从永久根集合移除
}

} // namespace mjs
