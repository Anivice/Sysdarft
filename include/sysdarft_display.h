/**
 * @file amber_phosphor_screen.h
 * @brief This file offers the class object (extern) sysdarft_display for emulating Amber Phosphor Screens
 */

#ifndef SYSDARFT_AMBER_PHOSPHOR_SCREEN_H
#define SYSDARFT_AMBER_PHOSPHOR_SCREEN_H

#include <pybind11/embed.h> // pybind11's embedding module
#include <map>
#include <string>
namespace py = pybind11;

typedef std::map < std::string /* stream type (Control/Input) */,
    std::map < std::string, std::string > > input_stream_t;

extern
class __attribute__((visibility("hidden"))) sysdarft_display_t
{
private:
    py::scoped_interpreter guard{};
    py::object AmberScreen{};

public:
    sysdarft_display_t();
    ~sysdarft_display_t();

    input_stream_t query_input();
    void input_stream_pop_first_element();
    void display_char(int, int, char);
    void join();
    void sleep(float);

} sysdarft_display;

#endif // SYSDARFT_AMBER_PHOSPHOR_SCREEN_H
