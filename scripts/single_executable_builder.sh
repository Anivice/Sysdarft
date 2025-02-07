#!/usr/bin/env bash
set -o errexit

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

dynamic_loader=""

for item in $FILE_LIST; do
    if echo "$item" | grep "\\.so" > /dev/null 2> /dev/null; then
        cp "$item" "$TEMP_DIR"/Sysdarft.AppDir/usr/lib

        if echo "$item" | grep "ld-linux" > /dev/null 2> /dev/null; then
            dynamic_loader="\$SCRIPT_DIR/usr/lib/$(basename "$item")"
        fi
    else
        cp "$item" "$TEMP_DIR"/Sysdarft.AppDir/usr/bin/"$(basename "$item")".bin
        {
            echo \
"#!/usr/bin/env bash
# $item

\"$dynamic_loader\" --library-path \"\$SCRIPT_DIR/usr/lib\" \"\$SCRIPT_DIR/usr/bin/$(basename "$item").bin\" \${@}
exit \$?
"
        } > "$TEMP_DIR"/Sysdarft.AppDir/usr/bin/"$(basename "$item")"
        chmod +x "$TEMP_DIR"/Sysdarft.AppDir/usr/bin/"$(basename "$item")"
    fi
done

if [ -z "$dynamic_loader" ]; then
    echo "Dynamic loader absent from packed libraries!" >&2
    exit 1
else
    echo "Using loader $dynamic_loader..."
fi

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
"
file=\"\$1\"
if echo \"\$file\" | grep '\.sig' 2>/dev/null >/dev/null; then # Skip signature files
    exit 0
fi

echo \"Verifying \$file\"

if ! \$SCRIPT_DIR/usr/bin/openssl dgst -sha256 -verify \"\$SCRIPT_DIR/Public.pem\" -signature \"\${file}.sig\" \"\$file\" 2>/dev/null >/dev/null; then
    echo \"Checksum failed for \$file\"
else
    echo \"Checksum passed for \$file\"
fi
"
} > "$TEMP_DIR"/Sysdarft.AppDir/VerifyFile
chmod +x "$TEMP_DIR"/Sysdarft.AppDir/VerifyFile

{
    echo \
"#!/usr/bin/env bash
# AppRun
set -o errexit

verbose=0

for arg in \"\$@\"; do
    if [[ \"\$arg\" == \"-V\" || \"\$arg\" == \"--verbose\" ]]; then
        verbose=1
        break
    fi
done

if [ \"\$verbose\" -eq 1 ]; then
    echo \"Verbose mode enabled\"
fi

SCRIPT_DIR=\$(dirname \"\$(readlink -f \"\$0\")\")
export SCRIPT_DIR       # export for use in the subshell
export verbose          # export the presence of verbose flag

if [ \"\$verbose\" -eq 1 ]; then
    echo \"SCRIPT_DIR=\$SCRIPT_DIR\"
fi

for arg in \"\$@\"; do
    if [[ \"\$arg\" == \"--verify-file-integrity\" ]]; then
        \$SCRIPT_DIR/usr/bin/find \"\$SCRIPT_DIR/\" -type f ! -name \"\*.sig\" -exec \"\$SCRIPT_DIR\"/VerifyFile {} \;
        echo \"File integrity check completed\"
        exit 0
    fi
done

SYSROOT=\"\$SCRIPT_DIR\"
export SYSROOT
\$SCRIPT_DIR/usr/bin/sysdarft-system \${@}
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
