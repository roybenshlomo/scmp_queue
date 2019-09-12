#pragma once
#include <list>
#include <atomic>
#include <chrono>
#include <thread>
#include "lockless_stack.h"

template <class T>
class scmp_queue
{
    public:
        /*
         * constructor
         */
        scmp_queue(std::uint64_t blocking_interval_us = 5000);

        /*
         * destructor
         */
        ~scmp_queue();

        /*
         * push an element to the queue - this is called from multiple threads
         */
        void push(const T &item);

        /*
         * pop an item from the queue - this is called from a single thread
         */
        T pop();
    private:
        std::atomic<lockless_stack<T>*> m_producers_stack;
        std::atomic<lockless_stack<T>*> m_consumer_stack;
        std::list<T> m_consumer_list;
        std::uint64_t m_blocking_interval_us;
};

template <class T>
scmp_queue<T>::scmp_queue(std::uint64_t blocking_interval_us /* = 5000 */)
{
    // create two lockless stacks
    m_producers_stack = new lockless_stack<T>();
    m_consumer_stack = new lockless_stack<T>();

    m_blocking_interval_us = blocking_interval_us;
}

template<class T>
scmp_queue<T>::~scmp_queue()
{
    delete m_producers_stack;
    delete m_consumer_stack;
}

template <class T>
void scmp_queue<T>::push(const T &item)
{
    (*m_producers_stack).push(item);
}

/*
 * This method is by definition called from a single thread, therefore a lot of the operations
 * here assume that they don't require locking and can happen without being preempted
 */
template <class T>
T scmp_queue<T>::pop()
{
    /*
     * If there is data available in the consumer queue, we just pop that queue and return
     * the value as-is. If there isn't anything in the consumer queue, we flip the stacks
     */
    if (m_consumer_list.empty()) {
        do
        {
            // swap the stacks and review the stack filled by the producer threads
            m_consumer_stack = m_producers_stack.exchange(m_consumer_stack);
            
            // check if the stack is empty, if so we need to blocking-wait for it to be filled with something
            if ((*m_consumer_stack).empty()) {
                std::this_thread::sleep_for(std::chrono::microseconds(m_blocking_interval_us));
            } else {
                while (!(*m_consumer_stack).empty()) {
                    m_consumer_list.push_front((*m_consumer_stack).pop());
                }
                break;
            }
        } while (true);
    }

    // take the first element in the queue
    T queue_front = m_consumer_list.front();
    // pop it from the queue
    m_consumer_list.pop_front();
    return queue_front;
}
