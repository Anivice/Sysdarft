#!/usr/bin/env bash

SOURCE_ROOT_DIR="$1"

# Get the directory of the current script
SCRIPT_DIR=$(dirname "$(realpath "$0")")

# Define color codes
GREEN='\033[0;32m'
BLUE='\033[0;34m'
BOLD='\033[1m'
REGULAR='\033[0m'
RED='\033[0;31m'
IFS=$'\n'

# auto install dependencies
if ! pip install markdown2 pdfkit 2> /dev/null; then
    echo -e "${RED}pip installation failed! Run pip install markdown2 pdfkit to see details.${REGULAR}"
    exit 1
fi

whereis_wkhtmltopdf=$(whereis wkhtmltopdf)

if ! wkhtmltopdf --help 2> /dev/null > /dev/null; then
    echo "wkhtmltopdf not installed! Automatic installation begins..."
    if [ -f /etc/debian_version ]; then
        # Debian/Ubuntu
        sudo apt update -y
        sudo apt install wkhtmltopdf -y
    elif [ -f /etc/arch-release ]; then
        # Arch Linux
        sudo pacman -Syu --noconfirm wkhtmltopdf
    elif [ -f /etc/fedora-release ]; then
        # Fedora
        sudo dnf install wkhtmltopdf -y
    else
        echo -e "${RED}Unsupported Linux distribution.${REGULAR}"
        exit 1
    fi
else
    echo "Requirement already satisfied: $whereis_wkhtmltopdf"
fi

# Function to generate PDF filename from a markdown file path
function generate_pdf_filename_from_path
{
    echo "$(basename "$1" .md)"".pdf"  # Replace .md with .pdf
}

# Function to calculate and print the current percentage
function current_percentage {
    local up="$1"
    local dr="$2"
    # Use shell arithmetic to avoid `bc`
    local pct=$(( (up * 100) / dr ))
    echo "$pct%"
}

# Find all markdown files under the source root directory
ALL_MARKDOWN_FILES=("$(find "$SOURCE_ROOT_DIR" -type f -name "*.md")")
FILE_COUNT=${#ALL_MARKDOWN_FILES[@]}
CURRENT_FILE=1

# Create output PDF directory
mkdir -p doc

# Process each markdown file
for FILE in "${ALL_MARKDOWN_FILES[@]}"; do
    echo -ne "${GREEN}[$(current_percentage $CURRENT_FILE $FILE_COUNT)]:\t${BOLD}Converting file: $FILE <"

    if ! "$SCRIPT_DIR/convert.py" "$FILE" "$PWD/doc/$(generate_pdf_filename_from_path "$FILE")"; then
        echo -e "${RED}Conversion failed!${REGULAR}"
        exit 1
    fi

    echo -e ">${REGULAR}"
    ((CURRENT_FILE++))  # Increment the file counter
done

echo -e "${BLUE}${BOLD}Build Completed!${REGULAR}"
