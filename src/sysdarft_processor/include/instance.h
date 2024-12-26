#ifndef INSTANCE_H
#define INSTANCE_H

#include <global.h>
#include <array>

#define SYSDARFT_PROCESSOR_INSTANCE    "SysdarftProcessor"
#define SYSDARFT_PROCESSOR_PUSH_CODE   "PushCode"
#define SYSDARFT_PROCESSOR_START       "Start"
#define SYSDARFT_PROCESSOR_STOP        "Stop"
#define SYSDARFT_PROCESSOR_HALT        "Halt"
#define SYSDARFT_PROCESSOR_RESET       "Reset"
#define SYSDARFT_PROCESSOR_INTERRUPT   "Interrupt"

#define SYSDARFT_PROCESSOR_COLLABORATION "Collaboration"

typedef std::array<uint8_t, 4096> code_block_t;

inline void sysdarft_start() {
    GlobalEventProcessor(SYSDARFT_PROCESSOR_INSTANCE, SYSDARFT_PROCESSOR_START)();
}

inline void sysdarft_stop() {
    GlobalEventProcessor(SYSDARFT_PROCESSOR_INSTANCE, SYSDARFT_PROCESSOR_STOP)();
}

inline void sysdarft_collaboration() {
    GlobalEventProcessor(SYSDARFT_PROCESSOR_INSTANCE, SYSDARFT_PROCESSOR_COLLABORATION)();
}

#endif //INSTANCE_H
