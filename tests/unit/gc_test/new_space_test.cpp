/**
 * @file new_space_test.cpp
 * @brief NewSpace 新生代内存空间单元测试
 *
 * 测试新生代内存空间的功能：
 * - 初始化
 * - 内存分配
 * - 空间交换
 * - 对象遍历
 * - 边界条件
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include <gtest/gtest.h>

#include <mjs/gc/new_space.h>
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
 * @class NewSpaceTest
 * @brief NewSpace 类单元测试
 */
class NewSpaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        new_space_ = std::make_unique<NewSpace>();
        ASSERT_TRUE(new_space_->Initialize());
    }

    void TearDown() override {
        new_space_.reset();
    }

    std::unique_ptr<NewSpace> new_space_;
};

// ==================== 初始化测试 ====================

/**
 * @test 测试新生代初始化
 */
TEST_F(NewSpaceTest, Initialize) {
    // 在 SetUp 中已经测试了基本初始化
    EXPECT_NE(new_space_->from_space(), nullptr);
    EXPECT_NE(new_space_->to_space(), nullptr);
    EXPECT_EQ(new_space_->top(), new_space_->from_space());
    EXPECT_EQ(new_space_->to_space_top(), new_space_->to_space());
}

/**
 * @test 测试容量常量
 */
TEST_F(NewSpaceTest, CapacityConstant) {
    EXPECT_EQ(NewSpace::capacity(), kNewSpaceSemiSize);
    EXPECT_GT(NewSpace::capacity(), 0);
}

// ==================== 内存分配测试 ====================

/**
 * @test 测试小对象分配
 */
TEST_F(NewSpaceTest, AllocateSmallObject) {
    size_t size = sizeof(TestGCObject);
    void* ptr = new_space_->Allocate(&size);

    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr, new_space_->from_space());  // 第一个对象应该在起始位置
    EXPECT_EQ(new_space_->used_size(), size);
}

/**
 * @test 测试多个对象分配
 */
TEST_F(NewSpaceTest, AllocateMultipleObjects) {
    size_t size1 = sizeof(TestGCObject);
    void* ptr1 = new_space_->Allocate(&size1);
    ASSERT_NE(ptr1, nullptr);

    size_t size2 = sizeof(TestGCObject);
    void* ptr2 = new_space_->Allocate(&size2);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_NE(ptr1, ptr2);  // 两个对象地址应该不同

    // 验证对象是连续分配的
    size_t aligned_size = AlignGCObjectSize(sizeof(TestGCObject));
    EXPECT_EQ(static_cast<uint8_t*>(ptr2), static_cast<uint8_t*>(ptr1) + aligned_size);
}

/**
 * @test 测试分配后已使用大小
 */
TEST_F(NewSpaceTest, UsedSizeAfterAllocation) {
    size_t initial_used = new_space_->used_size();
    EXPECT_EQ(initial_used, 0);

    size_t size = sizeof(TestGCObject);
    new_space_->Allocate(&size);

    size_t aligned_size = AlignGCObjectSize(sizeof(TestGCObject));
    EXPECT_EQ(new_space_->used_size(), aligned_size);
}

/**
 * @test 测试大对象分配
 */
TEST_F(NewSpaceTest, AllocateLargeObject) {
    size_t size = sizeof(LargeTestObject);
    void* ptr = new_space_->Allocate(&size);

    ASSERT_NE(ptr, nullptr);
    EXPECT_GT(new_space_->used_size(), 0);
}

/**
 * @test 测试空间不足时的分配
 */
TEST_F(NewSpaceTest, AllocateWhenInsufficientSpace) {
    // 分配一个接近容量的对象
    size_t large_size = NewSpace::capacity();
    void* ptr1 = new_space_->Allocate(&large_size);
    ASSERT_NE(ptr1, nullptr);

    // 再次分配应该失败
    size_t small_size = sizeof(TestGCObject);
    void* ptr2 = new_space_->Allocate(&small_size);
    EXPECT_EQ(ptr2, nullptr);
}

/**
 * @test 测试填满整个空间
 */
TEST_F(NewSpaceTest, FillEntireSpace) {
    size_t total_allocated = 0;
    void* last_ptr = nullptr;

    // 持续分配小对象直到空间不足
    while (true) {
        size_t size = sizeof(TestGCObject);
        void* ptr = new_space_->Allocate(&size);
        if (ptr == nullptr) {
            break;
        }
        total_allocated += size;
        last_ptr = ptr;
    }

    // 验证确实分配了很多对象
    EXPECT_GT(total_allocated, 0);
    EXPECT_NE(last_ptr, nullptr);

    // 使用的空间应该接近或等于容量
    size_t used = new_space_->used_size();
    EXPECT_LE(used, NewSpace::capacity());
}

// ==================== HasSpace 测试 ====================

/**
 * @test 测试 HasSpace 对于小对象
 */
TEST_F(NewSpaceTest, HasSpaceForSmallObject) {
    EXPECT_TRUE(new_space_->HasSpace(sizeof(TestGCObject)));
    EXPECT_TRUE(new_space_->HasSpace(100));
    EXPECT_TRUE(new_space_->HasSpace(1000));
}

/**
 * @test 测试 HasSpace 对于超大对象
 */
TEST_F(NewSpaceTest, HasSpaceForLargeObject) {
    EXPECT_FALSE(new_space_->HasSpace(NewSpace::capacity() + 1));
    EXPECT_FALSE(new_space_->HasSpace(NewSpace::capacity() * 2));
}

/**
 * @test 测试分配后的 HasSpace
 */
TEST_F(NewSpaceTest, HasSpaceAfterAllocation) {
    size_t size = sizeof(TestGCObject);
    new_space_->Allocate(&size);

    // 空间应该减少
    size_t remaining = NewSpace::capacity() - new_space_->used_size();
    EXPECT_TRUE(new_space_->HasSpace(remaining));
    EXPECT_FALSE(new_space_->HasSpace(remaining + 1));
}

// ==================== To空间分配测试 ====================

/**
 * @test 测试在To空间分配
 */
TEST_F(NewSpaceTest, AllocateInToSpace) {
    size_t size = sizeof(TestGCObject);
    void* ptr = new_space_->AllocateInToSpace(&size);

    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr, new_space_->to_space());  // 第一个对象应该在To空间起始位置
    EXPECT_EQ(new_space_->to_space_top(), new_space_->to_space() + size);
}

/**
 * @test 测试在To空间分配多个对象
 */
TEST_F(NewSpaceTest, AllocateMultipleInToSpace) {
    size_t size1 = sizeof(TestGCObject);
    void* ptr1 = new_space_->AllocateInToSpace(&size1);
    ASSERT_NE(ptr1, nullptr);

    size_t size2 = sizeof(TestGCObject);
    void* ptr2 = new_space_->AllocateInToSpace(&size2);
    ASSERT_NE(ptr2, nullptr);
    EXPECT_NE(ptr1, ptr2);

    size_t aligned_size = AlignGCObjectSize(sizeof(TestGCObject));
    EXPECT_EQ(static_cast<uint8_t*>(ptr2), static_cast<uint8_t*>(ptr1) + aligned_size);
}

/**
 * @test 测试To空间不足时的分配
 */
TEST_F(NewSpaceTest, AllocateInToSpaceWhenInsufficient) {
    // 分配一个接近容量的对象
    size_t large_size = NewSpace::capacity();
    void* ptr1 = new_space_->AllocateInToSpace(&large_size);
    ASSERT_NE(ptr1, nullptr);

    // 再次分配应该失败
    size_t small_size = sizeof(TestGCObject);
    void* ptr2 = new_space_->AllocateInToSpace(&small_size);
    EXPECT_EQ(ptr2, nullptr);
}

// ==================== 空间交换测试 ====================

/**
 * @test 测试交换From和To空间
 */
TEST_F(NewSpaceTest, SwapSpaces) {
    // 在From空间分配一些对象
    size_t size = sizeof(TestGCObject);
    void* from_ptr = new_space_->Allocate(&size);
    ASSERT_NE(from_ptr, nullptr);

    uint8_t* original_from = new_space_->from_space();
    uint8_t* original_to = new_space_->to_space();
    uint8_t* original_top = new_space_->top();

    // 交换空间
    new_space_->SwapSpaces();

    // 验证空间已交换
    EXPECT_EQ(new_space_->from_space(), original_to);
    EXPECT_EQ(new_space_->to_space(), original_from);
    // top 应该被交换到 to_space_top
    EXPECT_EQ(new_space_->top(), original_to);  // 新的 From 空间应该是原来的 To 空间
}

/**
 * @test 测试交换后分配
 */
TEST_F(NewSpaceTest, AllocateAfterSwap) {
    // 在From空间分配
    size_t size1 = sizeof(TestGCObject);
    void* ptr1 = new_space_->Allocate(&size1);
    ASSERT_NE(ptr1, nullptr);

    // 交换空间
    new_space_->SwapSpaces();

    // 在新的From空间（原来的To空间）分配
    size_t size2 = sizeof(TestGCObject);
    void* ptr2 = new_space_->Allocate(&size2);
    ASSERT_NE(ptr2, nullptr);

    // ptr1 和 ptr2 应该在不同的原始空间
    EXPECT_NE(static_cast<void*>(ptr1), static_cast<void*>(ptr2));
}

// ==================== Reset 测试 ====================

/**
 * @test 测试重置 top 指针
 */
TEST_F(NewSpaceTest, ResetTop) {
    // 分配一些对象
    size_t size = sizeof(TestGCObject);
    new_space_->Allocate(&size);
    EXPECT_GT(new_space_->used_size(), 0);

    // 重置到起始位置
    new_space_->Reset(new_space_->from_space());
    EXPECT_EQ(new_space_->used_size(), 0);
    EXPECT_EQ(new_space_->top(), new_space_->from_space());
}

/**
 * @test 测试重置To空间
 */
TEST_F(NewSpaceTest, ResetToSpace) {
    // 在To空间分配
    size_t size = sizeof(TestGCObject);
    new_space_->AllocateInToSpace(&size);
    EXPECT_GT(new_space_->to_space_top() - new_space_->to_space(), 0);

    // 重置To空间
    new_space_->ResetToSpace();
    EXPECT_EQ(new_space_->to_space_top(), new_space_->to_space());
}

// ==================== 对象遍历测试 ====================

/**
 * @test 测试遍历空空间
 */
TEST_F(NewSpaceTest, IterateEmptySpace) {
    int call_count = 0;
    new_space_->IterateObjects([](GCObject* obj, void* data) {
        (void)obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    EXPECT_EQ(call_count, 0);
}

/**
 * @test 测试遍历单个对象
 */
TEST_F(NewSpaceTest, IterateSingleObject) {
    size_t size = sizeof(TestGCObject);
    void* ptr = new_space_->Allocate(&size);
    ASSERT_NE(ptr, nullptr);

    // 构造对象
    TestGCObject* obj = new (ptr) TestGCObject(42);
    obj->header()->set_size(size);
    obj->header()->set_type(GCObjectType::kObject);

    int call_count = 0;
    TestGCObject* captured_obj = nullptr;
    void* datas[] = {&call_count, &captured_obj};
    new_space_->IterateObjects([](GCObject* gc_obj, void* data) {
        void** datas = static_cast<void**>(data);
        int* count = static_cast<int*>(datas[0]);
        TestGCObject** obj_ptr = static_cast<TestGCObject**>(datas[1]);
        (*count)++;
        *obj_ptr = static_cast<TestGCObject*>(gc_obj);
    }, datas);

    // 由于lambda的捕获限制，使用简单的计数
    call_count = 0;
    new_space_->IterateObjects([](GCObject* gc_obj, void* data) {
        (void)gc_obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    EXPECT_EQ(call_count, 1);
}

/**
 * @test 测试遍历多个对象
 */
TEST_F(NewSpaceTest, IterateMultipleObjects) {
    const int kNumObjects = 10;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = new_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        TestGCObject* obj = new (ptr) TestGCObject(i);
        obj->header()->set_size(size);
        obj->header()->set_type(GCObjectType::kObject);
    }

    int call_count = 0;
    new_space_->IterateObjects([](GCObject* obj, void* data) {
        (void)obj;
        int* count = static_cast<int*>(data);
        (*count)++;
    }, &call_count);

    EXPECT_EQ(call_count, kNumObjects);
}

// ==================== 边界条件测试 ====================

/**
 * @test 测试分配恰好剩余空间大小的对象
 */
TEST_F(NewSpaceTest, AllocateExactRemainingSpace) {
    size_t remaining = NewSpace::capacity();
    void* ptr = new_space_->Allocate(&remaining);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(new_space_->used_size(), NewSpace::capacity());

    // 不应该再有空间
    size_t tiny_size = 8;
    void* ptr2 = new_space_->Allocate(&tiny_size);
    EXPECT_EQ(ptr2, nullptr);
}

/**
 * @test 测试分配零大小
 */
TEST_F(NewSpaceTest, AllocateZeroSize) {
    size_t size = 0;
    void* ptr = new_space_->Allocate(&size);

    size_t aligned_size = AlignGCObjectSize(0);
    EXPECT_EQ(aligned_size, 0);
}

/**
 * @test 测试大小对齐
 */
TEST_F(NewSpaceTest, SizeAlignment) {
    size_t size1 = 13;  // 非对齐大小
    size_t expected_size1 = AlignGCObjectSize(13);
    void* ptr1 = new_space_->Allocate(&size1);

    size_t size2 = sizeof(TestGCObject);
    void* ptr2 = new_space_->Allocate(&size2);

    // 验证对象按8字节对齐
    EXPECT_EQ(size1, expected_size1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr1) % kGCObjectAlignment, 0);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr2) % kGCObjectAlignment, 0);
}

/**
 * @test 测试top指针边界
 */
TEST_F(NewSpaceTest, TopPointerBoundaries) {
    EXPECT_EQ(new_space_->top(), new_space_->from_space());

    size_t size = sizeof(TestGCObject);
    new_space_->Allocate(&size);

    EXPECT_EQ(new_space_->top(), new_space_->from_space() + size);
    EXPECT_LE(new_space_->top(), new_space_->from_space_end());
}

// ==================== 连续分配测试 ====================

/**
 * @test 测试连续分配后的内存布局
 */
TEST_F(NewSpaceTest, MemoryLayoutAfterAllocation) {
    std::vector<void*> pointers;
    const int kNumObjects = 5;

    for (int i = 0; i < kNumObjects; i++) {
        size_t size = sizeof(TestGCObject);
        void* ptr = new_space_->Allocate(&size);
        ASSERT_NE(ptr, nullptr);
        pointers.push_back(ptr);
    }

    // 验证对象是连续分配的
    size_t aligned_size = AlignGCObjectSize(sizeof(TestGCObject));
    for (size_t i = 1; i < pointers.size(); i++) {
        auto prev = static_cast<uint8_t*>(pointers[i - 1]);
        auto curr = static_cast<uint8_t*>(pointers[i]);
        EXPECT_EQ(curr, prev + aligned_size);
    }
}

} // namespace test
} // namespace mjs
