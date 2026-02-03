/**
 * @file gc_heap.cpp
 * @brief GC堆管理器实现
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件包含 GCHeap 类的实现：
 * - 初始化和构造
 * - 内存分配
 * - GC触发控制
 * - 统计信息
 * - 根集合管理
 * - Scavenge (新生代复制GC)
 * - Mark-Compact (老年代标记-压缩GC)
 */

#include <mjs/gc/gc_heap.h>

#include <cstring>
#include <algorithm>

#include <mjs/context.h>
#include <mjs/runtime.h>
#include <mjs/value/value.h>
#include <mjs/value/object/object.h>
#include <mjs/stack_frame.h>
#include <mjs/job_queue.h>
#include <mjs/gc/handle.h>

namespace mjs {

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

void* GCHeap::Allocate(size_t* total_size, GCGeneration* generation) {
    // 检查是否需要GC
    if (!in_gc_ && new_space_->used_size() > kNewSpaceSemiSize * gc_threshold_ / 100) {
        CollectGarbage(false);
    }

    void* mem = nullptr;

    // 大对象直接在老年代分配
    if (*total_size >= kLargeObjectThreshold) {
        mem = old_space_->Allocate(total_size);
        *generation = GCGeneration::kOld;
        if (!mem) {
            // 尝试完整GC
            if (!in_gc_ && CollectGarbage(true)) {
                mem = old_space_->Allocate(total_size);
            }
            // 如果GC后仍然空间不足，尝试扩容
            if (!mem && !in_gc_) {
                if (ExpandOldSpace(*total_size)) {
                    mem = old_space_->Allocate(total_size);
                }
            }
        }
    }
    else {
        // 在新生代分配
        mem = new_space_->Allocate(total_size);
        *generation = GCGeneration::kNew;
        if (!mem) {
            // 新生代满了，触发Scavenge
            if (!in_gc_ && Scavenge()) {
                mem = new_space_->Allocate(total_size);
            }
        }
    }

    if (mem) {
        total_allocated_ += *total_size;
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

void GCHeap::AddRoot(Value* value) {
    if (!value) {
        return;
    }
    // 添加到永久根集合（全局根）
    root_set_.global_roots.emplace(value);
}

void GCHeap::RemoveRoot(Value* value) {
    if (!value) {
        return;
    }
    // 从全局根集合移除
    auto& global_roots = root_set_.global_roots;
    global_roots.erase(value);
}

void GCHeap::IterateRoots(RootCallback callback, void* data) {
    if (!context_) {
        return;
    }

    // 遍历栈上的所有值
    Runtime& runtime = const_cast<Runtime&>(context_->runtime());
    Stack& stack = runtime.stack();
    for (size_t i = 0; i < stack.size(); ++i) {
        Value& val = stack.get(i);
        if (val.IsObject()) {
            callback(&val, data);
        }
    }

    // 遍历 HandleScope 栈中的所有句柄
    GCHandleScopeBase* scope = context_->current_handle_scope();
    while (scope) {
        const GCObject* const* data_ptr = scope->data();
        size_t size = scope->size();
        for (size_t i = 0; i < size; ++i) {
            // HandleScope 中存储的肯定是对象，无需类型检查
            GCObject* gc_obj = const_cast<GCObject*>(data_ptr[i]);
            if (gc_obj) {
                // 将 GCObject* 转换为 Value 供回调使用
                Value val(static_cast<Object*>(gc_obj));
                callback(&val, data);
            }
        }
        scope = scope->prev();
    }

    // 遍历微任务队列中的所有值
    JobQueue& job_queue = context_->microtask_queue();
    for (auto& job : job_queue) {
        // Job包含func_, this_val_, argv_
        Value* func_ptr = const_cast<Value*>(&job.func());
        Value* this_ptr = const_cast<Value*>(&job.this_val());
        if (func_ptr->IsObject()) {
            callback(func_ptr, data);
        }
        if (this_ptr->IsObject()) {
            callback(this_ptr, data);
        }
        for (auto& arg : job.argv()) {
            Value* arg_ptr = const_cast<Value*>(&arg);
            if (arg_ptr->IsObject()) {
                callback(arg_ptr, data);
            }
        }
    }

    // 不需要遍历全局常量表，全局常量对象及子对象理应分配到永久代，不应该参与回收，目前设计上全局常量表存储的Value都是基于引用计数的String和Def等

    // 遍历局部常量表，局部常量表目前没有放置对象，可以先不遍历

    // todo: 这里以后要在这里遍历global_this和class_def中的对象

    // 遍历用户手动添加的永久根（global_roots）
    for (auto* root : root_set_.global_roots) {
        callback(root, data);
    }
}

// ==================== Scavenge (Copying GC) ====================

bool GCHeap::Scavenge() {
    ++gc_count_;

    // 重置To空间分配指针
    new_space_->ResetToSpace();

    // 遍历所有根对象，将其复制到To空间
    IterateRoots([](Value* root, void* data) {
        GCHeap* heap = static_cast<GCHeap*>(data);
        heap->ProcessCopyOrReference(root);
    }, this);

    // Cheney扫描算法：遍历To空间中已复制的对象，处理其引用，这个时候遍历的就是从根向下找的可达对象
    // 扫描同时会持续将To中找到的对象的子对象复制到To区后，直到所有对象都被处理完毕
    uint8_t* scan = new_space_->to_space();
    while (scan < new_space_->to_space_top()) {
        GCObject* obj = reinterpret_cast<GCObject*>(scan);

        // 遍历对象的子对象，处理引用指针
        obj->GCTraverse(context_, [](Context* context, Value* child) {
            GCHeap* heap = context->gc_manager().heap();
            heap->ProcessCopyOrReference(child);
        });

        scan += obj->header()->size();
    }

    // 在交换空间前，遍历From空间（当前空间），调用死亡对象的析构函数
    // 死亡对象是未被转发的对象
    uint8_t* current = new_space_->from_space();
    uint8_t* end = new_space_->top();
    int i = 0;
    while (current < end) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        // 在调用析构函数前先记录大小，因为调用后虚函数表可能被破坏
        size_t obj_size = obj->header()->size();
        // 如果对象没有被转发，说明它是死亡对象
        if (!obj->header()->IsForwarded() && !obj->header()->IsDestructed()) {
            // 标记为已析构，避免重复调用
            obj->header()->SetDestructed(true);
            // 调用虚析构函数
            obj->~GCObject();
        }
        current += obj_size;
        ++i;
    }

    // 交换From和To空间
    size_t survived = static_cast<size_t>(new_space_->top() - new_space_->from_space());
    new_space_->SwapSpaces();

    // 计算回收的内存
    size_t collected = kNewSpaceSemiSize - survived;
    total_collected_ += collected;

    return true;
}

GCObject* GCHeap::CopyObject(GCObject* obj) {
    size_t size = obj->header()->size();

    // 在To空间分配
    void* mem = new_space_->AllocateInToSpace(&size);
    if (!mem) {
        // To空间不足，尝试晋升
        return PromoteObject(obj);
    }

    // 复制对象
    std::memcpy(mem, obj, size);
    GCObject* new_obj = reinterpret_cast<GCObject*>(mem);

    // 增加年龄
    new_obj->header()->IncrementAge();

    // 重置析构标记（复制的新对象不应该有析构标记）
    new_obj->header()->SetDestructed(false);

    // 转发指针
    obj->header()->SetForwardingAddress(new_obj);

    return new_obj;
}

GCObject* GCHeap::PromoteObject(GCObject* obj) {
    size_t size = obj->header()->size();

    void* mem = old_space_->Allocate(&size);
    if (!mem) {
        return nullptr;
    }

    // 复制对象
    std::memcpy(mem, obj, size);
    GCObject* new_obj = reinterpret_cast<GCObject*>(mem);

    // 设置为老年代
    new_obj->header()->set_generation(GCGeneration::kOld);
    new_obj->header()->ClearAge();

    // 重置析构标记（晋升的新对象不应该有析构标记）
    new_obj->header()->SetDestructed(false);

    obj->header()->SetForwardingAddress(new_obj);

    return new_obj;
}

void GCHeap::ProcessCopyOrReference(Value* value) {
    if (!value || !value->IsObject()) return;

    Object* obj = &value->object();
    GCObject* gc_obj = static_cast<GCObject*>(obj);

    if (gc_obj->header()->generation() != GCGeneration::kNew) {
        return;
    }

    GCObject* new_obj = nullptr;

    if (gc_obj->header()->IsForwarded()) {
        // 已经复制过，获取新地址
        new_obj = gc_obj->header()->GetForwardingAddress();
    }
    else if (gc_obj->header()->age() >= kTenureAgeThreshold) {
        // 晋升到老年代
        new_obj = PromoteObject(gc_obj);
    }
    else {
        // 复制到To空间
        new_obj = CopyObject(gc_obj);
    }

    if (new_obj) {
        // 更新Value中的引用
        *value = Value(static_cast<Object*>(new_obj));
    }
}

// ==================== Mark-Compact ====================

bool GCHeap::MarkCompact() {
    ++full_gc_count_;

    // 标记阶段
    MarkPhase();

    // 压缩阶段
    CompactPhase();

    return true;
}

void GCHeap::MarkPhase() {
    // 清除老年代区域中的所有对象的标记
    old_space_->IterateObjects([](GCObject* obj, void* data) {
        obj->header()->SetMarked(false);
    }, nullptr);

    // 遍历所有根并标记从根开始的所有可达对象
    IterateRoots([](Value* root, void* data) {
        if (!root || !root->IsObject()) {
            return;
        }

        Object* obj = &root->object();
        GCObject* gc_obj = static_cast<GCObject*>(obj);

        GCHeap* heap = static_cast<GCHeap*>(data);
        heap->MarkObject(gc_obj);
    }, this);
}

void GCHeap::MarkObject(GCObject* obj) {
    if (!obj || obj->header()->IsMarked()) {
        return;
    }

    // 标记对象
    obj->header()->SetMarked(true);

    // 递归标记子对象
    obj->GCTraverse(context_, [](Context* context, Value* child) {
        if (!child->IsObject()) {
            return;
        }
        Object* child_obj = &child->object();
        GCObject* child_gc_obj = static_cast<GCObject*>(child_obj);
        context->gc_manager().heap()->MarkObject(child_gc_obj);
    });
}

void GCHeap::CompactPhase() {
    // 在压缩之前，遍历所有对象，调用死亡对象（未标记）的析构函数
    old_space_->IterateObjects([](GCObject* obj, void* data) {
        // 在调用析构函数前先检查是否已析构，避免重复调用
        if (!obj->header()->IsMarked() && !obj->header()->IsDestructed()) {
            // 标记为已析构，避免重复调用
            obj->header()->SetDestructed(true);
            // 对象未标记，是死亡对象，调用析构函数
            obj->~GCObject();
        }
    }, nullptr);

    // 计算新的位置
    uint8_t* new_top = old_space_->ComputeCompactTop();

    // 第一遍：计算转发地址，使用内联转发指针
    OldSpace::CompactForwardData fwd_data;
    fwd_data.new_pos = old_space_->space_start();

    old_space_->IterateObjects(OldSpace::ComputeForwardingAddr, &fwd_data);

    // 第二遍：移动对象
    OldSpace::MoveObjectData move_data;
    move_data.heap = this;

    old_space_->IterateLiveObjects(OldSpace::MoveObject, &move_data);

    // 更新根引用（使用 IterateRoots 遍历所有根）
    IterateRoots([](Value* root, void* data) {
        OldSpace::UpdateReference(root);
    }, nullptr);

    // 更新对象内部引用
    uint8_t* current = old_space_->space_start();
    while (current < fwd_data.new_pos) {
        GCObject* obj = reinterpret_cast<GCObject*>(current);
        obj->GCTraverse(context_, [](Context* context, Value* child) {
            OldSpace::UpdateReference(child);
        });
        current += obj->header()->size();
    }

    // 清除转发标记
    old_space_->IterateObjects([](GCObject * obj, void* data) {
        obj->header()->SetForwardingAddress(nullptr);
    }, nullptr);

    // 更新top
    old_space_->set_top(fwd_data.new_pos);
}

bool GCHeap::ExpandOldSpace(size_t min_size) {
    // 步骤1：扩容（分配新内存，复制对象，设置转发地址）
    if (!old_space_->Expand(min_size)) {
        return false;
    }

    // 步骤2：更新所有引用（使用转发地址）
    // 更新根引用
    IterateRoots([](Value* value, void* data) {
        GCHeap* heap = static_cast<GCHeap*>(data);
        OldSpace::UpdateReference(value);
    }, this);

    // 更新对象内部引用
    old_space_->IterateObjects([](GCObject* obj, void* data) {
        GCHeap* heap = static_cast<GCHeap*>(data);
        obj->GCTraverse(heap->context_, [](Context* context, Value* child) {
            OldSpace::UpdateReference(child);
        });
    }, this);

    // 步骤3：完成扩容（清除转发标记，释放旧内存）
    old_space_->FinishExpand();

    return true;
}

} // namespace mjs
