#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::in(__uint128_t, WidthAndOperandsType & Operands)
{
    const auto & port = Operands.second[0].get_val();
    auto && buffer = SysdarftIOHub::ins(port);
    const auto & data = buffer.pop<uint64_t>();
    Operands.second[1].set_val(data);
}

void SysdarftCPUInstructionExecutor::out(__uint128_t, WidthAndOperandsType & Operands)
{
    const auto & port = Operands.second[0].get_val();
    const auto & data = Operands.second[1].get_val();

    ControllerDataStream buffer;
    buffer.push(data);
    SysdarftIOHub::outs(port, buffer);
}

void SysdarftCPUInstructionExecutor::ins(__uint128_t, WidthAndOperandsType & Operands)
{
    const auto DB = SysdarftRegister::load<DataBaseType>();
    const auto DP = SysdarftRegister::load<DataPointerType>();
    const auto CX = SysdarftRegister::load<FullyExtendedRegisterType, 0>();
    const auto & port = Operands.second[0].get_val();

    auto & buffer = SysdarftIOHub::ins(port);
    if (buffer.getSize() != CX) {
        throw SysdarftDeviceIOError("IO data length mismatch");
    }
    std::vector<uint8_t> wbuf;
    wbuf.resize(buffer.getSize());

    SysdarftCPUMemoryAccess::write_memory(DB + DP, (char*)wbuf.data(), CX);
    buffer.push(wbuf);
}

void SysdarftCPUInstructionExecutor::outs(__uint128_t, WidthAndOperandsType & Operands)
{
    ControllerDataStream buffer;
    const auto DB = SysdarftRegister::load<DataBaseType>();
    const auto DP = SysdarftRegister::load<DataPointerType>();
    const auto CX = SysdarftRegister::load<FullyExtendedRegisterType, 0>();
    const auto & port = Operands.second[0].get_val();

    std::vector<uint8_t> wbuf;
    wbuf.resize(CX);

    SysdarftCPUMemoryAccess::read_memory(DB + DP, (char*)wbuf.data(), CX);
    buffer.insert(wbuf);
    SysdarftIOHub::outs(port, buffer);
}
