#!/usr/bin/env bash
# find_all_libs.sh - Recursively find all shared libraries required by an executable
#
# Usage:
#   ./find_all_libs.sh <executable>
#
# The script uses ldd on the given executable and then recursively
# processes each library dependency. It prints out the full path of each library.

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <executable>"
    exit 1
fi

TARGET="$1"

if [ ! -x "$TARGET" ]; then
    echo "Error: File '$TARGET' does not exist or is not executable."
    exit 1
fi

# Declare an associative array to track already processed libraries.
declare -A seen

function process_lib() {
    local file="$1"

    # If file already processed, return.
    if [[ ${seen["$file"]} ]]; then
        return
    fi

    # Mark this file as seen.
    seen["$file"]=1

    # Only process files that exist.
    if [ ! -e "$file" ]; then
        return
    fi

    # Print the library file.
    echo "$file"

    # Use ldd to list the dependencies.
    # The output lines are typically in one of these forms:
    #   linux-vdso.so.1 =>  (0x00007fff5b1b9000)
    #   libm.so.6 => /lib64/libm.so.6 (0x00007f4bd78ab000)
    #   /lib64/ld-linux-x86-64.so.2 (0x00007f4bd7a9f000)
    ldd "$file" 2>/dev/null | while read -r line; do
        # Use awk to extract the path after '=>'
        lib_path=$(echo "$line" | awk '{
            for (i=1; i<=NF; i++) {
                if ($i == "=>") {
                    print $(i+1);
                    exit;
                }
            }
        }')

        # If nothing was printed (i.e. no "=>" found), extract the first token.
        if [ -z "$lib_path" ]; then
            lib_path=$(echo "$line" | awk '{print $1}')
        fi

        # Skip lines where lib_path is empty.
        if [ -z "$lib_path" ]; then
            continue
        fi

        # Only process absolute paths.
        if [[ "$lib_path" =~ ^/ ]]; then
            process_lib "$lib_path"
        fi
    done
}

# Start by processing the target executable.
process_lib "$TARGET" | sort | uniq
