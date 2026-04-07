#ifndef THREADSAFE_QUEUE_HPP
#define THREADSAFE_QUEUE_HPP

#include <queue>
#include <mutex>

template <typename T>
class ThreadSafe_queue{
    public:

        /*
         * Summary:
         * Initialize queue with maximum allowed size.
         *
         * Parameters:
         * - mx_size (int): maximum number of elements stored in queue.
         *
         * Returns:
         * - No return value.
         */
        explicit ThreadSafe_queue(int mx_size): max_size(mx_size){}

        /*
         * Summary:
         * Push element into queue and discard oldest if full.
         *
         * Parameters:
         * - item (T): element to insert into queue.
         *
         * Returns:
         * - No return value.
         */
        void push(T item);

        /*
         * Summary:
         * Pop and return oldest element from queue.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - T: oldest element stored in queue.
         */
        T pop();

        /*
         * Summary:
         * Return current queue size.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - int: number of elements in queue.
         */
        int size();

        /*
         * Summary:
         * Check whether queue is empty.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - bool: true if queue is empty.
         */
        bool empty();
    
    private:
        std::queue<T> queue;
        std::mutex mutex;
        int max_size;
};

template <typename T>
void ThreadSafe_queue<T>::push(T item){
    std::lock_guard<std::mutex> lock(mutex);

    if (queue.size() >= max_size) {
        queue.pop();
    }
    queue.push(item);
}

template <typename T>
T ThreadSafe_queue<T>::pop(){
    std::lock_guard<std::mutex> lock(mutex);

    T item = queue.front();
    queue.pop();
    return item;
}

template <typename T>
int ThreadSafe_queue<T>::size(){
    std::lock_guard<std::mutex> lock(mutex);
    return queue.size();
}

template <typename T>
bool ThreadSafe_queue<T>::empty(){
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
}
#endif

