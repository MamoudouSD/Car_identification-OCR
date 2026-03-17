#include "ThreadSafe_queue.hpp"

ThreadSafe_queue::ThreadSafe_queue(int mx_size){
    max_size = mx_size;
}

void ThreadSafe_queue::push(T item){
    std::lock_guard<std::mutex> lock(mutex);

    if (queue.size() >= max_size) {
        queue.pop();
    }
    queue.push(item);
}
T ThreadSafe_queue::pop(){
    std::lock_guard<std::mutex> lock(mutex);

    T item = queue.front();
    queue.pop();
    return item;
}
int ThreadSafe_queue::size(){
    std::lock_guard<std::mutex> lock(mutex);
    return queue.size();
}

bool ThreadSafe_queue::empty(){
    std::lock_guard<std::mutex> lock(mutex);
    return queue.empty();
}
