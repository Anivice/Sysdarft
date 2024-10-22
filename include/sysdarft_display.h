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

namespace py = pybind11;

typedef std::map < std::string /* stream type (Control/Input) */,
                   std::map < std::string, std::string > > input_stream_t;

extern class sysdarft_display_t
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
    void input_stream_pop_first_element();
    void display_char(int row, int col, char _char);
    void join_service_loop();
    void sleep(float seconds);
    std::string get_char_at_pos(int x, int y);
    std::vector < std::pair< std::string, int> > get_current_config();
private:
    // Wait for the service thread to complete before cleanup
    void wait_for_service_to_stop();
} sysdarft_display;

#endif // SYSDARFT_AMBER_PHOSPHOR_SCREEN_H
