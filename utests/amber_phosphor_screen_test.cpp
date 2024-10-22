#include <map>
#include <pybind11/embed.h> // pybind11's embedding module
#include <res_packer.h>
#include <string>
#include <debug.h>
#include <tools.h>

namespace py = pybind11;

// Function to convert Python object (list) to std::map<std::string, std::map<std::string, std::string>>
std::map<std::string, std::map<std::string, std::string>>
convert_input_stream(const py::list &py_list)
{
    if (py_list.empty()) {
        return {};
    }

    std::map<std::string, std::map<std::string, std::string>> cpp_map;

    // Convert the Python list to C++ types
    const auto &outer_key = py::str(py_list[0]);  // First element (outer key)

    // Inner list initialization and size check in if-init-statement
    if (const auto &inner_list = py_list[1].cast<py::list>(); inner_list.size() == 2)
    {
        const auto &inner_key = py::str(inner_list[0]);
        const auto &inner_value = py::str(inner_list[1]);

        // Populate the map
        cpp_map[outer_key][inner_key] = inner_value;
    }

    return cpp_map;
}

// Function to load the script from memory and get the class
py::object load_class_from_script(const unsigned char *script, size_t size, const std::string &class_name)
{
    // Convert the script to a Python string
    py::str py_script(reinterpret_cast<const char*>(script), size);

    // Execute the script in a global namespace
    py::dict globals = py::globals();
    py::exec(py_script, globals);

    // Extract the class from the global namespace
    py::object py_class = globals[class_name.c_str()];

    return py_class;
}

// Singleton class to ensure that the interpreter is initialized only once
class PythonInterpreter {
public:
    static PythonInterpreter& getInstance() {
        static PythonInterpreter instance;
        return instance;
    }

private:
    py::scoped_interpreter guard; // Scoped interpreter
    PythonInterpreter() : guard(true) {} // Private constructor to initialize interpreter
    ~PythonInterpreter() = default; // Destructor ensures cleanup

    // Delete copy constructor and assignment operator
    PythonInterpreter(const PythonInterpreter&) = delete;
    PythonInterpreter& operator=(const PythonInterpreter&) = delete;
};

int main()
{
    try {
        PythonInterpreter& interpreter = PythonInterpreter::getInstance();

        // Manually invoke Python garbage collection
        py::module gc = py::module::import("gc");

        initialize_resource_filesystem();

        auto file = get_res_file_list().at("scripts_AmberScreenEmulator.py");
        auto AmberScreen_t = load_class_from_script(file.file_content,
                                                    file.file_length,
                                                    "AmberScreenEmulator");

        auto libxxd = find_containing_substring(list_shared_libraries(), "libxxd_binary_content.so");
        if (libxxd.empty()) {
            throw sysdarft_error_t(sysdarft_error_t::CANNOT_OBTAIN_DYNAMIC_LIBRARIES);
        }
        auto xxd_lib_path = libxxd.at(0);

        std::regex libPattern("libxxd_binary_content\\.so");
        std::string event_lib_path = std::regex_replace(xxd_lib_path, libPattern, "libsysdarft_event_vec.so");

        // Create an instance of the ScreenEmulator class
        py::object AmberScreen = AmberScreen_t(xxd_lib_path.c_str(), event_lib_path.c_str());

        // Start the emulator (calls the start_service method)
        if (AmberScreen.attr("start_service")().cast<int>() == -1) {
            throw std::runtime_error("Error when starting service loop!");
        }

        // Use the emulator (display and sleep functions)
        AmberScreen.attr("display_char")(0, 0, '$');

        py::gil_scoped_release release;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        py::gil_scoped_acquire acquire;

        // Stop the emulator service
        if (AmberScreen.attr("stop_service")().cast<int>() == -1) {
            throw std::runtime_error("Error when stopping service loop!");
        }

        AmberScreen.attr("join_service_loop")();

        // Explicitly delete the `AmberScreen` object to release it
        AmberScreen.release();

        // Manually trigger garbage collection to ensure all Python objects are cleaned up
        gc.attr("collect")();

        // Clear the global namespace to avoid memory leaks
        py::dict globals = py::globals();
        globals.clear();

    } catch (const std::exception &e) {
        sysdarft_log::log(sysdarft_log::LOG_ERROR, "Error: ", e.what(), "\n");
        return 1;
    }

    return 0;
}
