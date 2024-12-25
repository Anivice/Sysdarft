// main.cpp

#include "crow_all.h"           // Include Crow
#include <nlohmann/json.hpp>    // For JSON handling
#include <boost/process.hpp>    // For process management
#include <thread>
#include <string>
#include <dlfcn.h>              // get_shared_library_path.cpp
#include <debug.h>
#include <global.h>
#include <cstdio>
#include <vector>
#include "shutdown.h"
#include <ui_curses.h>

std::string getSharedLibraryPath()
{
    Dl_info dl_info;
    // Use a known function within the shared library; here, it's getSharedLibraryPath itself
    if (dladdr((void*)getSharedLibraryPath, &dl_info)) {
        if (dl_info.dli_fname) {
            return dl_info.dli_fname;
        }
    }
    return "";
}

namespace bp = boost::process;

class backend {
private:
    std::atomic<bool> shutdown_confirmed = false;
    std::atomic<bool> shutdown_finished = true;
    std::thread worker;
    using json = nlohmann::json;
    bp::child py_child;

    // Function to start the Python client using Boost.Process
    void start_python_client()
    {
        try {
            std::string python_executable = "/usr/bin/python3";
            std::string prefix = getSharedLibraryPath();
            while (prefix.back() != '/') {
                prefix.pop_back();
            }

            std::string python_script = prefix + "/resources/main.py";

            // Launch the Python script as a detached child process
            py_child = bp::child(python_executable, python_script);
            py_child.detach();

            std::cout << "Python client started successfully.\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to start Python client: " << e.what() << "\n";
        }
    }

    void main_thread()
    {
        shutdown_confirmed = false;
        shutdown_finished = false;

        // Create a Crow app
        crow::SimpleApp app;

        // Define the route for processing JSON
        CROW_ROUTE(app, "/portal").methods("POST"_method)
        ([](const crow::request& req) -> crow::response
        {
            try {
                // Parse the incoming JSON
                auto received_json = json::parse(req.body);

                // Example processing: Add new fields
                received_json["processed"] = true;
                received_json["message"] = "Data processed successfully by C++ server.";

                // Serialize JSON to string
                std::string response_str = received_json.dump();

                // Return the response with Content-Type: application/json
                return {200, "application/json; charset=utf-8", response_str};
            }
            catch (const json::parse_error &e) {
                std::cerr << "JSON parse error: " << e.what() << "\n";
                const json error_json = {{"error", "Invalid JSON format"}, {"details", e.what()}};
                return {400, "application/json; charset=utf-8", error_json.dump() };
            }
            catch (const std::exception &e) {
                std::cerr << "C++ exception: " << e.what() << "\n";
                const json error_json = {{"error", "Server error"}, {"details", e.what()}};
                return {500, "application/json; charset=utf-8", error_json.dump()};
            }
        });

        // Define the /shutdown route
        CROW_ROUTE(app, "/shutdown").methods("POST"_method)
        ([&app, this](const crow::request& /* req */) -> crow::response
        {
            std::cout << "Shutdown request received." << std::endl;
            shutdown_confirmed = true;
            auto shutdown = [&app, this]()->void
            {
                std::cout << "Shutdown in " << std::flush;
                for (signed int i = 5; i > 0; i--) {
                    std::cout << i << " " << std::flush;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }

                std::cout << std::endl;
                app.stop(); // Gracefully stop the server
                py_child.terminate(); // not so gracefully kill my child
                shutdown_finished = true;
            };

            std::thread shutdown_thread(shutdown);
            shutdown_thread.detach();

            return {200, "text/plain",
                "Server will be shut down in 5 seconds."};
        });

        // Define the /shutdown route
        CROW_ROUTE(app, "/status").methods("POST"_method)
        ([this](const crow::request& /* req */) -> crow::response
        {
            std::string ret = R"({"status":"alive"})";
            if (shutdown_confirmed) {
                ret = R"({"status":"dying"})";
            }

            return {200, "application/json; charset=utf-8", ret};
        });

        // Start the Python client in a separate thread
        std::thread python_thread(&backend::start_python_client, this);

        // Start the Crow server on port 50001
        std::cout << "Starting server on port 50001...\n";
        app.port(50001).multithreaded().run();

        // Wait for the Python client thread to finish (if it ever does)
        if(python_thread.joinable()){
            python_thread.join();
        }
    }

public:
    void cleanup()
    {
        if (!shutdown_finished)
        {
            send_shutdown_request();

            while (!shutdown_finished) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            debug::log("main thread exited!\n");
        }
    }

    void initialize()
    {
        worker = std::thread(&backend::main_thread, this);
        worker.detach();
        debug::log("main thread started!\n");
    }

    void set_cursor(int, int) { }
    cursor_position_t get_cursor() { return { 0, 0 }; }
    void display_char(int, int, int) { }
    void set_cursor_visibility(bool) { }
} dummy;

extern "C" {
    int EXPORT module_init(void);
    void EXPORT module_exit(void);
    std::vector<std::string> EXPORT module_dependencies(void);
}

std::vector<std::string> EXPORT module_dependencies(void)
{
    return { };
}

int EXPORT module_init(void)
{
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
    UI_CLEANUP_METHOD_NAME, &backend::cleanup);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_INITIALIZE_METHOD_NAME, &backend::initialize);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_SET_CURSOR_METHOD_NAME, &backend::set_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_GET_CURSOR_METHOD_NAME, &backend::get_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_DISPLAY_CHAR_METHOD_NAME, &backend::display_char);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_SET_CURSOR_VISIBILITY_METHOD_NAME, &backend::set_cursor_visibility);
    debug::log("UI instance overridden!\n");
    return 0;
}

void EXPORT module_exit(void)
{
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
    UI_CLEANUP_METHOD_NAME, &ui_curses::cleanup);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_INITIALIZE_METHOD_NAME, &ui_curses::initialize);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_SET_CURSOR_METHOD_NAME, &ui_curses::set_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_GET_CURSOR_METHOD_NAME, &ui_curses::get_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_DISPLAY_CHAR_METHOD_NAME, &ui_curses::display_char);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_SET_CURSOR_VISIBILITY_METHOD_NAME, &ui_curses::set_cursor_visibility);
    debug::log("UI instance restored!\n");
    debug::log("Backend exited!\n");
}
