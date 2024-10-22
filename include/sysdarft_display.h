/**
 * @file amber_phosphor_screen.h
 * @brief This file offers the class object (extern) sysdarft_display for emulating Amber Phosphor Screens
 */

#ifndef SYSDARFT_AMBER_PHOSPHOR_SCREEN_H
#define SYSDARFT_AMBER_PHOSPHOR_SCREEN_H

#include <pybind11/embed.h> // pybind11's embedding module
#include <map>
#include <string>
#include <chrono>  // for std::chrono::seconds
#include <atomic>
#include <future>

namespace py = pybind11;

typedef std::map < std::string /* stream type (Control/Input) */,
                   std::map < std::string, std::string > > input_stream_t;

class sysdarft_display_t
{
private:
    py::object *            AmberScreen     = nullptr;      // The instantiated AmberScreen object from Python
    py::scoped_interpreter* guard           = nullptr;      // Manages the Python interpreter lifecycle
    bool                    did_i_cleanup   = false;

public:
    void initialize();
    void cleanup();
    sysdarft_display_t& operator=(const sysdarft_display_t&) = delete;
    input_stream_t query_input();
    void display_char(int row, int col, char _char);
    void join_service_loop();
    std::vector < std::pair< std::string, int> > get_current_config();

private:
    // Wait for the service thread to complete before cleanup
    void wait_for_service_to_stop();
};

class sysdarft_gpu_t
{
public:
    class SafeVideoMemory
    {
    public:
        using VideoMemoryType = std::vector<std::vector<char>>;

        // Store a new value for video memory
        void store(const VideoMemoryType& new_memory) {
            std::lock_guard<std::mutex> lock(mutex_);
            video_memory_ = new_memory;
        }

        // Load the current value of video memory
        VideoMemoryType load() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return video_memory_;
        }

    private:
        VideoMemoryType video_memory_; // The actual video memory storage
        mutable std::mutex mutex_;     // Mutex for synchronizing access
    } video_memory;

private:
    sysdarft_display_t gui_display;
    int row = 0;
    int col = 0;

    std::promise < void >   gpuExitSignal;
    std::future < void >    gpuFutureObj;
    std::atomic < bool >    should_gpu_service_be_running = false;
    void start_sysdarft_gpu_service_loop();
    void stop_sysdarft_gpu_service_loop();

    void refresh_screen();
public:
    void initialize();
    void sleep_without_blocking(unsigned long int);
};

#endif // SYSDARFT_AMBER_PHOSPHOR_SCREEN_H
