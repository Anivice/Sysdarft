#include <pybind11/embed.h> // pybind11's embedding module
#include <iostream>
#include <map>
#include <string>
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

int main()
{
    // Initialize the Python interpreter
    py::scoped_interpreter guard{};

    try {
        // Import the Python module
        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("append")(CMAKE_SOURCE_DIR "/scripts");
        py::object AmberScreenEmulator = py::module_::import("AmberScreenEmulator");

        // Get the ScreenEmulator class from the module
        py::object AmberScreen_t = AmberScreenEmulator.attr("AmberScreenEmulator");

        // Create an instance of the ScreenEmulator class
        py::object AmberScreen = AmberScreen_t();

        // Start the emulator (calls the start_service method)
        if (AmberScreen.attr("start_service")().cast<int>() == -1) {
            throw std::runtime_error("Error when starting service loop!");
        }

        // Optionally, query the input stream
        auto input_stream = convert_input_stream(AmberScreen.attr("query_input_stream")());

        AmberScreen.attr("sleep")(1);

        if (AmberScreen.attr("stop_service")().cast<int>() == -1) {
            throw std::runtime_error("Error when stopping service loop!");
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
