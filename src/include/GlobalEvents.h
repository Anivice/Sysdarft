/* GlobalEvents.h
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
