#ifndef DEBUG_H
#define DEBUG_H

#ifdef log
#warning Marco "log" being overriden by debug.h!
#undef log
#endif // log

#include <atomic>
#include <map>
#include <mutex>
#include <pthread.h>
#include <regex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <sstream>

#define SYSDARFT_EXPORT_SYMBOL __attribute__((visibility("default")))

// This contains all functions for debug purposes
namespace debug
{
    // contains command status, including stdout, stderr and exit code
    struct cmd_status
    {
        std::string fd_stdout; // normal output
        std::string fd_stderr; // error information
        int exit_status{}; // exit status
    };

    // execute a command from system shell and return its information
    cmd_status SYSDARFT_EXPORT_SYMBOL _exec_command(const std::string &cmd,
        const std::vector<std::string> &args, const std::string &input);
    // system shell command executor, wrapper for `_exec_command`
    template <typename... Strings>
    cmd_status exec_command(const std::string& cmd, const std::string &input, Strings&&... args);

    // returns current timesstamp
    std::string SYSDARFT_EXPORT_SYMBOL get_current_date_time();

    // helper functions:
    /////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T, typename = void>
    struct is_container : std::false_type
    {
    };

    template <typename T>
    struct is_container<T,
                        std::void_t<decltype(std::begin(std::declval<T>())),
                                    decltype(std::end(std::declval<T>()))>> : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_container_v = is_container<T>::value;

    template <typename T>
    struct is_map : std::false_type
    {
    };

    template <typename Key, typename Value>
    struct is_map<std::map<Key, Value>> : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_map_v = is_map<T>::value;

    template <typename T>
    struct is_unordered_map : std::false_type
    {
    };

    template <typename Key, typename Value, typename Hash, typename KeyEqual,
              typename Allocator>
    struct is_unordered_map<
            std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>>
        : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_unordered_map_v = is_unordered_map<T>::value;

    template <typename T>
    struct is_string : std::false_type
    {
    };

    template <>
    struct is_string<std::basic_string<char>> : std::true_type
    {
    };

    template <>
    struct is_string<const char*> : std::true_type
    {
    };

    template <typename T>
    constexpr bool is_string_v = is_string<T>::value;

    template <typename Container>
    std::enable_if_t<is_container_v<Container> && !is_map_v<Container>
                     && !is_unordered_map_v<Container>,
                     void>
    print_container(const Container& container);

    template <typename Map>
    std::enable_if_t<is_map_v<Map> || is_unordered_map_v<Map>, void>
    print_container(const Map& map);

    extern std::mutex SYSDARFT_EXPORT_SYMBOL log_mutex;
    template <typename ParamType>
    void _log(const ParamType& param);
    void _log(const __uint128_t& param);
    template <typename ParamType, typename... Args>
    void _log(const ParamType& param, const Args&... args);
    /////////////////////////////////////////////////////////////////////////////////////////////

    // python print() like log() output
    template <typename... Args> void d_log(const Args&... args);

    typedef std::vector < std::pair<std::string, void*> > backtrace_info;

    backtrace_info SYSDARFT_EXPORT_SYMBOL obtain_stack_frame();
    std::string SYSDARFT_EXPORT_SYMBOL separate_before_slash(const std::string& input);

    extern std::atomic<bool> SYSDARFT_EXPORT_SYMBOL verbose;

    std::string get_verbose_info();

    inline void set_thread_name(const std::string& name) {
        pthread_setname_np(pthread_self(), name.c_str());
    }
}

#if !defined(__DEBUG__) || defined(__CLEAN_OUTPUT__)
#define log(...) ::debug::_log(__VA_ARGS__);
#else
#define log(...) ::debug::_log(__FILE__, ":", __LINE__, ":", __PRETTY_FUNCTION__, ": ", __VA_ARGS__);
#endif

/**
 * @brief SysdarftBaseError is a base error class to which all error classes are
 * derivative
 */
class SYSDARFT_EXPORT_SYMBOL SysdarftBaseError : public std::runtime_error
{
protected:
    int cur_errno; // system errno
public:
    /**
     * @brief SysdarftBaseError is not meant to be called by user, but
     * automatically invoked by derivatives.
     *
     * @param msg error message
     */
    explicit SysdarftBaseError(const std::string& msg);
};

#include "SysdarftDebug.inl"

#endif // DEBUG_H
