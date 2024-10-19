#include <iostream>
#include <map>
#include <pybind11/embed.h> // pybind11's embedding module
#include <res_packer.h>
#include <string>
#include <debug.h>

namespace py = pybind11;

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

int main()
{
    // Initialize the Python interpreter
    py::scoped_interpreter guard{};

    try {
        initialize_resource_filesystem();

        auto file = get_res_file_list().at("scripts_AmberScreenEmulator.py");
        auto AmberScreen_t = load_class_from_script(file.file_content,
            file.file_length,
            "AmberScreenEmulator");

        // Create an instance of the ScreenEmulator class
        py::object AmberScreen = AmberScreen_t();

        // Start the emulator (calls the start_service method)
        if (AmberScreen.attr("start_service")().cast<int>() == -1) {
            throw std::runtime_error("Error when starting service loop!");
        }

        // Optionally, query the input stream
        auto input_stream = convert_input_stream(AmberScreen.attr("query_input_stream")());

        AmberScreen.attr("display_char")(0, 0, ' ');
        AmberScreen.attr("sleep")(1);

        if (AmberScreen.attr("stop_service")().cast<int>() == -1) {
            throw std::runtime_error("Error when stopping service loop!");
        }
    } catch (const std::exception &e) {
        sysdarft_log::log(sysdarft_log::LOG_ERROR, "Error: ", e.what(), "\n");
        return 1;
    }

    return 0;
}
