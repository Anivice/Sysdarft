#include <SysdarftMain.h>

void compile_to_binary(const std::vector< std::string > & source_files, const std::string & binary_filename)
{
    std::vector < std::vector < uint8_t > > binary_cct;
    for (const std::string & source_file : source_files)
    {
        try {

            std::fstream file(source_file, std::ios::in | std::ios::out);
            if (!file.is_open()) {
                throw SysdarftAssemblerError("Could not open file " + source_file);
            }

            std::vector < uint8_t > binary;
            CodeProcessing(binary, file);

            file.close();
            binary_cct.emplace_back(binary);
        } catch (const std::exception & e) {
            throw SysdarftAssemblerError("Error when processing file " + source_file + ": " + e.what());
        }
    }

    try {
        std::ofstream file(binary_filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            throw SysdarftAssemblerError("Could not open file " + binary_filename);
        }

        for (const auto & cct : binary_cct) {
            file.write(reinterpret_cast<const char*>(cct.data()), static_cast<ssize_t>(cct.size()));
        }

        file.close();
    } catch (const std::exception & e) {
        throw SysdarftAssemblerError("Error when writing to output file " + binary_filename + ": " + e.what());
    }
}

void disassemble(const std::string & binary_filename, const uint64_t org)
{
    std::ifstream file(binary_filename, std::ios::in | std::ios::binary);
    std::vector < uint8_t > assembled_code;
    auto file_size = std::filesystem::file_size(binary_filename);

    // read
    if (!file.is_open()) {
        throw SysdarftDisassemblerError("Could not open file " + binary_filename);
    }

    assembled_code.resize(file_size);
    file.read((char*)(assembled_code.data()), static_cast<ssize_t>(file_size));
    if (static_cast<uint64_t>(file.gcount()) != file_size) {
        throw SysdarftDisassemblerError("Short read on file " + binary_filename);
    }

    file.close();

    const std::regex _8bit_data(R"(.8bit_data <(.*)>)");
    const auto space = assembled_code.size();
    std::vector < std::string > lines;
    std::vector < std::pair < uint64_t, std::string > > bad_data;
    auto process_bad_data = [&]()->void
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
           << bad_data.front().first;
        ss << ": .8bit_data <";
        for (auto & [location, data] : bad_data) {
            ss << " " << data << ",";
        }
        ss << " >";
        bad_data.clear();
        lines.push_back(ss.str());
    };

    while (!assembled_code.empty())
    {
        std::stringstream off;
        std::vector < std::string > line;
        auto current_pos = space - assembled_code.size() + org;
        off << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
            << current_pos;

        decode_instruction(line, assembled_code);

        if (!line.empty())
        {
            bool continue_flag = false;
            for (const auto & ls : line)
            {
                if (std::smatch match; std::regex_match(ls, match, _8bit_data)) {
                    bad_data.emplace_back(current_pos, match[1].str());
                    continue_flag = true;
                }
            }

            if (continue_flag) {
                continue;
            }

            // not a match, flash cache
            if (!bad_data.empty())
            {
                process_bad_data();
            }

            off << ": " << line[0];
            lines.push_back(off.str());
        }
    }

    if (!bad_data.empty())
    {
        process_bad_data();
    }

    for (const auto& line : lines) {
        std::cout << line << "\n";
    }
}
