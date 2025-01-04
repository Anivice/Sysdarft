#include <iostream>
#include <cstring>
#include <stdexcept>
#include <rfb/rfb.h>

// supposedly a GPU module to replace curses and supports graphical interfaces,
// though not ganna be implemented anytime soon since the CPU itself is still real-mode only

// Constants
constexpr int FRAMEBUFFER_WIDTH = 800;
constexpr int FRAMEBUFFER_HEIGHT = 600;

// Class representing the VNC Minimal Server
class VNCMiniServer {
public:
    VNCMiniServer(int argc, char** argv)
        : server_(nullptr)
    {
        setupServer(argc, argv);
    }

    ~VNCMiniServer()
    {
        if (server_) {
            rfbScreenCleanup(server_);
        }
    }

    void run()
    {
        std::cout << "Entering server loop.\n";
        while (rfbIsActive(server_))
        {
            if (server_->bitsPerPixel != 0)
            {
                // Cast the framebuffer to a 32-bit integer array for efficient assignment
                auto* fb = reinterpret_cast<uint32_t*>(server_->frameBuffer);
                constexpr size_t total_pixels = FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT;

                // Assign the blue color to each pixel
                for (size_t i = 0; i < total_pixels; ++i)
                {
                    const uint32_t blue_pixel = 0xFF0000FF;
                    fb[i] = blue_pixel;
                }

                // Mark the entire screen as modified to ensure the client updates
                rfbMarkRectAsModified(server_, 0, 0, server_->width, server_->height);
            }

            // Process events with a timeout of 100 ms
            rfbProcessEvents(server_, 100000); // 100 ms timeout
        }

        std::cout << "Server loop exited.\n";
    }

    [[nodiscard]] int getPort() const {
        return server_->port;
    }

private:
    rfbScreenInfoPtr server_;

    void setupServer(int argc, char** argv) {
        // Initialize the VNC server with 32 bits per pixel, 24 depth initially
        server_ = rfbGetScreen(&argc, argv, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 32, 24, 0);
        if (!server_) {
            throw std::runtime_error("Failed to initialize VNC server.");
        }

        // Manually set pixel format fields
        server_->serverFormat.bitsPerPixel = 32;
        server_->serverFormat.depth = 32; // Changed from 24 to 32 to match bitsPerPixel
        server_->serverFormat.bigEndian = 0;
        server_->serverFormat.trueColour = 1;
        server_->serverFormat.redMax = 255;
        server_->serverFormat.greenMax = 255;
        server_->serverFormat.blueMax = 255;
        server_->serverFormat.redShift = 16;
        server_->serverFormat.greenShift = 8;
        server_->serverFormat.blueShift = 0;

        // Correctly set the standalone bitsPerPixel and depth fields
        server_->bitsPerPixel = server_->serverFormat.bitsPerPixel;
        server_->depth = server_->serverFormat.depth;

        // Log pixel format
        std::cout << "Pixel Format:\n"
                  << "BitsPerPixel: " << static_cast<int>(server_->serverFormat.bitsPerPixel) << "\n"
                  << "Depth: " << static_cast<int>(server_->serverFormat.depth) << "\n"
                  << "BigEndian: " << static_cast<int>(server_->serverFormat.bigEndian) << "\n"
                  << "TrueColour: " << static_cast<int>(server_->serverFormat.trueColour) << "\n"
                  << "RedMax: " << static_cast<int>(server_->serverFormat.redMax) << "\n"
                  << "GreenMax: " << static_cast<int>(server_->serverFormat.greenMax) << "\n"
                  << "BlueMax: " << static_cast<int>(server_->serverFormat.blueMax) << "\n"
                  << "RedShift: " << static_cast<int>(server_->serverFormat.redShift) << "\n"
                  << "GreenShift: " << static_cast<int>(server_->serverFormat.greenShift) << "\n"
                  << "BlueShift: " << static_cast<int>(server_->serverFormat.blueShift) << "\n";

        // Allocate framebuffer (32 bits per pixel)
        server_->frameBuffer = static_cast<char*>(malloc(FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 4));
        if (!server_->frameBuffer) {
            throw std::runtime_error("Failed to allocate framebuffer.");
        }
        std::memset(server_->frameBuffer, 0, FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 4); // Initialize to black

        // Set framebuffer format properties
        server_->desktopName = const_cast<char*>("VNC Minimal Server");
        server_->alwaysShared = TRUE;

        // Initialize the server
        rfbInitServer(server_);
    }
};

int main(int argc, char** argv)
{
    try {
        // Create and run the minimal VNC server
        VNCMiniServer server(argc, argv);
        std::cout << "Starting VNC minimal server on port " << server.getPort() << "...\n";
        server.run();
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
