#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

template<typename T>
class ConcurrentQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
    std::condition_variable empty_cond_var_;
    std::atomic<bool> shutdown_{false};

public:
    void push(T&& item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(item));
        }
        cond_var_.notify_one();
    }

    void push(const T& item) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(item);
        }
        cond_var_.notify_one();
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_var_.wait(lock, [this]() {
            return !queue_.empty() || shutdown_.load();
        });

        if (queue_.empty() && shutdown_.load()) {
            return false;
        }

        item = std::move(queue_.front());
        queue_.pop();

        if (queue_.empty()) {
            empty_cond_var_.notify_all();
        }

        return true;
    }

    void wait_empty() {
        std::unique_lock<std::mutex> lock(mutex_);
        empty_cond_var_.wait(lock, [this]() {
            return queue_.empty();
        });
    }

    void shutdown() {
        shutdown_.store(true);
        cond_var_.notify_all();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};
