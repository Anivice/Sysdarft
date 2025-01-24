/* SysdarftCPU.h
*
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <memory>
#include <EncodingDecoding.h>
#include <SysdarftRegister.h>
#include <SysdarftInstructionExec.h>
#include <WorkerThread.h>

class MultipleCPUInstanceCreation final : public SysdarftBaseError
{
public:
    explicit MultipleCPUInstanceCreation() :
        SysdarftBaseError("Trying to create multiple CPU instances!") { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftCPU final : public SysdarftCPUInstructionExecutor {
private:
    __uint128_t timestamp;

public:
    explicit SysdarftCPU(uint64_t memory,
        const std::vector < uint8_t > & bios,
        const std::string & hdd,
        const std::string & fda,
        const std::string & fdb);
    ~SysdarftCPU() override { SysdarftCursesUI::cleanup(); }

    void set_abort_next() { do_abort_int = true; }
    void system_hlt() { SystemHalted = true; }
    void Boot();

    SysdarftCPU & operator = (const SysdarftCPU &) = delete;
    [[nodiscard]] uint64_t SystemTotalMemory() const { return TotalMemory; }

    template <typename DeviceType, typename... Args,
              typename = std::enable_if_t<std::is_base_of_v<SysdarftExternalDeviceBaseClass, DeviceType>>>
    void add_device(Args &...args)
    {
        device_list.emplace_back(std::make_unique<DeviceType>(args...));
    }

    explicit operator bool() const
    {
        return !SystemHalted;
    }
};

#endif //CPU_H
