#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <exception>
#include <utility>

template<class T>
class BufferedChannel {
public:
    explicit BufferedChannel(int size) : capacity(size > 0 ? size : 0), closed(false) {}

    void Send(T value) {
        std::unique_lock<std::mutex> lock(mutex);

        not_full.wait(lock, [this]() {
            return queue.size() < capacity || closed;
            });

        if (closed) {
            throw std::runtime_error("Cannot send to closed channel");
        }

        queue.push(std::move(value));
        not_empty.notify_one();
    }

    std::pair<T, bool> Recv() {
        std::unique_lock<std::mutex> lock(mutex);

        not_empty.wait(lock, [this]() {
            return !queue.empty() || (closed && queue.empty());
            });

        if (queue.empty() && closed) {
            return std::make_pair(T(), false);
        }

        T value = std::move(queue.front());
        queue.pop();

        not_full.notify_one();

        return std::make_pair(std::move(value), true);
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;

        not_full.notify_all();
        not_empty.notify_all();
    }

private:
    std::queue<T> queue;
    size_t capacity;
    bool closed;

    std::mutex mutex;
    std::condition_variable not_full;
    std::condition_variable not_empty;
};

#endif // BUFFERED_CHANNEL_H_