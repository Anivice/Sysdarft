#include <SysdarftMain.h>
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

std::string insert_newlines_every_24(const std::string& input)
{
    constexpr std::size_t line_length = 24;
    if (input.length() < line_length) {
        return input;
    }

    std::string result;
    result.reserve(input.size() + input.size() / line_length);
    // Reserve space considering added newlines

    for (std::size_t i = 0; i < input.size(); ++i)
    {
        if (i > 0 && i % line_length == 0) {
            result.push_back('\n');
        }
        result.push_back(input[i]);
    }

    return result;
}

std::string disassemble_code(std::vector < uint8_t > assembled_code, const uint64_t org)
{
    auto assembled_code_backup = assembled_code;
    std::stringstream ret;
    const std::regex _8bit_data(R"(.8bit_data <(.*)>)");
    const auto space = assembled_code.size();
    std::vector < std::string > lines;
    std::vector < std::pair < uint64_t, std::string > > bad_data;
    auto process_bad_data = [&]()->void
    {
        std::stringstream ss;
        std::stringstream dat;

        ss << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
           << bad_data.front().first;
        ss << ": ";

        dat << ".8bit_data <";
        for (auto & [location, data] : bad_data) {
            dat << " " << data << ",";
        }
        dat << " >";
        bad_data.clear();

        lines.push_back(ss.str());
        lines.push_back(dat.str());
        lines.emplace_back("");
    };

    auto ins_code = [&](const std::vector<uint8_t> & code)->std::string
    {
        std::stringstream ss;
        for (const auto & c : code) {
            ss << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
               << static_cast<int>(c);
            ss << " ";
        }

        return ss.str();
    };

    while (!assembled_code.empty())
    {
        std::stringstream off;
        std::vector < std::string > line;
        auto current_pos = space - assembled_code.size() + org;
        off << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
            << current_pos;

        auto offset_before = space - assembled_code.size();
        decode_instruction(line, assembled_code);
        auto offset_after = space - assembled_code.size();

        auto code_length = offset_after - offset_before;
        std::vector < uint8_t > current_code;
        current_code.resize(code_length);
        std::memcpy(
            current_code.data(),
            assembled_code_backup.data() + offset_before,
            code_length);

        if (!line.empty())
        {
            bool continue_flag = false;
            // if bad data
            for (const auto & ls : line)
            {
                if (std::smatch match; std::regex_match(ls, match, _8bit_data)) {
                    bad_data.emplace_back(current_pos, match[1].str());
                    continue_flag = true;
                }
            }

            // bad, skip print
            if (continue_flag) {
                continue;
            }

            // not a match, flash cache
            if (!bad_data.empty())
            {
                process_bad_data();
            }

            off << ": ";
            lines.push_back(off.str());
            lines.push_back(ins_code(current_code));
            lines.push_back(line[0]);
            lines.emplace_back("");
        }
    }

    if (!bad_data.empty())
    {
        process_bad_data();
    }

    auto it = lines.begin();
    if (lines.size() < 3) {
        return "";
    }

    while (it != lines.end())
    {
        // check DATA2 in [cur] [DATA] [DATA2]
        const auto is_8bit_data = (it + 2)->empty(); // if empty, then 8bit data, else, no
        if (!is_8bit_data)
        {
            const auto & off_marker = *it;
            auto ins_code_exp = *(it + 1);
            ins_code_exp = insert_newlines_every_24(ins_code_exp);
            const auto & instruction_statement = *(it + 2);
            std::string padding_before(18, ' ');
            int padding_after_len = static_cast<int>(24 - (ins_code_exp.size() - ins_code_exp.find_last_of('\n') - 1));

            if (ins_code_exp.size() > 24)
            {
                // '00000000000C18D3: ', 18 bytes
                replace_all(ins_code_exp, "\n", "\n" + padding_before);
                std::string padding_after(padding_after_len, ' ');
                ins_code_exp += padding_after;

                auto pos = ins_code_exp.find_first_of('\n');
                ins_code_exp.insert(pos, "   " + instruction_statement);

                ret << off_marker << ins_code_exp << std::endl;
            } else {
                std::string padding_after(27 - ins_code_exp.size(), ' ');
                ret << off_marker << ins_code_exp << padding_after << instruction_statement << std::endl;
            }

            it += 4;
        } else {
            ret << *it << *(it + 1) << std::endl;
            it += 3;
        }
    }

    return ret.str();
}

void RemoteDebugServer::crow_setup_disassemble_an_area()
{
    CROW_ROUTE(JSONBackend, "/DisassembleMemory").methods(crow::HTTPMethod::GET)
        ([this](const crow::request& req)
        {
            // Create a JSON object using nlohmann/json
            json response;
            response["Version"] = SYSDARFT_VERSION;
            const auto timeNow = std::chrono::system_clock::now();
            response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                timeNow.time_since_epoch()).count());

            if (!breakpoint_triggered) {
                response["Result"] = "Still running";
                return crow::response(400, response.dump());
            }

            std::string bodyString = req.body;
            json clientJson;
            try {
                clientJson = json::parse(bodyString);
            } catch (const std::exception&) {
                response["Result"] = "Invalid Argument";
                return crow::response(400, response.dump());
            }

            if  (!clientJson.contains("Begin"))
            {
                response["Result"] = "Invalid Argument (Missing begin)";
                return crow::response(400, response.dump());
            }

            if  (!clientJson.contains("End"))
            {
                response["Result"] = "Invalid Argument (Missing end)";
                return crow::response(400, response.dump());
            }

            try {
                std::string begin = clientJson["Begin"];
                std::string end = clientJson["End"];
                remove_spaces(begin);
                remove_spaces(end);

                process_base16(begin);
                process_base16(end);

                if (begin.empty()) {
                    throw std::invalid_argument("Invalid Expression");
                }

                if (end.empty()) {
                    throw std::invalid_argument("Invalid Expression");
                }

                const uint64_t beginAddress = std::stoull(begin);
                const uint64_t endAddress = std::stoull(end);

                if (beginAddress >= endAddress) {
                    throw std::invalid_argument("Invalid Expression");
                }

                std::vector<std::string> instructions;
                const uint64_t offset = std::min(beginAddress, CPUInstance.SystemTotalMemory());
                const uint64_t length = std::min<uint64_t>(endAddress - beginAddress, CPUInstance.SystemTotalMemory() - offset);
                std::vector<uint8_t> buffer;
                buffer.resize(length);
                CPUInstance.read_memory(offset, (char*)buffer.data(), length);
                response["Result"] = disassemble_code(buffer, beginAddress);
            } catch (const std::exception & e) {
                response["Result"] = "Actionable Expression Error: " + std::string(e.what());
                return crow::response(400, response.dump());
            }

            return crow::response{response.dump()};
        });
}
