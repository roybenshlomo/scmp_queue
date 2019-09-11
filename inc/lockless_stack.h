#pragma once
#include <atomic>

template <class T>
class lockless_stack
{
    struct node
    {
        node(const T& data)
        {
            m_data = data;
            m_next = nullptr;
        }
        T m_data;
        node * m_next;
    };

    public:
        /*
         * constructor
         */
        lockless_stack();

        /*
         * destructor
         */
        ~lockless_stack();

        /*
         * push an element into the stack
         */
        void push(const T&);

        /*
         * pop an element from the stack
         */
        T&& pop();

        /*
         * check if the stack is empty (not thread safe)
         */
        bool empty() const
        {
            return (m_head == nullptr);
        }
    private:
        std::atomic<lockless_stack::node *> m_head;
};

template <class T>
lockless_stack<T>::lockless_stack()
{

}

template <class T>
lockless_stack<T>::~lockless_stack()
{
    // pop all items from the queue to release the memory allocations
    while (!empty()) {
        pop();
    }
}

template <class T>
void lockless_stack<T>::push(const T & data)
{
    lockless_stack::node * new_node = new lockless_stack::node(data);
    
    do
    {
        new_node->m_next = m_head.load(std::memory_order_relaxed);
    } while (!std::atomic_compare_exchange_weak_explicit(&m_head, 
                                                         &new_node->m_next,
                                                         new_node,
                                                         std::memory_order_release, 
                                                         std::memory_order_relaxed));
}

template <class T>
T&& lockless_stack<T>::pop()
{
    lockless_stack::node * current_head = nullptr;

    do
    {
        current_head = m_head.load(std::memory_order_relaxed);
    } while (!std::atomic_compare_exchange_weak_explicit(&m_head, 
                                                         &current_head,
                                                         current_head->m_next,
                                                         std::memory_order_release, 
                                                         std::memory_order_relaxed));
    
    T return_value = current_head->m_data;
    delete current_head;
    return std::move(return_value);
}
