#ifndef GLOBAL_EVENT_H
#define GLOBAL_EVENT_H

#include <SysdarftMessageMap.h>
#include <SysdarftDebug.h>

extern SYSDARFT_EXPORT_SYMBOL SysdarftMessageMap GlobalEventProcessor;

#define _g_method_install(instance_name, instance, method_name, method) \
    GlobalEventProcessor.install_instance(                              \
                instance_name, &instance,                               \
                method_name, &decltype(instance)::method)

#endif //GLOBAL_EVENT_H
