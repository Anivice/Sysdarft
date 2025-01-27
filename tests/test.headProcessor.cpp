#include <EncodingDecoding.h>

int main()
{
    std::vector < std::string > file = {
        "%ifndef RTC_ASM",
        "%define RTC_ASM",
        " %include \"rtc.asm\" ",
        " %define MARCO_PLAIN ",
        " %define MARCO_CONTENT MARCO_CONTENT ",
        "%endif",
        "add .64bit <%fer0>, <$(1)>"
    };

    auto file2 = file;

    source_file_c_style_definition_t definition;
    header_file_list_t header_files, header_files2;

    HeadProcess(file, definition, header_files);
    HeadProcess(file2, definition, header_files2);
}
