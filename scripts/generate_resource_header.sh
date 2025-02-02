#!/usr/bin/env bash

string="$1"
resource_header="$2"
IFS=' ' read -r -a LIST <<< "$string"

rm -rf "$resource_header"

for item in "${LIST[@]}"; do
    {
        echo "extern unsigned char $item [];"
        echo "extern unsigned int ""$item""_len;"
        echo "extern unsigned long long int ""$item""_original_len;"
    } >> "$resource_header"
done
