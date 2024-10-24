#include <csignal>
#include <cstdio>
#include <event_vector.h>
#include <pygame_keys.h>
#include <unistd.h>
#include <debug.h>
#include <atomic>
#include <cstdint>

static_assert(sizeof(void*) == 8, "You have to ba an 64bit Operating System in order for the interruption parameters to work!");

#define MAX_VECTOR 32
typedef int (*interruption_vec_t)(uint64_t);
interruption_vec_t interruption_vector[MAX_VECTOR] { };
size_t interruption_vector_count = 0;

// Macro to define an interruption handler and register it in the vector
#define MASS_DEF_INTERRUPTION_HANDLER(interruption_name, parameter)                 \
int interruption_name(uint64_t parameter);                                          \
void register_##interruption_name() {                                               \
    if (interruption_vector_count < MAX_VECTOR) {                                   \
        interruption_vector[interruption_vector_count++] = (interruption_name);     \
    }                                                                               \
}                                                                                   \
                                                                                    \
int interruption_name(uint64_t parameter)                                           \


#define convert_to_parameter(num)       (*(uint64_t*)(&(num)))
#define convert_to_data_ptr(parameter)  ((void *)(*(uint64_t*)(&parameter)))

//                     Which function to call  Information for caller  Parameters 1-8, 7 bits each
// interruption data: [4]<interruption number> [4]<interruption flags> [7]<1.....................8>
#define get_interruption_number(data)   (convert_to_parameter(data) & 0x0F)
#define get_interruption_flags(data)    (convert_to_parameter(data) & 0xF0)
#define get_parameter_at(data, offset)  ((uint8_t)((convert_to_parameter(data) & (0x7F00 << (7 * offset))) >> (7 * offset + 8)))
#define get_parameter1(data)            get_parameter_at(data, 0)
#define get_parameter2(data)            get_parameter_at(data, 1)
#define get_parameter3(data)            get_parameter_at(data, 2)
#define get_parameter4(data)            get_parameter_at(data, 3)
#define get_parameter5(data)            get_parameter_at(data, 4)
#define get_parameter6(data)            get_parameter_at(data, 5)
#define get_parameter7(data)            get_parameter_at(data, 6)
#define get_parameter8(data)            get_parameter_at(data, 7)

MASS_DEF_INTERRUPTION_HANDLER(sample_handler, parameter)
{
    printf("Handler invoked! Parameter count: %lu\n", *(uint64_t*)(&parameter));
    fflush(stdout);
    return 0;
}

// Function to map key constants to their ASCII values
char key_to_ascii(int key)
{
    if (key >= KEY_A && key <= KEY_Z) {
        return 'A' + (key - KEY_A);
    }

    if (key >= KEY_0 && key <= KEY_9) {
        return '0' + (key - KEY_0);
    }

    switch (key) {
        case KEY_EXCLAMATION:   return '!';
        case KEY_AT:            return '@';
        case KEY_HASH:          return '#';
        case KEY_DOLLAR:        return '$';
        case KEY_PERCENT:       return '%';
        case KEY_CARET:         return '^';
        case KEY_AMPERSAND:     return '&';
        case KEY_ASTERISK:      return '*';
        case KEY_LEFT_PAREN:    return '(';
        case KEY_RIGHT_PAREN:   return ')';
        case KEY_UNDERSCORE:    return '_';
        case KEY_MINUS:         return '-';
        case KEY_PLUS:          return '+';
        case KEY_EQUAL:         return '=';
        case KEY_LESS_THAN:     return '<';
        case KEY_COMMA:         return ',';
        case KEY_PERIOD:        return '.';
        case KEY_GREATER_THAN:  return '>';
        case KEY_COLON:         return ':';
        case KEY_SEMICOLON:     return ';';
        case KEY_DOUBLE_QUOTE:  return '"';
        case KEY_SINGLE_QUOTE:  return '\'';
        case KEY_PIPE:          return '|';
        case KEY_BACKSLASH:     return '\\';
        case KEY_QUESTION:      return '?';
        case KEY_SLASH:         return '/';
        case KEY_SPACE:         return ' ';
        case KEY_BACKQUOTE:     return '`';
        case KEY_TILDE:         return '~';
        default:                return 0;
    }
}

MASS_DEF_INTERRUPTION_HANDLER(keyboard_interruption, parameter)
{
    std::cout << (unsigned)get_parameter1(parameter) << " " << std::flush;
    return 0;
}

void general_interruption_handler(int signal, siginfo_t* info, void*)
{
    // Hardware interruptions
    if (signal == SIGUSR1)
    {
        const auto data = convert_to_parameter(info->si_value.sival_ptr);

        uint8_t interruption_number = 0, interruption_flags = 0;
        interruption_number = data & 0x0F;
        interruption_flags  = data & 0xF0;

        if (interruption_number > interruption_vector_count) {
            return;
        }

        int ret = interruption_vector[interruption_number](data);
    }

    // Reserved
    if (signal == SIGUSR2)
    {
    }
}

void initialize_interruption_handler()
{
    register_sample_handler();
    register_keyboard_interruption();

    // Set up the signal handler to handle SIGUSR1 signal
    struct sigaction action;

    action.sa_sigaction = general_interruption_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &action, nullptr) == -1)
    {
        perror("Initialization failed!");
        exit(1);
        // throw sysdarft_error_t(sysdarft_error_t::SIGNAL_SIGACTION_REGISTRATION_FAILED);
    }
}

int call_interruption_handler(
    int target_pid,
    int interruption_number,
    int flags,
    void * interruption_buffer,
    unsigned int interruption_parameter_length)
{
    if (!(interruption_number >= 0 && interruption_number < MAX_VECTOR)) {
        return -1;
    }

    if (interruption_parameter_length > 8) {
        return -1;
    }

    uint64_t parameter_data = 0;
    int offset = 0;
    for (int i = 0; i < interruption_parameter_length; i++)
    {
        uint8_t current_parameter = static_cast<int*>(interruption_buffer)[i];

        // discard the highest bit
        current_parameter <<= 1;
        current_parameter >>= 1;

        // create a temporary cache, with the lowest 7 bit being the parameter
        uint64_t tmp = current_parameter;

        // move the 7 bit to upper place
        tmp <<= offset;

        // update the offset
        offset += 7;

        // store the current packed parameter
        parameter_data |= tmp;
    }

    uint8_t header = 0;
    header |= (interruption_number & 0x0F); // interruption number
    header |= (flags & 0xF0); // interruption flags

    parameter_data <<= 8; // move up 8 bits (4 for int num, 4 for flags)
    parameter_data |= header; // pack header into the data

    sigval sig_value{};
    sig_value.sival_ptr = convert_to_data_ptr(parameter_data);

    if (sigqueue(target_pid, SIGUSR1, sig_value) == -1)
    {
        perror("[INTERRUPT] Failed to send interruption call!");
        return -1;
    }

    return 0;
}
