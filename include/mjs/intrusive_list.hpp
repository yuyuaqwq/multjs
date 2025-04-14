// intrusive_list.hpp
#pragma once

#include <utility>
#include <type_traits>

template <typename T>
class intrusive_list {
public:
    class node {
        friend class intrusive_list;
        
        node* prev = nullptr;
        node* next = nullptr;
        
    public:
        node() = default;
        virtual ~node() { unlink(); }
        
        node(const node&) = delete;
        node& operator=(const node&) = delete;
        
        void unlink() {
            if (prev) prev->next = next;
            if (next) next->prev = prev;
            prev = next = nullptr;
        }
    };

private:
    node head_;

    static T* as_item(node* n) {
        return static_cast<T*>(n);
    }

public:
    intrusive_list() {
        head_.prev = head_.next = &head_;
    }

    ~intrusive_list() {
        clear();
    }

    // 迭代器
    class iterator {
        friend class intrusive_list;
        node* current_;
        
    public:
        explicit iterator(node* n) : current_(n) {}
        
        T& operator*() const { return *as_item(current_); }
        T* operator->() const { return as_item(current_); }
        
        iterator& operator++() {
            current_ = current_->next;
            return *this;
        }
        
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        bool operator==(const iterator& other) const {
            return current_ == other.current_;
        }
        
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

    iterator begin() { return iterator(head_.next); }
    iterator end() { return iterator(&head_); }

    // 容量
    bool empty() const { return head_.next == &head_; }

    // 元素访问
    T& front() { return *as_item(head_.next); }
    T& back() { return *as_item(head_.prev); }

    // 修改器
    void push_front(T& item) {
        insert(begin(), item);
    }

    void push_back(T& item) {
        insert(end(), item);
    }

    void insert(iterator pos, T& item) {
        node* new_node = &item;
        node* next_node = pos.current_;
        node* prev_node = next_node->prev;
        
        new_node->next = next_node;
        new_node->prev = prev_node;
        prev_node->next = new_node;
        next_node->prev = new_node;
    }

    void erase(iterator pos) {
        pos.current_->unlink();
    }

    void clear() {
        while (!empty()) {
            front().unlink();
        }
    }
};