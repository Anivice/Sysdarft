/* RealTimeClock.h
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

#ifndef REALTIMECLOCK_H
#define REALTIMECLOCK_H

#define RTC_CURRENT_TIME  (0x70UL)
#define RTC_SET_INTERRUPT (0x71UL) /* [63-39] Reserved */
                                 /* [37-8] * 5000ns (0.005 ms) */
                                 /* [7-0] interruption number, <= 0x1F means disable interruption */

#include <SysdarftIOHub.h>
#include <WorkerThread.h>
#include <SysdarftCPU.h>

class SysdarftRealTimeClock final : public SysdarftExternalDeviceBaseClass
{
private:
    decltype(std::chrono::system_clock::now()) m_startTime;
    decltype(std::chrono::system_clock::now()) m_machineTime;
    SysdarftCPU & m_cpu;
    std::mutex m_mutex;
    WorkerThread update_time_worker;
    std::atomic < uint64_t > interruption_number = 0;
    std::atomic < uint64_t > interruption_scale = 0;

    void update_time(std::atomic < bool > & running);

public:
    explicit SysdarftRealTimeClock(SysdarftCPU & _instance);
    ~SysdarftRealTimeClock() override { update_time_worker.stop(); }
    bool request_read(uint64_t port) override;
    bool request_write(uint64_t port) override;
};

#endif //REALTIMECLOCK_H
