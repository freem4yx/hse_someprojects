#include "parallel_processor.h"
#include <thread>
#include <chrono>


ParallelImageProcessor::ParallelImageProcessor(int num_workers) {

    if (num_workers <= 0) {
        num_workers = std::thread::hardware_concurrency();
    }

    std::cout << "Creating " << num_workers << " worker threads" << std::endl;

    for (int i = 0; i < num_workers; ++i) {
        consumers_.emplace_back([this, i]() {
            consumer_loop(i);
        });
    }
}

ParallelImageProcessor::~ParallelImageProcessor() {
    stop();
}

void ParallelImageProcessor::stop() {
    task_queue_.shutdown();
    result_queue_.shutdown();

    for (auto& thread : consumers_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ParallelImageProcessor::consumer_loop(int consumer_id) {
    ImageTask task;

    while (task_queue_.pop(task)) {
        try {

            task.processor(task.start_row, task.end_row);

            TaskResult result;
            result.task_id = task.task_id;
            result.success = true;
            result_queue_.push(std::move(result));

            int completed = tasks_completed_.fetch_add(1) + 1;

        } catch (const std::exception& e) {
            TaskResult result;
            result.task_id = task.task_id;
            result.success = false;
            result.error_message = e.what();
            result_queue_.push(std::move(result));
        }
    }
}

void ParallelImageProcessor::wait_completion() {
    while (tasks_completed_.load() < total_tasks_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
