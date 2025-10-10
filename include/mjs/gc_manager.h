/**
 * @file gc_manager.h
 * @brief JavaScript 垃圾回收管理器
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的垃圾回收管理器，负责自动内存管理和
 * 对象生命周期管理，使用标记-清除算法进行垃圾回收。
 */

#pragma once

#include <iostream>

#include <mjs/noncopyable.h>
#include <mjs/object.h>

namespace mjs {

/**
 * @class GCManager
 * @brief 垃圾回收管理器类
 *
 * 负责 JavaScript 引擎中的垃圾回收功能：
 * - 标记-清除垃圾回收算法
 * - 对象引用计数管理
 * - 循环引用检测和回收
 * - 对象树遍历和标记
 *
 * @note 继承自 noncopyable 确保单例特性
 * @see Object 对象基类
 */
class GCManager : public noncopyable {
public:
    /**
     * @brief 默认构造函数
     */
    GCManager() {}

    /**
     * @brief 执行垃圾回收
     *
     * 使用标记-清除算法进行垃圾回收，包括以下步骤：
     * 1. 标记阶段：遍历所有对象，标记可达对象
     * 2. 清除阶段：回收不可达对象的内存
     * 3. 引用计数调整：处理循环引用
     *
     * @param context 执行上下文指针
     */
    void GC(Context* context) {
        intrusive_list<Object> tmp_list;

        // 第一阶段：标记可达对象
        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(!cur.gc_mark());

            // 遍历子对象，处理弱引用
            cur.GCForEachChild(context, &tmp_list, [](Context* context, intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                assert(obj.ref_count() > 0);
                obj.WeakDereference();
                // 如果子对象引用计数为0且已被标记，说明父对象即将被释放，子对象也会被释放
                // 如果子对象已经被标记但父对象还存在，说明子对象也需要被标记
                if (obj.ref_count() == 0 && obj.gc_mark()) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;

            // 已扫描的对象，标记为已访问
            cur.set_gc_mark(true);
            if (cur.ref_count() == 0) {
                cur.unlink();
                tmp_list.push_back(cur);
            }
        }

        // 检查 object_list_ 中是否有未被回收的对象，可能存在内存泄漏
        // 检查 Value::IsObject 是否没有添加对应的对象类型

        // 第二阶段：重置不可回收对象的标记
        it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(cur.ref_count() > 0);

            // 剩下的都是不可回收的对象，重置标记
            cur.set_gc_mark(false);

            // 处理不可回收对象的子对象引用
            cur.GCForEachChild(context, &object_list_, [](Context* context, intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                // 不可回收的对象引用的子对象，当然也可能被回收，需要重新引用
                obj.Reference();
                if (obj.ref_count() == 1) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;
        }

        // 第三阶段：恢复剩余 object_list_ 中子对象的引用计数
        // 释放对象时，子对象的引用计数也要加回去
        it = tmp_list.begin();
        while (it != tmp_list.end()) {
            Object& cur = *it;

            cur.GCForEachChild(context, &object_list_, [](Context* context, intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                obj.Reference();
            });

            ++it;
        }

        // 第四阶段：释放临时列表中的对象
        // 注意：需要手动初始化释放对象，但这里只需要 Value 自动析构
        // 如果没有，引用计数不为0，引用计数不为0就不会释放这些对象
        while (!tmp_list.empty()) {
            Object& obj = tmp_list.front();
            // assert(obj.ref_count() == 0);
            assert(obj.gc_mark());
            // obj.WeakDereference();
            delete &obj;
        }
    }

	/**
	 * @brief 添加对象到垃圾回收管理器
	 * @param object 对象指针
	 */
	void AddObject(Object* object) {
		object_list_.push_back(*object);
	}

    /**
     * @brief 打印对象树结构
     *
     * 用于调试目的，打印所有对象及其子对象的关系和引用计数。
     *
     * @param context 执行上下文指针
     */
    void PrintObjectTree(Context* context) {
        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(!cur.gc_mark());

            std::cout << Value(&cur).ToString(context).string_view();
            std::cout << " ref_count:" << cur.ref_count();
            std::cout << std::endl;

            cur.GCForEachChild(context, nullptr, [](Context* context, intrusive_list<Object>* list, const Value& child) {
                std::cout << "\t\t" << child.ToString(context).string_view();
                if (child.IsObject()) {
                    std::cout << " ref_count:" << child.object().ref_count();
                }
                std::cout << std::endl;
                });

            std::cout << std::endl;

            ++it;
        }
    }

private:
	intrusive_list<Object> object_list_; ///< 对象链表，存储所有需要垃圾回收的对象
};

} // namespace mjs