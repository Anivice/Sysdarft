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
    void display_char(int row, int col, char _char);
    void unblocked_sleep(unsigned int);

    sysdarft_display_t& operator=(const sysdarft_display_t&) = delete;
};

#endif // SYSDARFT_AMBER_PHOSPHOR_SCREEN_H
