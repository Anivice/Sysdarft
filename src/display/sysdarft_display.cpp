#include <map>
#include <string>
#include <sysdarft_display.h>
#include <debug.h>
#include <res_packer.h>
#include <tools.h>
#include <chrono>
#include <res_packer.h>
#include <event_vector.h>

// Function to convert a Python list of lists to a std::vector<std::pair<std::string, int>>
std::vector<std::pair<std::string, int>> convert_list(const py::list & pyList)
{
    std::vector<std::pair<std::string, int>> result;
    for (auto item : pyList) {
        // Cast each item to py::list to access by index
        py::list inner_list = py::cast<py::list>(item);

        // Access elements within the inner list
        std::string key = py::cast<std::string>(inner_list[0]);
        int value = py::cast<int>(inner_list[1]);

        result.emplace_back(key, value);
    }
    return result;
}

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
    initialize_resource_filesystem();
    initialize_interruption_handler();

    guard = new pybind11::scoped_interpreter();
    auto file = get_res_file_list().at("scripts_AmberScreenEmulator.py");
    auto AmberScreen_t = load_class_from_script(file.file_content, file.file_length, "AmberScreenEmulator");
    auto libxxd = find_containing_substring(list_shared_libraries(), "libxxd_binary_content.so");
    if (libxxd.empty()) {
        throw sysdarft_error_t(sysdarft_error_t::CANNOT_OBTAIN_DYNAMIC_LIBRARIES);
    }
    auto xxd_lib_path = libxxd.at(0);
    std::regex libPattern("libxxd_binary_content\\.so");
    std::string event_lib_path = std::regex_replace(xxd_lib_path, libPattern, "libsysdarft_event_vec.so");

    AmberScreen = new py::object(
        AmberScreen_t(
            xxd_lib_path.c_str(),
            event_lib_path.c_str(),
            getpid(),
            "Sysdarft Emulator Screen"
        )
    );

    // Start the emulator (calls the start_service method)
    if (AmberScreen->attr("start_service")().cast<int>() == -1) {
        throw sysdarft_error_t(sysdarft_error_t::SCREEN_SERVICE_LOOP_EXECUTION_FAILED);
    }

    py::gil_scoped_release release;
}

void sysdarft_display_t::cleanup()
{
    if (did_i_cleanup || !guard || !AmberScreen) {
        return;
    }

    try { {
            py::gil_scoped_acquire acquire;

            // Stop the service
            if (AmberScreen->attr("stop_service")().cast<int>() == -1) {
                std::cerr << "Service failed to stop correctly." << std::endl;
            }

            delete AmberScreen;
            AmberScreen = nullptr;
        }

        delete guard;
        guard = nullptr;

        did_i_cleanup = true;
    } catch (const std::exception& e) {
        std::cerr << "Error during cleanup: " << e.what() << std::endl;
    }
}

void sysdarft_display_t::display_char(int row, int col, char _char)
{
    py::gil_scoped_acquire acquire;
    AmberScreen->attr("display_char")(row, col, _char);
    py::gil_scoped_release release;
}

void sysdarft_display_t::unblocked_sleep(unsigned int sec)
{
    py::gil_scoped_release release;
    std::this_thread::sleep_for(std::chrono::milliseconds(sec));
}
