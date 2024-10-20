#!/usr/bin/env bash

echo -n "Automatically generating README.md..."

SOURCE_DIR="$1"

LIST=$(git ls-files --others --exclude-standard --cached "$SOURCE_DIR" && git diff --name-only --diff-filter=ADR "$SOURCE_DIR");
lines=0;
size=0;
for FILE in $LIST;
do
    ((lines +=$(wc -l < "$FILE" )));
    ((size += $(du "$FILE" | awk '{print $1}')));
done;

echo "# Sysdarft""
![Lines of Code](https://img.shields.io/badge/ProjectLines-$lines-cyan)
![Size of Code](https://img.shields.io/badge/ProjectSize-"$size%20"K-yellow)

> ⚠️ **Warning:** ⚠️ THIS README FILE IS AUTO GENERATED! ANY MODIFICATION WON'T BE PRESERVED! MODIFY [THIS FILE](scripts/res/README.md) INSTEAD!

" > "$SOURCE_DIR/README.md"

cat "$SOURCE_DIR/scripts/res/README.md" >> "$SOURCE_DIR/README.md"

echo " done"
