/* WorkerThread.h
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
    std::string workerThreadName;

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

#ifdef __DEBUG__
        workerThreadName = debug::demangle(typeid(InstanceType).name()) + "::" + debug::demangle(typeid(method).name());
#endif
    }

    // Delete copy constructor and copy assignment operator
    WorkerThread(const WorkerThread&) = delete;
    WorkerThread& operator=(const WorkerThread&) = delete;
    WorkerThread(WorkerThread&& other) = delete;
    WorkerThread& operator=(WorkerThread&& other) = delete;

    template <typename... Args>
    void start(Args&... args)
    {
        if (running) {
            return;
        }

        log("[Worker] Starting worker thread ", workerThreadName, "...\n");
        running = true;

        // Prepare the arguments as a vector of std::any
        std::vector<std::any> any_args = { args... };

        // Start the thread with the correct arguments
        WorkerThreadInstance = std::thread(method_, std::ref(running), any_args);
        log("[Worker] Worker thread ", workerThreadName, " detached...\n");
    }

    void stop()
    {
        if (!running) {
            return;
        }

        log("[Worker] Stopping worker thread ", workerThreadName, "...\n");
        running = false;
        if (WorkerThreadInstance.joinable()) {
            WorkerThreadInstance.join();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        log("[Worker] Worker thread ", workerThreadName, " stopped...\n");
    }

    ~WorkerThread()
    {
        if (running)
        {
            log("[Worker] Stopping worker thread ", workerThreadName, " automatically...\n");
            stop();
        }
    }
};

#endif // WORKER_H
