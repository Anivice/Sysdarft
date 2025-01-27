#include <EncodingDecoding.h>
#include <fstream>

const std::regex include_pattern(R"(\s*\%include\s+\"(.*)\"\s*)", std::regex_constants::icase);
const std::regex ifdef_pattern(R"(\s*\%ifdef\s+([A-Za-z_]+|[A-Za-z0-9_]+)\s*)", std::regex_constants::icase);
const std::regex ifndef_pattern(R"(\s*\%ifndef\s+([A-Za-z_]+|[A-Za-z0-9_]+)\s*)", std::regex_constants::icase);
const std::regex define_pattern(R"(\s*\%define\s+([A-Za-z_]+|[A-Za-z0-9_]+)(.*))", std::regex_constants::icase);
const std::regex else_pattern(R"(\s*\%else\s*)", std::regex_constants::icase);
const std::regex endif_pattern(R"(\s*\%endif\s*)");
const std::regex warning_pattern(R"(\s*\%warning\s+(.*))");
const std::regex error_pattern(R"(\s*\%error\s+(.*))");

void process_include(std::string & line, header_file_list_t & file_list, const uint64_t line_number)
{
    std::smatch match;
    std::regex_match(line, match, include_pattern);
    const std::string include_file = match[1].str();

    std::fstream infile(include_file, std::ios::in);
    if (!infile) {
        throw SysdarftPreProcessorError("Couldn't open include file " + include_file);
    }

    std::vector < std::string > content;
    std::string fline;
    while (std::getline(infile, fline)) {
        content.push_back(fline);
    }

    include_file_t file = { .file_name = include_file, .appearance_at_line_num = line_number, .content = content };

    file_list.emplace_back(file);
    line.clear();
}

void process_define(std::string & line, source_file_c_style_definition_t & definition_list)
{
    std::smatch match;
    std::regex_match(line, match, define_pattern);
    std::string marco_name = match[1].str();
    std::string marco_value = match.size() == 3 ? match[2].str() : "";
    replace_all(marco_name, " ", "");
    replace_all(marco_value, " ", "");
    definition_list.emplace(marco_name, marco_value);
    line = ".equ '" + marco_name + "', '" + marco_value + "'";
}

std::vector<std::string> splitStringByComma(const std::string& input)
{
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;

    while (std::getline(ss, item, ',')) {
        replace_all(item, " ", "");
        result.push_back(item);
    }

    return result;
}

std::string directive_parameter_extraction(const std::string& line, const std::regex & pattern)
{
    std::smatch match;
    std::regex_match(line, match, pattern);
    return match[1].str();
}

std::string what_required_by_by_ifdef(const std::string & line)
{
    return directive_parameter_extraction(line, ifdef_pattern);
}

std::string what_required_by_by_ifndef(const std::string & line)
{
    return directive_parameter_extraction(line, ifndef_pattern);
}

std::string what_reported_by_by_error(const std::string & line)
{
    return directive_parameter_extraction(line, error_pattern);
}

std::string what_reported_by_by_warning(const std::string & line)
{
    return directive_parameter_extraction(line, warning_pattern);
}

std::string truncateAfterSemicolonOrHash(const std::string&);

void HeadProcess(
    std::vector < std::string > & file,
    source_file_c_style_definition_t & definition,
    header_file_list_t & header_files)
{

    uint64_t line_number = 0;
    bool inside_ifdef = false;
    bool requested_marco_present = false;

    for (auto & line : file)
    {
        line_number++;

        replace_all(line, "\t", "    ");
        line = truncateAfterSemicolonOrHash(line);

        if (std::regex_match(line, endif_pattern))
        {
            inside_ifdef = false;
            requested_marco_present = false;
            line.clear();
            continue;
        }

        if (inside_ifdef && !requested_marco_present)
        {
            if (std::regex_match(line, else_pattern)) {
                requested_marco_present = true;
            }

            line.clear();
            continue;
        }

        if (std::regex_match(line, include_pattern)) {
            process_include(line, header_files, line_number);
        } else if (std::regex_match(line, define_pattern)) {
            process_define(line, definition);
        } else if (std::regex_match(line, ifdef_pattern)) {
            const auto what_is_being_requested = what_required_by_by_ifdef(line);
            inside_ifdef = true;
            requested_marco_present = definition.contains(what_is_being_requested);
            line.clear();
        }  else if (std::regex_match(line, ifndef_pattern)) {
            const auto what_is_being_requested = what_required_by_by_ifndef(line);
            inside_ifdef = true;
            requested_marco_present = !definition.contains(what_is_being_requested);
            line.clear();
        } else if (std::regex_match(line, warning_pattern)) {
            std::cerr << "\033[31;1mWarning: " << what_reported_by_by_warning(line) << "\033[0m" << std::endl;
        } else if (std::regex_match(line, error_pattern)) {
            throw SysdarftPreProcessorError("Exception caught from source file: " + what_reported_by_by_error(line));
        }
    }
}