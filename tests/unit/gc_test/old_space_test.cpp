/**
 * @file old_space_test.cpp
 * @brief OldSpace 老年代内存空间单元测试
 *
 * 测试老年代内存空间的功能：
 * - 初始化
 * - 内存分配
 * - 空间扩容
 * - 对象遍历
 * - 压缩计算
 * - 边界条件
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>

#include <mjs/gc/old_space.h>
#include <mjs/gc/gc_object.h>
#include <cstring>

namespace mjs {
namespace test {

// 测试用的简单 GCObject 派生类
class TestGCObject : public GCObject {
public:
    TestGCObject() : data_(0) {}
    explicit TestGCObject(int data) : data_(data) {}

    int data() const { return data_; }
    void set_data(int data) { data_ = data; }

    void GCTraverse(Context* context, GCTraverseCallback callback) override {
        (void)context;
        (void)callback;
    }

private:
    int data_;
};

// 测试用的较大对象
class LargeTestObject : public GCObject {
public:
    static constexpr size_t kDataSize = 1024;

    LargeTestObject() {
        std::memset(data_, 0, kDataSize);
    }

    void GCTraverse(Context* context, GCTraverseCallback callback) override {
        (void)context;
        (void)callback;
    }

private:
    char data_[kDataSize];
};

/**
 * @class OldSpaceTest
 * @brief OldSpace 类单元测试
 */
class OldSpaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        old_space_ = std::make_unique<OldSpace>();
        ASSERT_TRUE(old_space_->Initialize(kOldSpaceInitialSize));
    }

    void TearDown() override {
        old_space_.reset();
    }

    std::unique_ptr<OldSpace> old_space_;
};

// ==================== 初始化测试 ====================

/**
 * @test 测试老年代初始化
 */
TEST_F(OldSpaceTest, Initialize) {
    EXPECT_NE(old_space_->space_start(), nullptr);
    EXPECT_EQ(old_space_->top(), old_space_->space_start());
    EXPECT_EQ(old_space_->capacity(), kOldSpaceInitialSize);
    EXPECT_EQ(old_space_->used_size(), 0);
}

/**
 * @test 测试自定义大小初始化
 */
TEST_F(OldSpaceTest, InitializeWithCustomSize) {
    OldSpace custom_space;
    size_t custom_size = 2 * 1024 * 1024;  // 2MB
    ASSERT_TRUE(custom_space.Initialize(custom_size));

    EXPECT_EQ(custom_space.capacity(), custom_size);
}

/**
 * @test 测试初始化零大小
 */
TEST_F(OldSpaceTest, InitializeWithZeroSize) {
    OldSpace custom_space;
    // 零大小应该能初始化成功（虽然分配会失败）
    bool result = custom_space.Initialize(0);
    if (result) {
        EXPECT_EQ(custom_space.capacity(), 0);
    }
}

// ==================== 内存分配测试 ====================

/**
 * @test 测试小对象分配
 */
TEST_F(OldSpaceTest, AllocateSmallObject) {
    size_t size = sizeof(TestGCObject);
    void* ptr = old_space_->Allocate(&size);

    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr, old_space_->space_start());
    EXPECT_EQ(old_space_->used_size(), size);
}

/**
 * @test 测试多个对象分配
 */
TEST_F(OldSpaceTest, AllocateMultipleObjects) {
    std::vector<void*> pointers;
    const int kNumObjects = 10;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        pointers.push_back(ptr);
    }

    // 验证所有指针都不同
    for (size_t i = 0; i < pointers.size(); i++) {
        for (size_t j = i + 1; j < pointers.size(); j++) {
            EXPECT_NE(pointers[i], pointers[j]);
        }
    }

    // 验证对象是连续分配的
    size_t aligned_size = AlignGCObjectSize(sizeof(TestGCObject));
    for (size_t i = 1; i < pointers.size(); i++) {
        auto prev = static_cast<uint8_t*>(pointers[i - 1]);
        auto curr = static_cast<uint8_t*>(pointers[i]);
        EXPECT_EQ(curr, prev + aligned_size);
    }
}

/**
 * @test 测试大对象分配
 */
TEST_F(OldSpaceTest, AllocateLargeObject) {
    size_t size = sizeof(LargeTestObject);
    void* ptr = old_space_->Allocate(&size);

    ASSERT_NE(ptr, nullptr);
    EXPECT_GT(old_space_->used_size(), 0);
}

/**
 * @test 测试空间不足时的分配
 */
TEST_F(OldSpaceTest, AllocateWhenInsufficientSpace) {
    // 分配接近容量的对象
    size_t large_size = old_space_->capacity();
    void* ptr1 = old_space_->Allocate(&large_size);
    ASSERT_NE(ptr1, nullptr);

    // 再次分配应该失败
    size_t small_size = sizeof(TestGCObject);
    void* ptr2 = old_space_->Allocate(&small_size);
    EXPECT_EQ(ptr2, nullptr);
}

/**
 * @test 测试填满整个空间
 */
TEST_F(OldSpaceTest, FillEntireSpace) {
    size_t total_allocated = 0;
    void* last_ptr = nullptr;
    int object_count = 0;

    // 持续分配小对象直到空间不足
    while (true) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        if (ptr == nullptr) {
            break;
        }
        total_allocated += size;
        last_ptr = ptr;
        object_count++;
    }

    // 验证确实分配了很多对象
    EXPECT_GT(total_allocated, 0);
    EXPECT_NE(last_ptr, nullptr);
    EXPECT_GT(object_count, 0);

    // 使用的空间应该接近或等于容量
    size_t used = old_space_->used_size();
    EXPECT_LE(used, old_space_->capacity());
}

// ==================== 空间边界测试 ====================

/**
 * @test 测试空间起始和结束地址
 */
TEST_F(OldSpaceTest, SpaceBoundaries) {
    EXPECT_NE(old_space_->space_start(), nullptr);
    EXPECT_NE(old_space_->space_end(), nullptr);
    EXPECT_GT(old_space_->space_end(), old_space_->space_start());
    EXPECT_EQ(old_space_->space_end(), old_space_->space_start() + old_space_->capacity());
}

/**
 * @test 测试top指针位置
 */
TEST_F(OldSpaceTest, TopPointerPosition) {
    EXPECT_EQ(old_space_->top(), old_space_->space_start());

    size_t size = sizeof(TestGCObject);
    old_space_->Allocate(&size);

    EXPECT_EQ(old_space_->top(), old_space_->space_start() + size);
    EXPECT_LE(old_space_->top(), old_space_->space_end());
}

/**
 * @test 测试分配后容量和已使用大小
 */
TEST_F(OldSpaceTest, CapacityAndUsedSize) {
    size_t initial_used = old_space_->used_size();
    EXPECT_EQ(initial_used, 0);

    size_t size1 = sizeof(TestGCObject);
    old_space_->Allocate(&size1);

    size_t size2 = sizeof(TestGCObject);
    old_space_->Allocate(&size2);

    size_t aligned_size = AlignGCObjectSize(sizeof(TestGCObject));
    EXPECT_EQ(old_space_->used_size(), aligned_size * 2);
}

// ==================== 对象遍历测试 ====================

/**
 * @test 测试遍历空空间
 */
TEST_F(OldSpaceTest, IterateEmptySpace) {
    int call_count = 0;
    old_space_->IterateObjects([](GCObject* obj, void* data) {
        (void)obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    EXPECT_EQ(call_count, 0);
}

/**
 * @test 测试遍历单个对象
 */
TEST_F(OldSpaceTest, IterateSingleObject) {
    size_t size = sizeof(TestGCObject);
    void* ptr = old_space_->Allocate(&size);
    ASSERT_NE(ptr, nullptr);

    // 构造对象
    TestGCObject* obj = new (ptr) TestGCObject(42);
    obj->header()->set_size(size);
    obj->header()->set_type(GCObjectType::kObject);

    int call_count = 0;
    old_space_->IterateObjects([](GCObject* gc_obj, void* data) {
        (void)gc_obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    EXPECT_EQ(call_count, 1);
}

/**
 * @test 测试遍历多个对象
 */
TEST_F(OldSpaceTest, IterateMultipleObjects) {
    const int kNumObjects = 10;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        TestGCObject* obj = new (ptr) TestGCObject(i);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);
    }

    int call_count = 0;
    old_space_->IterateObjects([](GCObject* obj, void* data) {
        (void)obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    EXPECT_EQ(call_count, kNumObjects);
}

// ==================== 存活对象遍历测试 ====================

/**
 * @test 测试遍历存活对象（标记对象）
 */
TEST_F(OldSpaceTest, IterateLiveObjects) {
    const int kNumObjects = 10;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        TestGCObject* obj = new (ptr) TestGCObject(i);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);

        // 标记奇数索引的对象
        if (i % 2 == 1) {
            obj->header()->SetMarked(true);
        }
    }

    int call_count = 0;
    old_space_->IterateLiveObjects([](GCObject* obj, void* data) {
        (void)obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    // 只有标记的对象被遍历
    EXPECT_EQ(call_count, kNumObjects / 2);
}

/**
 * @test 测试遍历存活对象（全部标记）
 */
TEST_F(OldSpaceTest, IterateLiveObjectsAllMarked) {
    const int kNumObjects = 5;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        TestGCObject* obj = new (ptr) TestGCObject(i);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);
        obj->header()->SetMarked(true);  // 全部标记
    }

    int call_count = 0;
    old_space_->IterateLiveObjects([](GCObject* obj, void* data) {
        (void)obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    EXPECT_EQ(call_count, kNumObjects);
}

/**
 * @test 测试遍历存活对象（无标记）
 */
TEST_F(OldSpaceTest, IterateLiveObjectsNoneMarked) {
    const int kNumObjects = 5;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        TestGCObject* obj = new (ptr) TestGCObject(i);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);
        // 不标记任何对象
    }

    int call_count = 0;
    old_space_->IterateLiveObjects([](GCObject* obj, void* data) {
        (void)obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    EXPECT_EQ(call_count, 0);
}

// ==================== 压缩计算测试 ====================

/**
 * @test 测试计算压缩后的top（空空间）
 */
TEST_F(OldSpaceTest, ComputeCompactTopEmpty) {
    uint8_t* new_top = old_space_->ComputeCompactTop();
    EXPECT_EQ(new_top, old_space_->space_start());
}

/**
 * @test 测试计算压缩后的top（全部存活）
 */
TEST_F(OldSpaceTest, ComputeCompactTopAllLive) {
    const int kNumObjects = 5;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        TestGCObject* obj = new (ptr) TestGCObject(i);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);
        obj->header()->SetMarked(true);
    }

    uint8_t* new_top = old_space_->ComputeCompactTop();
    // 全部存活，压缩后top应该不变
    EXPECT_EQ(new_top, old_space_->top());
}

/**
 * @test 测试计算压缩后的top（部分存活）
 */
TEST_F(OldSpaceTest, ComputeCompactTopPartialLive) {
    const int kNumObjects = 10;
    size_t aligned_size = AlignGCObjectSize(sizeof(TestGCObject));

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        TestGCObject* obj = new (ptr) TestGCObject(i);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);

        // 只标记偶数索引的对象
        if (i % 2 == 0) {
            obj->header()->SetMarked(true);
        }
    }

    uint8_t* new_top = old_space_->ComputeCompactTop();
    // 一半存活，压缩后top应该是原来的一半
    size_t expected_offset = (kNumObjects / 2) * aligned_size;
    EXPECT_EQ(new_top, old_space_->space_start() + expected_offset);
}

// ==================== set_top 测试 ====================

/**
 * @test 测试设置新的top指针
 */
TEST_F(OldSpaceTest, SetTop) {
    // 分配一些对象
    size_t size1 = sizeof(TestGCObject);
    old_space_->Allocate(&size1);
    size_t size2 = sizeof(TestGCObject);
    old_space_->Allocate(&size2);

    EXPECT_GT(old_space_->used_size(), 0);

    // 设置新的top（模拟压缩后）
    old_space_->set_top(old_space_->space_start());
    EXPECT_EQ(old_space_->top(), old_space_->space_start());
}

// ==================== 扩容测试 ====================

/**
 * @test 测试空间扩容
 */
TEST_F(OldSpaceTest, ExpandSpace) {
    // 分配一些对象
    const int kNumObjects = 10;
    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        TestGCObject* obj = new (ptr) TestGCObject(i);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);
    }

    size_t old_capacity = old_space_->capacity();
    uint8_t* old_start = old_space_->space_start();

    // 扩容
    bool result = old_space_->Expand(sizeof(TestGCObject));
    EXPECT_TRUE(result);

    // 验证容量增加
    EXPECT_GT(old_space_->capacity(), old_capacity);

    // 验证旧内存地址保存
    EXPECT_EQ(old_space_->old_space_start(), old_start);

    // 完成扩容
    old_space_->FinishExpand();

    // 验证旧内存已释放
    EXPECT_EQ(old_space_->old_space_start(), nullptr);
}

/**
 * @test 测试扩容后对象转发地址设置
 */
TEST_F(OldSpaceTest, ExpandWithForwardingAddresses) {
    std::vector<TestGCObject*> old_objects;
    const int kNumObjects = 5;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = old_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);

        // 先创建一个临时对象来初始化 data 成员
        TestGCObject temp_obj(i);

        // 将整个对象（包括 vtable 和 header）复制到分配的内存
        std::memcpy(ptr, &temp_obj, size);

        // 然后初始化 header 字段
        TestGCObject* obj = reinterpret_cast<TestGCObject*>(ptr);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);
        obj->header()->set_generation(GCGeneration::kOld);
        obj->header()->SetMarked(false);
        obj->header()->SetDestructed(false);

        old_objects.push_back(obj);
    }

    // 扩容
    ASSERT_TRUE(old_space_->Expand(sizeof(TestGCObject)));

    // 验证所有旧对象都有转发地址
    for (auto* old_obj : old_objects) {
        EXPECT_TRUE(old_obj->header()->IsForwarded());
        EXPECT_NE(old_obj->header()->GetForwardingAddress(), nullptr);
    }

    // 完成扩容
    old_space_->FinishExpand();

    // 验证转发地址已清除，已经被释放，无法访问
    //for (auto* old_obj : old_objects) {
    //    EXPECT_FALSE(old_obj->header()->IsForwarded());
    //    EXPECT_EQ(old_obj->header()->GetForwardingAddress(), nullptr);
    //}
}

// ==================== 边界条件测试 ====================

/**
 * @test 测试分配零大小
 */
TEST_F(OldSpaceTest, AllocateZeroSize) {
    size_t size = 0;
    void* ptr = old_space_->Allocate(&size);
    ASSERT_NE(ptr, nullptr);

    // 对齐后应该是 0
    size_t aligned_size = AlignGCObjectSize(0);
    ASSERT_EQ(aligned_size, 0);
}

/**
 * @test 测试大小对齐
 */
TEST_F(OldSpaceTest, SizeAlignment) {
    size_t size1 = 13;  // 非对齐大小
    size_t expected_size1 = AlignGCObjectSize(13);
    void* ptr1 = old_space_->Allocate(&size1);

    size_t size2 = sizeof(TestGCObject);
    void* ptr2 = old_space_->Allocate(&size2);

    // 验证对象按8字节对齐
    EXPECT_EQ(size1, expected_size1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr1) % kGCObjectAlignment, 0);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr2) % kGCObjectAlignment, 0);
}

} // namespace test
} // namespace mjs
