#ifndef WORKER_H
#define WORKER_H

#include <functional>
#include <any>
#include <thread>
#include <atomic>
#include <vector>
#include <debug.h>
#include <chrono>
#include <msg_map.h>

class worker_thread {
private:
    // Adjust the signature to match the expected lambda signature
    std::function<void(std::atomic<bool>&, std::atomic<bool>&, const std::vector<std::any>&)> method_;
    std::atomic<bool> running = false;
    std::atomic<bool> stopped = true;
    std::atomic<bool> stopped_before = true;
public:
    template <typename InstanceType, typename... Args>
    worker_thread(
        InstanceType* instance,
        void (InstanceType::*method)(std::atomic<bool>&, std::atomic<bool>&, Args...)
    )
    {
        // Bind the method to the instance
        auto bound_function = [instance, method, this](Args... args) -> void {
            (instance->*method)(running, stopped, args...);
        };

        // Store a lambda that matches the signature of method_
        method_ = [bound_function](
            std::atomic<bool>& /* running */,
            std::atomic<bool>& /* stopped */,
            const std::vector<std::any>& args) -> void
        {
            // Invoke the bound function with the provided arguments
            // The return value (std::any) is ignored since method_ expects void
            invoke_with_any<decltype(bound_function), Args...>(bound_function, args);
        };
    }

    template <typename... Args>
    void start(Args&... args)
    {
        debug::log("[Worker] Starting worker thread...\n");
        running = true;
        stopped = false;
        stopped_before = false;

        // Prepare the arguments as a vector of std::any
        std::vector<std::any> any_args = { args... };

        // Start the thread with the correct arguments
        std::thread(method_, std::ref(running), std::ref(stopped), any_args).detach();
        debug::log("[Worker] Worker thread detached...\n");
    }

    void stop()
    {
        debug::log("[Worker] Stopping worker thread...\n");
        running = false;
        while (!stopped) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        debug::log("[Worker] Worker thread stopped...\n");
        stopped_before = true;
    }

    ~worker_thread()
    {
        if (!stopped_before) { // manual stop didn't invoked
            debug::log("[Worker] Stopping worker thread automatically...\n");
            stop();
        }
    }
};

#endif // WORKER_H
