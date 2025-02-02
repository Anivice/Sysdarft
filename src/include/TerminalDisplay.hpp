// TerminalDisplay.hpp
#ifndef TERMINALDISPLAY_H
#define TERMINALDISPLAY_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <SysdarftDebug.h>
#include <WorkerThread.h>

class TerminalDisplayError final: SysdarftBaseError {
public:
    explicit TerminalDisplayError(const std::string& message) :
        SysdarftBaseError("GUI Display Error: " + message) { }
};

class TerminalDisplay {
public:
    TerminalDisplay();
    ~TerminalDisplay();

    // Start the window and the main loop in a separate thread.
    void init();
    // Signal the main loop to stop and join the thread.
    void cleanup();

    // Registers a method to supply the video memory (2000 characters).
    // InstanceType must have a member method with signature:
    //    std::array<char, 2000> method();
    template <typename InstanceType>
    void register_linear_buffer_reader(InstanceType* instance,
        std::array<char, 2000> (InstanceType::*method)())
    {
        buffer_reader_ = [instance, method]() -> std::array<char, 2000> {
            return (instance->*method)();
        };
    }

    // Registers a method to handle key events.
    // InstanceType must have a member method with signature:
    //    void method(const int);
    template <typename InstanceType>
    void register_input_handler(InstanceType* instance,
        void (InstanceType::*method)(const int))
    {
        input_handler_ = [instance, method](int keyCode) {
            (instance->*method)(keyCode);
        };
    }

    std::atomic < int > cursor_x = 0;
    std::atomic < int > cursor_y = 0;
    std::atomic < bool > cursor_visible = true;

private:
    std::mutex font_decompressed_mutex_;
    std::vector < uint8_t > font_decompressed;
    WorkerThread mainLoopWorker;
    std::atomic<bool> is_inited { false };

    // Main loop running in a separate thread.
    void mainLoop(std::atomic<bool> & running_);

    // The function to call to update the display (should return 2000 characters)
    std::function<std::array<char, 2000>()> buffer_reader_;

    // The function to call when a key event is received.
    std::function<void(int)> input_handler_;
};

#endif // TERMINALDISPLAY_H
