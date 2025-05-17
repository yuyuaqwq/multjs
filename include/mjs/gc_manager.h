#pragma once

#include <iostream>

#include <mjs/noncopyable.h>
#include <mjs/object.h>

namespace mjs {

class GCManager : public noncopyable {
public:
    GCManager() {}

    void GC(Context* context) {
        intrusive_list<Object> tmp_list;

        auto it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(!cur.gc_mark());

            cur.GCForEachChild(context, &tmp_list, [](Context* context, intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                assert(obj.ref_count() > 0);
                obj.WeakDereference();
                // ����Ӷ�������ü�����Ϊ0��˵�������ǰ�����ͷţ��Ӷ���Ҳ�ᱻ�ͷ�
                // �����Ӷ����Ѿ�����ǹ�����ӵ���ʱ������Ϊ�Ӷ���Ҳ��Ҫ�������Ӷ���
                if (obj.ref_count() == 0 && obj.gc_mark()) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;

            // ���ɨ��Ķ��󣬱��һ��
            cur.set_gc_mark(true);
            if (cur.ref_count() == 0) {
                cur.unlink();
                tmp_list.push_back(cur);
            }
        }

        // �����￴object_list_��û�б����յľͿ�����©�����
        // ��һ��Value::IsObject��û����Ӷ�Ӧ������

        it = object_list_.begin();
        while (it != object_list_.end()) {
            Object& cur = *it;
            assert(cur.ref_count() > 0);

            // ʣ�µĶ��ǲ��ɻ��յĶ��������һ�±��
            cur.set_gc_mark(false);

            cur.GCForEachChild(context, &object_list_, [](Context* context, intrusive_list<Object>* list, const Value& child) {
                if (!child.IsObject()) return;
                auto& obj = child.object();
                // �����ɻ��յĶ������õ��Ӷ��󣬵�ȻҲ���ܻ��գ��һ�������
                obj.Reference();
                if (obj.ref_count() == 1) {
                    obj.unlink();
                    list->push_back(obj);
                }
            });

            ++it;
        }

        // ������һ��ֻ�ָ���ʣ�µ�object_list_���Ӷ�������ü���
        // ���ͷŶ�����Ӷ�������ü���ҲҪ�ӻ�ȥ
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


        // ʣ�µ�tmp_list�еĽڵ���������������ͷ�
        // ������Ϊ��Ҫ�ֶ���ʼ�ͷŶ��󣬵��������ֻ�����Value�Զ�����
        // ����û��У�����ü������������ü����ͷ���Щ����
        while (!tmp_list.empty()) {
            Object& obj = tmp_list.front();
            // assert(obj.ref_count() == 0);
            assert(obj.gc_mark());
            // obj.WeakDereference();
            delete &obj;
        }
    }

	void AddObject(Object* object) {
		object_list_.push_back(*object);
	}

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
	intrusive_list<Object> object_list_;
};

} // namespace mjs
