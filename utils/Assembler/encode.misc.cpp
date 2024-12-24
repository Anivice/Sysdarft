#include <instruction.h>
#include <debug.h>
#include <vector>
#include <regex>
#include <iomanip>

std::regex line_marker_pattern(R"()");
std::map < std::string /* line marker */,
    std::pair < uint64_t /* original code location */,
    uint64_t /* the actual address */ >
> line_marker_map;

void encode_line_markers(std::vector < uint64_t > & line_markers, std::string & lines)
{

}
