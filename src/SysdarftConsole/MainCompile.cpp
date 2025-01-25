/* MainCompile.cpp
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <SysdarftMain.h>

#define EXE_MAGIC ((uint32_t)(0x00455845))
#define SYS_MAGIC ((uint32_t)(0x00535953))

std::vector < uint8_t > generate_symbol_table(const object_t & obj)
{
    std::vector < uint8_t > symbol_table;
    symbol_table.resize(sizeof(uint64_t));
    const uint64_t entry_size = obj.symbol_table.size();
    std::memcpy(symbol_table.data(), &entry_size, sizeof(uint64_t));

    for (auto const & symbol : obj.symbol_table)
    {
        std::vector < uint8_t > symbol_table_entry;
        symbol_table_entry.resize(sizeof(uint16_t) + symbol.line_marker_name.size() + sizeof(uint64_t));

        uint64_t offset = 0;
        auto symbol_len = static_cast<uint16_t>(symbol.line_marker_name.size());

        std::memcpy(symbol_table_entry.data(), &symbol_len, sizeof(uint16_t));
        offset += sizeof(uint16_t);

        std::memcpy(symbol_table_entry.data() + offset,
            symbol.line_marker_name.c_str(),
            symbol.line_marker_name.size());
        offset += symbol.line_marker_name.size();

        std::memcpy(symbol_table_entry.data() + offset, &symbol.marker_position, sizeof(uint64_t));
        symbol_table.insert(symbol_table.end(), symbol_table_entry.begin(), symbol_table_entry.end());
    }

    return symbol_table;
}

void compile_to_binary(const std::vector<std::string> &source_files, const std::string &binary_filename, const bool regex,
                       const COMPILATION_MODE compile_mode)
{
    struct file_attr_t {
        defined_line_marker_t symbol_table;
        std::vector < std::vector< uint8_t > > code;
        std::vector < std::string > file;
        object_t object;
        std::string filename;
    };

    uint64_t org = 0;
    std::vector < file_attr_t > files;

    // reading
    for (const std::string & source_file : source_files)
    {
        try {
            file_attr_t file;
            std::fstream filestream(source_file, std::ios::in | std::ios::out);
            if (!filestream.is_open()) {
                throw SysdarftAssemblerError("Could not open file " + source_file);
            }

            std::string line;
            while (std::getline(filestream, line)) {
                file.file.push_back(line);
            }

            file.filename = source_file;

            files.emplace_back(file);
        } catch (const std::exception & e) {
            throw SysdarftAssemblerError("Error when processing file " + source_file + ": " + e.what());
        }
    }

    // preprocessing
    for (file_attr_t & file : files) {
        try {
            PreProcess(file.file, file.symbol_table, org, regex);
        } catch (const std::exception & err) {
            throw std::runtime_error("Error when processing file " + file.filename + ": " + err.what());
        }
    }

    // compiling
    for (file_attr_t & file : files) {
        try {
            file.object = SysdarftAssemble(file.code, file.file, org, file.symbol_table);
        } catch (const std::exception & err) {
            throw std::runtime_error("Error when processing file " + file.filename + ": " + err.what());
        }
    }

    std::vector <object_t> objects;

    // archiving
    for (file_attr_t & file : files) {
        objects.push_back(file.object);
    }

    object_t linked_object = SysdarftLink(objects);

    try {
        std::ofstream file(binary_filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            throw SysdarftAssemblerError("Could not open file " + binary_filename);
        }

        if (compile_mode == BIN) {
            for (const auto & cct : linked_object.code) {
                file.write(reinterpret_cast<const char*>(cct.data()), static_cast<ssize_t>(cct.size()));
            }
        } else if (compile_mode == EXE || compile_mode == SYS) {
            uint32_t magic = (compile_mode == EXE ? EXE_MAGIC : SYS_MAGIC);
            file.write((const char*)&magic, sizeof(magic));
            auto symbol_table = generate_symbol_table(linked_object);
            file.write((const char*)symbol_table.data(), static_cast<ssize_t>(symbol_table.size()));
            for (const auto & cct : linked_object.code) {
                file.write(reinterpret_cast<const char*>(cct.data()), static_cast<ssize_t>(cct.size()));
            }
        } else {
            throw SysdarftAssemblerError("Format not recognized");
        }

        file.close();
    } catch (const std::exception & e) {
        throw SysdarftAssemblerError("Error when writing to output file " + binary_filename + ": " + e.what());
    }
}

template < typename Type >
Type pop_num_from_vec(std::vector < uint8_t > & vec)
{
    Type ret {};
    if (vec.size() < sizeof(Type)) {
        throw SysdarftDisassemblerError("Invalid vector size");
    }

    std::memcpy((char*)&ret, vec.data(), sizeof(Type));
    vec.erase(vec.begin(), vec.begin() + sizeof(Type));
    return ret;
}

std::string pop_str_from_vec(std::vector < uint8_t > & vec)
{
    const auto len = pop_num_from_vec<uint16_t>(vec);
    std::string ret;
    ret.resize(len);
    if (vec.size() < len) {
        throw SysdarftDisassemblerError("Invalid vector size");
    }
    std::memcpy(ret.data(), vec.data(), len);
    vec.erase(vec.begin(), vec.begin() + len);
    return ret;
}

void print_table(const std::map < uint64_t, std::string > & table)
{
    std::cout << "SYMBOL TABLE - SIZE " << table.size() << ":" << std::endl;
    for (const auto & [value, entry] : table) {
        std::cout << std::hex << std::setfill('0') << std::setw(16) << std::uppercase << value;
        std::cout << "                             " << entry << std::endl;
    }
    std::cout << "\n" << std::endl;
}

void disassemble(const std::string & binary_filename, const uint64_t org, COMPILATION_MODE compile_mode)
{
    std::ifstream file(binary_filename, std::ios::in | std::ios::binary);
    std::vector < uint8_t > assembled_code;
    const auto file_size = std::filesystem::file_size(binary_filename);
    std::map < uint64_t, std::string > symbol_table;

    // read
    if (!file.is_open()) {
        throw SysdarftDisassemblerError("Could not open file " + binary_filename);
    }

    std::cout << "\n" << binary_filename << "        FORMAT    ";

    assembled_code.resize(file_size);
    file.read((char*)(assembled_code.data()), static_cast<ssize_t>(file_size));

    if (assembled_code.size() >= 3 && assembled_code[0] == 'E' && assembled_code[1] == 'X' && assembled_code[2] == 'E') {
        compile_mode = EXE;
    }

    if (assembled_code.size() >= 3 && assembled_code[0] == 'S' && assembled_code[1] == 'Y' && assembled_code[2] == 'S') {
        compile_mode = SYS;
    }

    if (compile_mode == SYS || compile_mode == EXE)
    {
        const auto magic = pop_num_from_vec<uint32_t>(assembled_code);
        if (magic != EXE_MAGIC && magic != SYS_MAGIC) {
            throw SysdarftDisassemblerError("Format not recognized");
        }

        switch (magic) {
            case EXE_MAGIC: std::cout << "EXE\n" << std::endl; break;
            case SYS_MAGIC: std::cout << "SYS\n" << std::endl; break;
        }

        const auto entry_size = pop_num_from_vec<uint64_t>(assembled_code);
        for (uint64_t i = 0; i < entry_size; i++) {
            const auto entry = pop_str_from_vec(assembled_code);
            const auto value = pop_num_from_vec<uint64_t>(assembled_code);
            symbol_table.emplace(value, entry);
        }

        // print symbol table
        print_table(symbol_table);
    } else {
        std::cout << "BIN\n" << std::endl;
    }

    if (static_cast<uint64_t>(file.gcount()) != file_size) {
        throw SysdarftDisassemblerError("Short read on file " + binary_filename);
    }

    file.close();

    std::cout << disassemble_code(assembled_code, org, symbol_table) << std::endl;
}
