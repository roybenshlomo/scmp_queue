#include <iostream>
#include <thread>
#include "inc/scmp_queue.h"
#ifdef TRIVIAL_QUEUE
#include <queue>
#include <mutex>
#include <condition_variable>
template <class T>
class blocking_queue
{
public:
    blocking_queue()
    {
    }
    void push(const T & item)
    {
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_queue.push(item);
        }

        m_cv.notify_all();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_cv.wait(lock, [&]() { return !m_queue.empty(); });

        auto value = m_queue.front();
        m_queue.pop();
        return value;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_lock;
    std::condition_variable m_cv;
};

blocking_queue<int> queue;
#else
scmp_queue<int> queue;
#endif

int n_threads = 1000;
int n_items = 1000;

void producer_thread()
{
    for (int i = 0; i < n_items; i++) {
        queue.push(i);
    }
}

void consumer_queue()
{
    uint64_t counter = 0;
    while (counter < (n_threads * n_items)) {
        queue.pop();
        counter++;
    }

    std::cout << "Popped " << counter << " items from the consumer queue" << std::endl;
}

int main(int argc, char * argv[])
{
    std::thread consumer(consumer_queue);
    std::list<std::thread> producers;

    for (int i = 0; i < n_threads; i++) {
        producers.push_front(std::move(std::thread(producer_thread)));
    }

    for (auto & i : producers) {
        i.join();
    }

    consumer.join();

    return 0;
}
