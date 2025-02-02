#!/usr/bin/env bash

echo "Building click-and-run executable for Sysdarft for Linux."

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

SYSDARFT_BIN="$1"
SYSDARFT_LIBS="$2"
TEMP_DIR="$3"
ARCH="$4"
IFS=' ' read -r -a SYSDARFT_LIBS <<< "$SYSDARFT_LIBS"

echo "Finding all requirements for $SYSDARFT_BIN..."

"$SCRIPT_DIR"/find_all_libs.sh "$SYSDARFT_BIN" > "$TEMP_DIR"/requirements

for item in "${SYSDARFT_LIBS[@]}"; do
    echo "Finding all requirements for $item..."
    "$SCRIPT_DIR"/find_all_libs.sh "$item" >> "$TEMP_DIR"/requirements
done

echo "Processing requirements..."
sort < "$TEMP_DIR"/requirements | uniq > "$TEMP_DIR"/requirements.sorted

echo "Setting up building hierarchy tree..."

rm -rf "$TEMP_DIR"/Sysdarft.AppDir

mkdir -p "$TEMP_DIR"/Sysdarft.AppDir/usr/bin
mkdir -p "$TEMP_DIR"/Sysdarft.AppDir/usr/lib
mkdir -p "$TEMP_DIR"/Sysdarft.AppDir/usr/share/icons/hicolor/128x128/apps
mkdir -p "$TEMP_DIR"/Sysdarft.AppDir/usr/share/applications
mkdir -p "$TEMP_DIR"/Sysdarft.AppDir/usr/share/doc

FILE_LIST=$(cat "$TEMP_DIR"/requirements.sorted)

echo "Copying files..."

for item in $FILE_LIST; do
    if echo "$item" | grep "\\.so" > /dev/null 2> /dev/null; then
        cp "$item" "$TEMP_DIR"/Sysdarft.AppDir/usr/lib
    else
        cp "$item" "$TEMP_DIR"/Sysdarft.AppDir/usr/bin
    fi
done

cp "$TEMP_DIR"/Sysdarft.pdf "$TEMP_DIR"/Sysdarft.AppDir/usr/share/doc
cp "$SCRIPT_DIR"/../resources/icon/sysdarft.svg "$TEMP_DIR"/Sysdarft.AppDir/usr/share/icons/hicolor/128x128/apps
cp "$SCRIPT_DIR"/../resources/icon/sysdarft.svg "$TEMP_DIR"/Sysdarft.AppDir/

echo "Generating scripting files..."

{
    echo "[Desktop Entry]"
    echo "Name=Sysdarft"
    echo "Exec=sysdarft-system"
    echo "Icon=sysdarft"
    echo "Type=Application"
    echo "Categories=Utility;"
} > "$TEMP_DIR"/Sysdarft.AppDir/usr/share/applications/Sysdarft.desktop

cp "$TEMP_DIR"/Sysdarft.AppDir/usr/share/applications/Sysdarft.desktop "$TEMP_DIR"/Sysdarft.AppDir/

{
    echo \
"#!/bin/bash
# AppRun

SCRIPT_DIR=\$(dirname \"\$(readlink -f \"\$0\")\")

\"\$SCRIPT_DIR/usr/bin/sysdarft-system\" \${@}
"
} > "$TEMP_DIR"/Sysdarft.AppDir/AppRun

chmod +x "$TEMP_DIR"/Sysdarft.AppDir/AppRun

cd "$TEMP_DIR" || exit 1

echo "Building AppImage..."

"$SCRIPT_DIR/../utils/appimagetool-$ARCH.AppImage" "$TEMP_DIR"/Sysdarft.AppDir
