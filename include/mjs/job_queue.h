/**
 * @file job_queue.h
 * @brief JavaScript 作业队列系统定义
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 *
 * 本文件定义了 JavaScript 引擎中的作业队列系统，包括作业队列类，
 * 用于管理待执行的作业任务和垃圾回收遍历。
 */

#pragma once

#include <deque>

#include <mjs/job.h>

namespace mjs {

/**
 * @class JobQueue
 * @brief 作业队列类
 *
 * 继承自 std::deque<Job>，提供作业队列功能，支持垃圾回收遍历所有作业。
 * 用于管理待执行的 JavaScript 作业任务。
 *
 * @see std::deque 双端队列基类
 * @see Job 作业类
 */
class JobQueue : public std::deque<Job> {
public:
    using Base = std::deque<Job>;
    using Base::Base;

    /**
     * @brief 垃圾回收遍历子对象
     * @param context 执行上下文指针
     * @param list 对象链表
     * @param callback 回调函数，用于标记子对象
     */
    void ForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
        for (auto& job : *this) {
            job.ForEachChild(context, list, callback);
        }
    }
};

} // namespace mjs