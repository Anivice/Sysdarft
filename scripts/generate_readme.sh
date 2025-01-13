#!/usr/bin/env bash

echo -n "Automatically generating README.md..."

SOURCE_DIR="$1"
cd "$SOURCE_DIR" || exit 1

LIST=$(git ls-files);
lines=0;
size=0;
for FILE in $LIST;
do
    if echo $FILE | grep JetBrains > /dev/null 2> /dev/null; then # a JetBrains font file
        continue; # skip
    fi

    if echo $FILE | grep sound > /dev/null 2> /dev/null; then # a sound file
        continue; # skip
    fi

    ((lines +=$(wc -l < "$FILE" )));
    ((size += $(du "$FILE" | awk '{print $1}')));
done;

echo "# Sysdarft""
![Lines of Code](https://img.shields.io/badge/ProjectLines-$lines-cyan)
![Size of Code](https://img.shields.io/badge/ProjectSize-"$size%20"K-yellow)
[![Automated CMake Test Workflow](https://github.com/Anivice/Sysdarft/actions/workflows/CMake_GitHub_Action.yml/badge.svg)](https://github.com/Anivice/Sysdarft/actions/workflows/CMake_GitHub_Action.yml)
" > "README.md"

cat "README.res.md" >> "README.md"

echo " done"
