#ifndef EVENT_VECTOR_H
#define EVENT_VECTOR_H

extern "C" {
    void initialize_interruption_handler();
    int call_interruption_handler(int interruption_number,
        void * interruption_parameter_list,
        unsigned int interruption_parameter_length);
}

#endif //EVENT_VECTOR_H
