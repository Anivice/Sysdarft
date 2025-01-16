#include <SysdarftMain.h>

std::vector < uint8_t >
RemoteDebugServer::
compile_conditional_expression_to_byte_code(const std::string & conditional_expression)
{
    return { };
}

bool
RemoteDebugServer::
is_condition_met(std::vector < uint8_t > condition_expression_byte_code)
{
    if (condition_expression_byte_code.empty()) {
        return true;
    }

    return false;
}

std::vector < uint8_t >
RemoteDebugServer::
compile_action_from_expression_to_byte_code(const std::string & action_expression)
{
    return { };
}
