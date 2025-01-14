#include <SysdarftMain.h>

void compile_to_binary(const std::vector< std::string > & source_files, const std::string & binary_filename)
{
    try {
        std::vector < std::vector < uint8_t > > binary_cct;
        for (const std::string & source_file : source_files)
        {
            std::fstream file(source_file, std::ios::in | std::ios::out);
            if (!file.is_open()) {
                throw SysdarftAssemblerError("Could not open file " + source_file);
            }

            std::vector < uint8_t > binary;
            CodeProcessing(binary, file);

            file.close();
            binary_cct.emplace_back(binary);
        }

        std::ofstream file(binary_filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            throw SysdarftAssemblerError("Could not open file " + binary_filename);
        }

        for (const auto & cct : binary_cct) {
            file.write(reinterpret_cast<const char*>(cct.data()), cct.size());
        }

        file.close();
    } catch (const std::exception & e) {
        throw SysdarftAssemblerError(e.what());
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
    file.read((char*)(assembled_code.data()), file_size);
    if (static_cast<uint64_t>(file.gcount()) != file_size) {
        throw SysdarftDisassemblerError("Short read on file " + binary_filename);
    }

    file.close();

    const auto space = assembled_code.size();
    std::vector < std::string > lines;
    while (!assembled_code.empty())
    {
        std::stringstream off;
        std::vector < std::string > line;
        off << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
            << space - assembled_code.size() + org;

        decode_instruction(line, assembled_code);

        if (!line.empty()) {
            off << ": " << line[0];
        } else {
            off << ": " << "(bad)";
        }

        lines.push_back(off.str());
    }

    for (const auto& line : lines) {
        std::cout << line << "\n";
    }
}
