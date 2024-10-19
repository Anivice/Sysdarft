#!/usr/bin/env bash

RESOURCE_FILE_LIST_H="$1"
RESOURCE_FILE_LIST_CPP="$2"
RESOURCE_FILE_DIR="$3"
PROJECT_ROOT_DIR="$4"
PROJECT_RESOURCE_FILE_LIST_FILE="$5"
BUILD_LOCK="$6"
IFS=$'\n'

if [ -f "$BUILD_LOCK" ]; then
    echo "Another process is already compiling the files. If you believe this is an error, delete $BUILD_LOCK"
    exit 0
fi

touch "$BUILD_LOCK"

# Initialize definitions
FILE_NAME_DEFINITION=()
FILE_CONTENT_VEC_DEF=()
FILE_CNT_VEC_LEN_DEF=()
FILE_COUNT=0

function process_file_name() {
    echo "$1" | sed 's/\//_/g'
}

function generate_definition() {
    local BASENAME=""
    BASENAME="${1//[.-]/_}"
    local FILE_NAME="${2//[\/]/_}"
    FILE_NAME_DEFINITION+=("$FILE_NAME")
    FILE_CONTENT_VEC_DEF+=("${BASENAME}")
    FILE_CNT_VEC_LEN_DEF+=("${BASENAME}_len")

    ((FILE_COUNT+=1))
}

function append_to_file() {
    local CONTENT_LIST=("$@")
    for CONTENT in "${CONTENT_LIST[@]}"; do
        [ -n "$CONTENT" ] && echo "$CONTENT"
    done
}

echo '[xxdCompiler]: Initializing resource_file_list.h/resource_file_list.cpp ...'
echo -n > "$RESOURCE_FILE_DIR/resource_file_cpp_code_list.txt"

cd "$PROJECT_ROOT_DIR" || exit 1

# Compile all resource files
FILELIST=$(cat $PROJECT_RESOURCE_FILE_LIST_FILE)
for FILE in $FILELIST; do
    BASENAME="$(process_file_name "$FILE")"
    xxd -i "$FILE" > "$RESOURCE_FILE_DIR/xxd_${BASENAME}.xxd"
    generate_definition "$BASENAME" "$FILE"
    echo "$RESOURCE_FILE_DIR/xxd_${BASENAME}.cpp" >> $RESOURCE_FILE_DIR/resource_file_cpp_code_list.txt
done

# Write the resource_file_list.cpp
{
    echo "/******************************************************************************"
    echo ' *          This file is automatically generated by the build system.         *'
    echo ' *                 !!! YOUR MODIFICATION WILL NOT BE SAVED !!!                *'
    echo ' * !! DO NOT DIRECTLY MODIFY THIS FILE !! INSTEAD, MODIFY THE BUILD SCRIPT !! *'
    echo ' *****************************************************************************/'
    echo
    echo '#include <vector>'
    echo '#include <debug.h>'
    echo
    echo "// Resource file content vector references:"
    cat "$RESOURCE_FILE_DIR/xxd_"*".xxd"
    echo
    echo 'std::vector < const char * > resource_file_list;                  // Resource file list'
    echo 'std::vector < unsigned char * > resource_file_content_vector;     // Resource file content vector'
    echo 'std::vector < unsigned int > resource_file_content_length_vector; // Resource file content length vector'
    echo
    echo 'void __initialization_for_resource_file_vectors__()'
    echo '{'
    # shellcheck disable=SC2028
    echo '    sysdarft_log::log(sysdarft_log::LOG_NORMAL, "Initializing resource reference vectors...\n");'
    echo
    for FILENAME_DEF in "${FILE_NAME_DEFINITION[@]}"; do
            echo "    resource_file_list.emplace_back(\"${FILENAME_DEF/\//_}\");"
    done
    echo
    for FILEVEC_DEF in "${FILE_CONTENT_VEC_DEF[@]}"; do
            echo "    resource_file_content_vector.emplace_back($FILEVEC_DEF);"
    done
    echo
    for FILEVEC_LEN_DEF in "${FILE_CNT_VEC_LEN_DEF[@]}"; do
            echo "    resource_file_content_length_vector.emplace_back($FILEVEC_LEN_DEF);"
    done
    echo
    # shellcheck disable=SC2028
    echo '    sysdarft_log::log(sysdarft_log::LOG_NORMAL, "Initialization complete\n");'
    echo '}'
    echo
} > "$RESOURCE_FILE_LIST_CPP"

echo '[xxdCompiler]: Initialization for resource_file_list.cpp completed.'

# Write the resource_file_list.h
{
    echo "/******************************************************************************"
    echo ' *          This file is automatically generated by the build system.         *'
    echo ' *                 !!! YOUR MODIFICATION WILL NOT BE SAVED !!!                *'
    echo ' * !! DO NOT DIRECTLY MODIFY THIS FILE !! INSTEAD, MODIFY THE BUILD SCRIPT !! *'
    echo ' *****************************************************************************/'
    echo
    echo '#ifndef SYSDARFT_RESOURCE_FILE_LIST_H'
    echo '#define SYSDARFT_RESOURCE_FILE_LIST_H'
    echo
    echo '#include <vector>'
    echo
    echo 'extern std::vector < const char * > resource_file_list;'
    echo 'extern std::vector < unsigned char * > resource_file_content_vector;'
    echo 'extern std::vector < unsigned int > resource_file_content_length_vector;'
    echo "const unsigned int resource_file_count = $FILE_COUNT;"
    echo 'void __initialization_for_resource_file_vectors__();'
    echo
    echo '#endif // SYSDARFT_RESOURCE_FILE_LIST_H'
} > "$RESOURCE_FILE_LIST_H"

echo '[xxdCompiler]: Initialization for resource_file_list.h completed.'

rm "$BUILD_LOCK"
