#include "cli_local.h"
#include <readline/readline.h>
#include <iterator>

std::vector<std::string> get_command_names()
{
    std::vector<std::string> names;
    for (const auto& c : commands) {
        names.push_back(c.name);
    }
    return names;
}

const CommandInfo* find_command_info(const std::string& name)
{
    for (const auto& command : commands) {
        if (command.name == name) return &command;
    }
    return nullptr;
}

// A helper function to produce dynamic completions based on command context
// Here we customize results depending on previously entered arguments.
std::vector<std::string> get_dynamic_completions(const std::string& cmd_name,
                                                 const std::vector<std::string>& /* words */,
                                                 int /* arg_index */)
{
    std::vector<std::string> results;

    if (cmd_name == "unload_module") {
        for (const auto& module : loaded_modules) {
            results.emplace_back(module.first);
        }
    }

    return results;
}

// We'll store context for dynamic completions so that the completer can access it.
static std::string current_cmd;
static std::vector<std::string> current_words;
static int current_arg_index;

char* dynamic_arg_completer(const char* text, int state)
{
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0) {
        matches.clear();
        match_index = 0;

        std::string prefix(text);
        // Use the command and words to get command-specific dynamic completions
        auto dynamic = get_dynamic_completions(current_cmd, current_words, current_arg_index);

        // Filter completions by prefix
        for (const auto& candidate : dynamic) {
            if (candidate.rfind(prefix, 0) == 0) {
                matches.push_back(candidate);
            }
        }
    }

    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    } else {
        return nullptr;
    }
}

char* static_arg_completer(const char* text, int state, const std::vector<std::string>& completions)
{
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0) {
        matches.clear();
        match_index = 0;
        std::string prefix(text);
        for (const auto& c : completions) {
            if (c.rfind(prefix, 0) == 0) {
                matches.push_back(c);
            }
        }
    }

    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    } else {
        return nullptr;
    }
}

char* command_name_completer(const char* text, int state)
{
    static std::vector<std::string> cmd_matches;
    static size_t match_index = 0;

    if (state == 0) {
        cmd_matches.clear();
        match_index = 0;
        std::string prefix(text);
        for (auto& cmd : get_command_names()) {
            if (cmd.rfind(prefix, 0) == 0) {
                cmd_matches.push_back(cmd);
            }
        }
    }

    if (match_index < cmd_matches.size()) {
        return strdup(cmd_matches[match_index++].c_str());
    } else {
        return nullptr;
    }
}

static const std::vector<std::string>* current_completions = nullptr;
static const char * empty_string = "";

char* static_completions_proxy(const char* t, int s)
{
    if (current_completions) {
        return static_arg_completer(t, s, *current_completions);
    } else {
        return const_cast<char *>(empty_string);
    }
}

// Main completion function
char** custom_completion(const char* text, int start, int end)
{
    // Parse the line
    char* buffer = strndup(rl_line_buffer, end);
    std::string input(buffer);
    free(buffer);

    std::istringstream iss(input);
    std::vector<std::string> words{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

    // If we're completing the first word (command)
    if (words.empty() || (words.size() == 1 && start == 0)) {
        rl_attempted_completion_over = 1;
        return rl_completion_matches(text, command_name_completer);
    }

    // Identify the command and argument
    const CommandInfo* cmd_info = find_command_info(words[0]);
    if (!cmd_info) {
        // Unknown command
        rl_attempted_completion_over = 1;
        return nullptr;
    }

    int arg_index = static_cast<int>(words.size()) - 1;
    if (arg_index < 0 || arg_index >= (int)cmd_info->arguments.size()) {
        // No argument info defined at this position
        rl_attempted_completion_over = 1;
        return nullptr;
    }

    const auto&[use_file_completion, static_completions, dynamic_completions] =
        cmd_info->arguments[arg_index];

    // Store context for dynamic completion
    current_cmd = words[0];
    current_words = words;
    current_arg_index = arg_index;

    // File completion
    if (use_file_completion) {
        rl_attempted_completion_over = 0; // Allow default filename completion
        return nullptr;
    }

    // Static completion
    if (!static_completions.empty()) {
        rl_attempted_completion_over = 1;
        current_completions = &static_completions;
        return rl_completion_matches(text, static_completions_proxy);
    }

    // Dynamic completion
    if (dynamic_completions) {
        rl_attempted_completion_over = 1;
        return rl_completion_matches(text, dynamic_arg_completer);
    }

    rl_attempted_completion_over = 1;
    return nullptr;
}
