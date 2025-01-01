#include <cpu.h>
#include <boost/core/data.hpp>

void processor::__InstructionExecutorType__::mov(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    log("[PROCESSOR, ", timestamp, "]: MOV .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum = operand2.get<uint64_t>();
    operand1 = static_cast<uint64_t>(opnum);
}

void processor::__InstructionExecutorType__::xchg(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    log("[PROCESSOR, ", timestamp, "]: XCHG .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    // exchange
    operand1 = static_cast<uint64_t>(opnum2);
    operand2 = static_cast<uint64_t>(opnum1);
}

void processor::__InstructionExecutorType__::push(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum;
    auto operand = pop_target();
    log("[PROCESSOR, ", timestamp, "]: PUSH .",
        bcd_width_str(width), "bit ",
        operand.literal, "\n");

    opnum = operand.get<uint64_t>();

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: rlmode_push_stack<1>((char*)&opnum); break;
        case 0x16: rlmode_push_stack<2>((char*)&opnum); break;
        case 0x32: rlmode_push_stack<4>((char*)&opnum); break;
        case 0x64: rlmode_push_stack<8>((char*)&opnum); break;
        default: throw IllegalInstruction("Unknown width");
        }
    }
}

void processor::__InstructionExecutorType__::pop(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum;
    auto operand = pop_target();
    log("[PROCESSOR, ", timestamp, "]: POP .",
        bcd_width_str(width), "bit ",
        operand.literal, "\n");

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: rlmode_pop_stack<1>((char*)&opnum); break;
        case 0x16: rlmode_pop_stack<2>((char*)&opnum); break;
        case 0x32: rlmode_pop_stack<4>((char*)&opnum); break;
        case 0x64: rlmode_pop_stack<8>((char*)&opnum); break;
        default: throw IllegalInstruction("Unknown width");
        }
    }

    operand = static_cast<uint64_t>(opnum);
}

struct alignas(8)
pushall_data
{
    uint64_t FER0;
    uint64_t FER1;
    uint64_t FER2;
    uint64_t FER3;
    uint64_t FER4;
    uint64_t FER5;
    uint64_t FER6;
    uint64_t FER7;
    uint64_t FER8;
    uint64_t FER9;
    uint64_t FER10;
    uint64_t FER11;
    uint64_t FER12;
    uint64_t FER13;
    uint64_t FER14;
    uint64_t FER15;
    uint64_t FG;
    uint64_t DP;
    uint64_t SP;
    uint64_t ESP;
    long double XMM0;
    long double XMM1;
    long double XMM2;
    long double XMM3;
    long double XMM4;
    long double XMM5;
    long double XMM6;
    long double XMM7;
};

void processor::__InstructionExecutorType__::pushall(const __uint128_t timestamp)
{
    log("[PROCESSOR, ", timestamp, "]: PUSHALL\n");

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);

        pushall_data data = {
            .FER0 = CPU.Registers.FullyExtendedRegister0,
            .FER1 = CPU.Registers.FullyExtendedRegister1,
            .FER2 = CPU.Registers.FullyExtendedRegister2,
            .FER3 = CPU.Registers.FullyExtendedRegister3,
            .FER4 = CPU.Registers.FullyExtendedRegister4,
            .FER5 = CPU.Registers.FullyExtendedRegister5,
            .FER6 = CPU.Registers.FullyExtendedRegister6,
            .FER7 = CPU.Registers.FullyExtendedRegister7,
            .FER8 = CPU.Registers.FullyExtendedRegister8,
            .FER9 = CPU.Registers.FullyExtendedRegister9,
            .FER10 = CPU.Registers.FullyExtendedRegister10,
            .FER11 = CPU.Registers.FullyExtendedRegister11,
            .FER12 = CPU.Registers.FullyExtendedRegister12,
            .FER13 = CPU.Registers.FullyExtendedRegister13,
            .FER14 = CPU.Registers.FullyExtendedRegister14,
            .FER15 = CPU.Registers.FullyExtendedRegister15,
            .FG = *(uint64_t*)&CPU.Registers.FlagRegister,
            .DP = CPU.Registers.DataPointer,
            .SP = CPU.Registers.StackPointer,
            .ESP = CPU.Registers.ExtendedSegmentPointer,
            .XMM0 = CPU.Registers.XMM0,
            .XMM1 = CPU.Registers.XMM1,
            .XMM2 = CPU.Registers.XMM2,
            .XMM3 = CPU.Registers.XMM3,
            .XMM4 = CPU.Registers.XMM4,
            .XMM5 = CPU.Registers.XMM5,
            .XMM6 = CPU.Registers.XMM6,
            .XMM7 = CPU.Registers.XMM7,
        };

        rlmode_push_stack<sizeof(data)>((char*)&data);
    }
}

void processor::__InstructionExecutorType__::popall(const __uint128_t timestamp)
{
    log("[PROCESSOR, ", timestamp, "]: POPALL\n");

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        pushall_data data {};
        rlmode_pop_stack<sizeof(data)>((char*)&data);

        CPU.Registers.FullyExtendedRegister0 = data.FER0;
        CPU.Registers.FullyExtendedRegister1 = data.FER1;
        CPU.Registers.FullyExtendedRegister2 = data.FER2;
        CPU.Registers.FullyExtendedRegister3 = data.FER3;
        CPU.Registers.FullyExtendedRegister4 = data.FER4;
        CPU.Registers.FullyExtendedRegister5 = data.FER5;
        CPU.Registers.FullyExtendedRegister6 = data.FER6;
        CPU.Registers.FullyExtendedRegister7 = data.FER7;
        CPU.Registers.FullyExtendedRegister8 = data.FER8;
        CPU.Registers.FullyExtendedRegister9 = data.FER9;
        CPU.Registers.FullyExtendedRegister10 = data.FER10;
        CPU.Registers.FullyExtendedRegister11 = data.FER11;
        CPU.Registers.FullyExtendedRegister12 = data.FER12;
        CPU.Registers.FullyExtendedRegister13 = data.FER13;
        CPU.Registers.FullyExtendedRegister14 = data.FER14;
        CPU.Registers.FullyExtendedRegister15 = data.FER15;
        memcpy(&CPU.Registers.FlagRegister, &data.FG, sizeof(data.FG));
        CPU.Registers.DataPointer = data.DP;
        CPU.Registers.StackPointer = data.SP;
        CPU.Registers.ExtendedSegmentPointer = data.ESP;
        CPU.Registers.XMM0 = data.XMM0;
        CPU.Registers.XMM1 = data.XMM1;
        CPU.Registers.XMM2 = data.XMM2;
        CPU.Registers.XMM3 = data.XMM3;
        CPU.Registers.XMM4 = data.XMM4;
        CPU.Registers.XMM5 = data.XMM5;
        CPU.Registers.XMM6 = data.XMM6;
        CPU.Registers.XMM7 = data.XMM7;
    }
}

void processor::__InstructionExecutorType__::enter(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum;
    auto operand = pop_target();
    log("[PROCESSOR, ", timestamp, "]: ENTER .",
        bcd_width_str(width), "bit ",
        operand.literal, "\n");

    opnum = operand.get<uint64_t>();

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.CurrentProcedureStackPreservationSpace = opnum;
        CPU.Registers.StackPointer -= opnum;
    }
}

void processor::__InstructionExecutorType__::leave(const __uint128_t timestamp)
{
    log("[PROCESSOR, ", timestamp, "]: LEAVE\n");

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.StackPointer += CPU.Registers.CurrentProcedureStackPreservationSpace;
        CPU.Registers.CurrentProcedureStackPreservationSpace = 0;
    }
}

void processor::__InstructionExecutorType__::movs(const __uint128_t timestamp)
{
    log("[PROCESSOR, ", timestamp, "]: MOVS\n");

    uint64_t dest, src, count;

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        dest = CPU.Registers.FullyExtendedRegister0;
        src = CPU.Registers.FullyExtendedRegister1;
        count = CPU.Registers.FullyExtendedRegister2;
    }

    auto buffer = new char [count];
    CPU.read_memory(src, buffer, count);
    CPU.write_memory(dest, buffer, count);
    delete[] buffer;
}
