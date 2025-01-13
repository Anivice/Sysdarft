#include <iomanip>
#include <EncodingDecoding.h>

int main()
{
    debug::verbose = true;
    std::vector < std::vector <uint8_t> > code;
    std::map < std::string, std::pair < uint64_t /* line position */, std::vector < uint64_t > > > defined_line_marker;
    defined_line_marker.emplace("_start", std::pair < uint64_t, std::vector < uint64_t > > (0, { }));
    defined_line_marker.emplace("_end", std::pair < uint64_t, std::vector < uint64_t > > (0, { }));
    std::stringstream ascii_code;
    ascii_code << "nop                  \n";
    ascii_code << "_start:              \n";
    ascii_code << "     nop                     \n";
    ascii_code << "     jmp <%CB>, <_end>       \n";
    ascii_code << "_end:                        \n";
    ascii_code << "     call <%CB>, <_start>    \n";
    SysdarftCompile(code, ascii_code, 0, defined_line_marker);

    std::vector < uint8_t > assembled_code;
    for (const auto& instruction : code)
    {
        for (auto xcode : instruction) {
            assembled_code.push_back(xcode);
        }
    }

    const auto space = assembled_code.size();
    std::vector < std::string > lines;
    while (!assembled_code.empty())
    {
        std::stringstream off;
        std::vector < std::string > line;
        off << std::hex << std::setfill('0') << std::setw(16) << std::uppercase << space - assembled_code.size();
        decode_instruction(line, assembled_code);
        off << ": " << line[0];
        lines.push_back(off.str());
    }

    for (const auto& line : lines) {
        std::cout << line << "\n";
    }
}
