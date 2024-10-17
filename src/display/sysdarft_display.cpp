#include <map>
#include <string>
#include <sysdarft_display.h>
#include <debug.h>

sysdarft_display_t sysdarft_display;

// Function to convert Python object (list) to std::map<std::string, std::map<std::string, std::string>>
std::map<std::string, std::map<std::string, std::string>>
convert_input_stream(const py::list & py_list)
{
    if (py_list.empty()) {
        return {};
    }

    std::map<std::string, std::map<std::string, std::string>> cpp_map;

    // Convert the Python list to C++ types
    const auto & outer_key = py::str(py_list[0]);  // First element (outer key)

    // Inner list initialization and size check in if-init-statement
    if (const auto & inner_list = py_list[1].cast<py::list>(); inner_list.size() == 2)
    {
        const auto & inner_key = py::str(inner_list[0]);
        const auto & inner_value = py::str(inner_list[1]);

        // Populate the map
        cpp_map[outer_key][inner_key] = inner_value;
    }

    return cpp_map;
}

sysdarft_display_t::sysdarft_display_t()
{
    // Import the Python module
    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("append")(CMAKE_SOURCE_DIR "/scripts");
    py::object AmberScreenEmulator = py::module_::import("AmberScreenEmulator");
    py::object AmberScreen_t = AmberScreenEmulator.attr("AmberScreenEmulator");
    AmberScreen = AmberScreen_t();

    // Start the emulator (calls the start_service method)
    if (AmberScreen.attr("start_service")().cast<int>() == -1) {
        throw sysdarft_error_t(sysdarft_error_t::SCREEN_SERVICE_LOOP_EXECUTION_FAILED);
    }
}
sysdarft_display_t::~sysdarft_display_t()
{
    // note: in C++11 destructors default to `noexcept`
    if (AmberScreen.attr("stop_service")().cast<int>() == -1) {
        // throw sysdarft_error_t(sysdarft_error_t::SCREEN_SERVICE_LOOP_FAILED_TO_STOP);
    }
}

input_stream_t sysdarft_display_t::query_input()
{
    return convert_input_stream(AmberScreen.attr("query_input_stream")());
}

void sysdarft_display_t::input_stream_pop_first_element()
{
    AmberScreen.attr("input_stream_pop_front")();
}

void sysdarft_display_t::display_char(int row, int col, char _char)
{
    AmberScreen.attr("display_char")(row, col, _char);
}

void sysdarft_display_t::join()
{
    AmberScreen.attr("join_service_loop")();
}

void sysdarft_display_t::sleep(float sec)
{
    AmberScreen.attr("sleep")(sec);
}
