#pragma once

#include <deque>

#include <mjs/job.h>

namespace mjs {

class JobQueue : public std::deque<Job> {
public:
    using Base = std::deque<Job>;
    using Base::Base;

    void ForEachChild(Context* context, intrusive_list<Object>* list, void(*callback)(Context* context, intrusive_list<Object>* list, const Value& child)) {
        for (auto& job : *this) {
            job.ForEachChild(context, list, callback);
        }
    }
};

} // namespace mjs