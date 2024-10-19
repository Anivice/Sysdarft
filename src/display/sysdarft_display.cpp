#include <map>
#include <string>
#include <sysdarft_display.h>
#include <debug.h>
#include <res_packer.h>

sysdarft_display_t sysdarft_display;

// Function to convert Python object (list) to std::map<std::string, std::map<std::string, std::string>>
std::map<std::string, std::map<std::string, std::string>>
convert_input_stream(const py::list & py_list)
{
    if (py_list.empty()) {
        return {};
    }

    std::map<std::string, std::map<std::string, std::string>> cpp_map;

    const auto & outer_key = py::str(py_list[0]);  // First element (outer key)

    if (const auto & inner_list = py_list[1].cast<py::list>(); inner_list.size() == 2)
    {
        const auto & inner_key = py::str(inner_list[0]);
        const auto & inner_value = py::str(inner_list[1]);

        cpp_map[outer_key][inner_key] = inner_value;
    }

    return cpp_map;
}

void sysdarft_display_t::wait_for_service_to_stop()
{
    try {
        // Wait for the Python service loop to terminate
        AmberScreen->attr("join_service_loop")();  // Calls the Python method to join the thread
    } catch (const std::exception& e) {
        std::cerr << "Error while waiting for service loop to stop: " << e.what() << std::endl;
    }
}

// Function to load the script from memory and get the class
py::object load_class_from_script(const unsigned char *script, size_t size, const std::string &class_name)
{
    py::str py_script(reinterpret_cast<const char*>(script), size);
    py::dict globals = py::globals();
    py::exec(py_script, globals);
    py::object py_class = globals[class_name.c_str()];
    return py_class;
}

void sysdarft_display_t::initialize()
{
    auto file = get_res_file_list().at("scripts_AmberScreenEmulator.py");

    gc = py::module::import("gc");
    AmberScreen_t = load_class_from_script(file.file_content, file.file_length, "AmberScreenEmulator");

    AmberScreen  = new py::object();

    *AmberScreen = AmberScreen_t();
    // *AmberScreen = AmberScreen_t();

    // Start the emulator (calls the start_service method)
    if (AmberScreen->attr("start_service")().cast<int>() == -1) {
        throw sysdarft_error_t(sysdarft_error_t::SCREEN_SERVICE_LOOP_EXECUTION_FAILED);
    }
}

void sysdarft_display_t::cleanup()
{
    try {
        // Stop the service
        if (AmberScreen->attr("stop_service")().cast<int>() == -1) {
            std::cerr << "Service failed to stop correctly." << std::endl;
        }

        // Ensure the service thread is joined properly
        wait_for_service_to_stop();

        // Manually invoke garbage collection
        gc.attr("collect")();

        // Clear the global namespace to avoid memory leaks
        py::dict globals = py::globals();
        globals.clear();

        delete AmberScreen;
    } catch (const std::exception& e) {
        std::cerr << "Error during cleanup: " << e.what() << std::endl;
    }
}

input_stream_t sysdarft_display_t::query_input()
{
    return convert_input_stream(AmberScreen->attr("query_input_stream")());
}

void sysdarft_display_t::input_stream_pop_first_element()
{
    AmberScreen->attr("input_stream_pop_front")();
}

void sysdarft_display_t::display_char(int row, int col, char _char)
{
    AmberScreen->attr("display_char")(row, col, _char);
}

void sysdarft_display_t::join()
{
    AmberScreen->attr("join_service_loop")();
}

void sysdarft_display_t::sleep(float sec)
{
    AmberScreen->attr("sleep")(sec);
}

sysdarft_display_t::~sysdarft_display_t()
{
    // Perform cleanup and ensure the service has stopped
    cleanup();
}