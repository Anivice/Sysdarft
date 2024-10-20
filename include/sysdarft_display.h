/**
 * @file amber_phosphor_screen.h
 * @brief This file offers the class object (extern) sysdarft_display for emulating Amber Phosphor Screens
 */

#ifndef SYSDARFT_AMBER_PHOSPHOR_SCREEN_H
#define SYSDARFT_AMBER_PHOSPHOR_SCREEN_H

#include <pybind11/embed.h> // pybind11's embedding module
#include <map>
#include <string>
#include <res_packer.h>
#include <chrono>  // for std::chrono::seconds

namespace py = pybind11;

typedef std::map < std::string /* stream type (Control/Input) */,
                   std::map < std::string, std::string > > input_stream_t;

extern class sysdarft_display_t {
private:
    py::object * AmberScreen = nullptr;
    // py::object AmberScreen;         // The instantiated AmberScreen object from Python
    py::scoped_interpreter guard;   // Manages the Python interpreter lifecycle
    py::object AmberScreen_t;       // The AmberScreenEmulator class object
    py::module gc;                  // Garbage collection module

    bool did_i_cleanup = false;

    // Initialize the AmberScreenEmulator instance
    void initialize();

    // Cleanup function to safely stop the service and collect garbage
    void cleanup();

public:
    sysdarft_display_t() {
        // Initialize the resource filesystem and the Amber Screen
        initialize_resource_filesystem();
        initialize();
    }

    ~sysdarft_display_t();

    // Deleted copy assignment to avoid accidental copying of the display object
    sysdarft_display_t& operator=(const sysdarft_display_t&) = delete;

    // Query input stream from the AmberScreen emulator
    input_stream_t query_input();

    // Pop the first element from the input stream
    void input_stream_pop_first_element();

    // Display a character on the screen at the specified row and column
    void display_char(int row, int col, char _char);

    // Join the service loop, ensuring that it is safely terminated
    void join();

    // Put the emulator to sleep for a specified time (in seconds)
    void sleep(float seconds);

private:
    // Wait for the service thread to complete before cleanup
    void wait_for_service_to_stop();
} sysdarft_display;

#endif // SYSDARFT_AMBER_PHOSPHOR_SCREEN_H
