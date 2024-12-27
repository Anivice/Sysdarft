#include <instruction.h>
#include <iomanip>

int main()
{
    try {
        debug::verbose = true;
        std::vector<uint8_t> buffer;
        encode_instruction(buffer, "add .64bit <*2&64(%FER0, $(0xFFFF), $(0x0C))>, <%FER14>");

        for (const auto& code : buffer) {
            std::cout << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                      << static_cast<int>(code) << " ";
        }
        std::cout << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}
