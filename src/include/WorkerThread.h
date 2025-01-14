#ifndef WORKER_H
#define WORKER_H

#include <functional>
#include <any>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <SysdarftDebug.h>
#include <SysdarftMessageMap.h>

class WorkerThread {
private:
    // Adjust the signature to match the expected lambda signature
    std::function<void(std::atomic<bool>&, const std::vector<std::any>&)> method_;
    std::atomic<bool> running = false;
    std::thread WorkerThreadInstance;

public:
    template <typename InstanceType, typename... Args>
    WorkerThread(
        InstanceType* instance,
        void (InstanceType::*method)(std::atomic<bool>&, Args...)
    )
    {
        // Bind the method to the instance
        auto bound_function = [instance, method, this](Args... args) -> void {
            (instance->*method)(running, args...);
        };

        // Store a lambda that matches the signature of method_
        method_ = [bound_function](
            std::atomic<bool>& /* running */,
            const std::vector<std::any>& args) -> void
        {
            // Invoke the bound function with the provided arguments
            // The return value (std::any) is ignored since method_ expects void
            invoke_with_any<decltype(bound_function), Args...>(bound_function, args);
        };
    }

    // Delete copy constructor and copy assignment operator
    WorkerThread(const WorkerThread&) = delete;
    WorkerThread& operator=(const WorkerThread&) = delete;

    // Move constructor and move assignment operator can be provided if needed
    WorkerThread(WorkerThread&& other) noexcept
        : method_(std::move(other.method_)),
          running(other.running.load()),
          WorkerThreadInstance(std::move(other.WorkerThreadInstance))
    {
        other.stop();
    }

    WorkerThread& operator=(WorkerThread&& other) noexcept
    {
        if (this != &other)
        {
            this->stop();
            other.stop();

            method_ = std::move(other.method_);
            running = other.running.load();
            WorkerThreadInstance = std::move(other.WorkerThreadInstance);
        }

        return *this;
    }

    template <typename... Args>
    void start(Args&... args)
    {
        if (running) {
            return;
        }

        log("[Worker] Starting worker thread...\n");
        running = true;

        // Prepare the arguments as a vector of std::any
        std::vector<std::any> any_args = { args... };

        // Start the thread with the correct arguments
        WorkerThreadInstance = std::thread(method_, std::ref(running), any_args);
        log("[Worker] Worker thread detached...\n");
    }

    void stop()
    {
        if (!running) {
            return;
        }

        log("[Worker] Stopping worker thread...\n");
        running = false;
        if (WorkerThreadInstance.joinable()) {
            WorkerThreadInstance.join();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        log("[Worker] Worker thread stopped...\n");
    }

    ~WorkerThread()
    {
        if (running)
        {
            log("[Worker] Stopping worker thread automatically...\n");
            stop();
        }
    }
};

#endif // WORKER_H
