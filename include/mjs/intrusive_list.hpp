#pragma once

#include <utility>
#include <type_traits>
#include <cassert>

template <typename T>
class intrusive_list {
public:
    class node {
        friend class intrusive_list;

        node* prev = nullptr;
        node* next = nullptr;

    public:
        node() = default;
         virtual ~node() { /*unlink();*/ }

        node(const node&) = delete;
        node& operator=(const node&) = delete;

        void unlink() {
            if (is_linked()) {
                if (prev) prev->next = next;
                if (next) next->prev = prev;
                prev = next = nullptr;
            }
        }

        bool is_linked() const {
            return prev != nullptr && next != nullptr;
        }
    };

private:
    node head_;

    static T* as_item(node* n) {
        return static_cast<T*>(n);
    }

    static const T* as_item(const node* n) {
        return static_cast<const T*>(n);
    }

public:
    intrusive_list() {
        head_.prev = head_.next = &head_;
    }

    ~intrusive_list() {
        clear();
    }

    // -----------------------
    //       Iterator
    // -----------------------
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

    class const_iterator {
        friend class intrusive_list;
        const node* current_;

    public:
        explicit const_iterator(const node* n) : current_(n) {}

        const T& operator*() const { return *as_item(current_); }
        const T* operator->() const { return as_item(current_); }

        const_iterator& operator++() {
            current_ = current_->next;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const {
            return current_ == other.current_;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }
    };

    // -----------------------
    //     Basic Access
    // -----------------------
    iterator begin() { return iterator(head_.next); }
    iterator end() { return iterator(&head_); }

    const_iterator begin() const { return const_iterator(head_.next); }
    const_iterator end() const { return const_iterator(&head_); }

    const_iterator cbegin() const { return const_iterator(head_.next); }
    const_iterator cend() const { return const_iterator(&head_); }

    bool empty() const { return head_.next == &head_; }

    T& front() { return *as_item(head_.next); }
    T& back() { return *as_item(head_.prev); }

    const T& front() const { return *as_item(head_.next); }
    const T& back() const { return *as_item(head_.prev); }

    // -----------------------
    //     Modifiers
    // -----------------------
    void push_front(T& item) {
        insert(begin(), item);
    }

    void push_back(T& item) {
        insert(end(), item);
    }

    void pop_front() {
        if (!empty()) {
            front().unlink();
        }
    }

    void pop_back() {
        if (!empty()) {
            back().unlink();
        }
    }

    void insert(iterator pos, T& item) {
        node* new_node = &item;
        assert(!new_node->is_linked() && "Element already in list!");

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

    // -----------------------
    //      Utilities
    // -----------------------
    bool contains(const T& item) const {
        const node* n = static_cast<const node*>(&item);
        return n->is_linked();
    }

    // 可选：计算 size（O(n)，不推荐频繁使用）
    size_t size() const {
        size_t count = 0;
        for (auto it = begin(); it != end(); ++it)
            ++count;
        return count;
    }
};
