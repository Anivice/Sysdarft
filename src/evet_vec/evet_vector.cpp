#include <event_vector.h>
#include <cstdio>

#define MAX_VECTOR 1024
#define CACHE_SIZE (1024 * 256)
typedef int (*interruption_vec_t)(void *parameter_buffer, int parameter_length);
interruption_vec_t interruption_vector[MAX_VECTOR] { };
size_t interruption_vector_count = 0;

// Macro to define an interruption handler and register it in the vector
#define MASS_DEF_INTERRUPTION_HANDLER(interruption_name, parameter_buffer, parameter_length)    \
int interruption_name(void *parameter_buffer, int parameter_length);                            \
void register_##interruption_name() {                                                           \
    if (interruption_vector_count < MAX_VECTOR) {                                               \
        interruption_vector[interruption_vector_count++] = (interruption_name);                 \
    }                                                                                           \
}                                                                                               \
int interruption_name(void *parameter_buffer, int parameter_length)                             \

MASS_DEF_INTERRUPTION_HANDLER(sample_handler, parameter_buffer, parameter_length)
{
    printf("Handler invoked! Parameter count: %d\n", parameter_length);
    printf("                 Parameters:\n");
    for (int i = 0; i < parameter_length; i++) {
        printf("                                %d\n", static_cast <int*>(parameter_buffer)[i]);
    }

    return 0;
}

MASS_DEF_INTERRUPTION_HANDLER(keyboard_interruption, parameter_buffer, parameter_length)
{
    printf("Keyboard interruption! Parameter count: %d\n", parameter_length);
    printf("Parameters: ");
    for (int i = 0; i < parameter_length; i++) {
        printf("%d, ", static_cast <int*>(parameter_buffer)[i]);
    }
    printf("\n");
    fflush(stdout);
    return 0;
}

void initialize_interruption_handler()
{
    register_sample_handler();
    register_keyboard_interruption();
}

int call_interruption_handler(int interruption_number,
    void * interruption_buffer,
    unsigned int interruption_parameter_length)
{
    if (!(interruption_number >= 0 && interruption_number < MAX_VECTOR)) {
        return -1;
    }

    return interruption_vector[interruption_number](interruption_buffer, interruption_parameter_length);
}
