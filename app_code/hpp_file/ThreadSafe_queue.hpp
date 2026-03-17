#ifndef THREADSAFE_QUEUE_HPP
#define THREADSAFE_QUEUE_HPP
#endif

#include <queue>
#include <mutex>

template <typename T>
class ThreadSafe_queue{
    public:
        explicit ThreadSafe_queue(int mx_size);
        void push(T item);
        T pop();
        int size();
        bool empty();
    
    private:
        std::queue<T> queue;
        std::mutex mutex;
        int max_size;
};