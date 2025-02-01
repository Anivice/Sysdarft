// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cstdlib>
#include <cxxabi.h>
#include <string>
#include <SysdarftDebug.h>

#if defined(__clang__)
static const char personality[] = "Clang/LLVM";
#elif defined(__ICC) || defined(__INTEL_COMPILER)
static const char personality[] = "Intel ICC/ICPC";
#elif defined(__GNUC__) || defined(__GNUG__)
static const char personality[] = "GNU GCC/G++";
#elif defined(_MSC_VER)
static const char personality[] = "Microsoft Visual Studio";
#else
static const char personality[] = "Unknown";
#endif

std::string debug::demangle(const char *mangled_name)
{
    int status = 0;
    const char *realname = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
    if (status != 0)
    {
        if (realname != nullptr) {
            free((void *)realname);
        }

        return mangled_name;
    }

    std::string ret = realname;
    free((void *)realname);
    return ret;
}
