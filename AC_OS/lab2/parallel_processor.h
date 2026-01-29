#pragma once
#include "concurrent_queue.h"
#include <vector>
#include <thread>
#include <functional>
#include <atomic>
#include <memory>
#include <iostream>

struct ImageTask {
    int start_row;
    int end_row;
    int task_id;
    std::function<void(int, int)> processor;
};

struct TaskResult {
    int task_id;
    bool success;
    std::string error_message;
};

class ParallelImageProcessor {
private:
    ConcurrentQueue<ImageTask> task_queue_;
    ConcurrentQueue<TaskResult> result_queue_;
    std::vector<std::thread> consumers_;
    std::atomic<int> tasks_completed_{0};
    std::atomic<int> total_tasks_{0};
    std::atomic<bool> processing_{false};

public:
    ParallelImageProcessor(int num_workers = 0);
    ~ParallelImageProcessor();

    template<typename TaskGenerator>
    void process_tasks(TaskGenerator&& generator, int total_tasks) {
        processing_.store(true);
        tasks_completed_.store(0);
        total_tasks_.store(total_tasks);

        for (int i = 0; i < total_tasks; ++i) {
            ImageTask task = generator(i);
            task_queue_.push(std::move(task));
        }

        wait_completion();
    }

    void stop();


private:
    void consumer_loop(int consumer_id);
    void wait_completion();
};
