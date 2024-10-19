#!/usr/bin/env bash

echo -n "Automatically generating README.md..."

cd "$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )" || exit 0
cd .. || exit 0

LIST=$(git ls-tree -r --name-only HEAD);
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

" > "./README.md"

cat "scripts/res/README.md" >> "./README.md"

echo " done"
