#include <instance.h>
#include <cpu.h>

class __sysdarft_instance_initialization__ {
protected:
    processor CPU;
public:
    __sysdarft_instance_initialization__()
    {
        GlobalEventProcessor.install_instance(SYSDARFT_PROCESSOR_INSTANCE, &CPU,
            SYSDARFT_PROCESSOR_START, &processor::start_triggering);
        GlobalEventProcessor.install_instance(SYSDARFT_PROCESSOR_INSTANCE, &CPU,
            SYSDARFT_PROCESSOR_STOP, &processor::stop_triggering);
        GlobalEventProcessor.install_instance(SYSDARFT_PROCESSOR_INSTANCE, &CPU,
            SYSDARFT_PROCESSOR_COLLABORATION, &processor::collaboration);
    }
} __sysdarft_instance_initialization_instance__;
