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

preload=""

for item in $FILE_LIST; do
    if echo "$item" | grep "\\.so" > /dev/null 2> /dev/null; then
        cp "$item" "$TEMP_DIR"/Sysdarft.AppDir/usr/lib
        preload="$preload:\$SCRIPT_DIR/usr/lib/$(basename "$item")"
    else
        cp "$item" "$TEMP_DIR"/Sysdarft.AppDir/usr/bin
    fi
done

# cp "$TEMP_DIR"/Sysdarft.pdf "$TEMP_DIR"/Sysdarft.AppDir/usr/share/doc
cp "$SCRIPT_DIR"/../resources/icon/icon.png "$TEMP_DIR"/Sysdarft.AppDir/usr/share/icons/hicolor/128x128/apps/sysdarft.png
cp "$SCRIPT_DIR"/../resources/icon/icon.png "$TEMP_DIR"/Sysdarft.AppDir/sysdarft.png

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

rm -rf /tmp/SysdarftChecksumStatus

verify_file() {
    file=\"\$1\"
    if echo \"\$file\" | grep '\.sig' 2>/dev/null >/dev/null; then # Skip signature files
        return
    fi

    if ! openssl dgst -sha256 -verify \"\$SCRIPT_DIR/Public.pem\" -signature \"\${file}.sig\" \"\$file\" 2>/dev/null >/dev/null; then
        echo \"Checksum failed for \$file\" >> /tmp/SysdarftChecksumStatus
    fi
}

export SCRIPT_DIR  # export for use in the subshell
export -f verify_file  # export the function so it can be used in -exec

find \"\$SCRIPT_DIR/\" -type f ! -name \"*.sig\" -exec bash -c 'verify_file \"\$0\"' {} \;

if [ -e /tmp/SysdarftChecksumStatus ]; then
    cat /tmp/SysdarftChecksumStatus
    rm -rf /tmp/SysdarftChecksumStatus
    exit 1
fi

LD_PRELOAD=\"\$SCRIPT_DIR/usr/lib/libSysdarft.so:\$SCRIPT_DIR/usr/lib/libSysdarftResources.so\" \"\$SCRIPT_DIR/usr/bin/sysdarft-system\" \${@}
"
} > "$TEMP_DIR"/Sysdarft.AppDir/AppRun

chmod +x "$TEMP_DIR"/Sysdarft.AppDir/AppRun

cd "$TEMP_DIR" || exit 1

echo "Generating verification checksum..."
openssl genrsa -out Private.pem 4096
openssl rsa -in Private.pem -pubout -out "$TEMP_DIR"/Sysdarft.AppDir/Public.pem
find "$TEMP_DIR"/Sysdarft.AppDir -type f -exec openssl dgst -sha256 -sign Private.pem -out {}.sig {} \;

echo "Building AppImage..."

"$SCRIPT_DIR/../utils/appimagetool-$ARCH.AppImage" "$TEMP_DIR"/Sysdarft.AppDir

mv "$TEMP_DIR"/Sysdarft-"$ARCH".AppImage "$TEMP_DIR"/sysdarft-system

echo "Build finished on " "$(date)"
