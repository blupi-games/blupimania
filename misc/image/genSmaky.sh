#!/bin/bash

INPUT_DIR="$1"
OUTPUT_DIR="$2"
ID="$3"
TEMP_DIR="$1/.out"
COLOR="#00ED18"

echo "This script in only an helper. It must not be used in the build"
echo "process because sometimes the resulting images are edited by hand."

if [ -z "$INPUT_DIR" ]; then
    echo "You must specify an input directory with *.image.png files"
    exit 1
fi
if [ -z "$OUTPUT_DIR" ]; then
    echo "You must specify an output directory for the *.smaky.png files"
    exit 2
fi

mkdir -p "$TEMP_DIR"

for img in "$INPUT_DIR"/*"$ID".image.png; do
    mogrify -path "$TEMP_DIR" -format png -fill "$COLOR" -opaque white "$img"
    img="$(basename "$img")"
    gmic -input "$TEMP_DIR/$img" blur_bloom 1,1,2,+,1,0 -output "$OUTPUT_DIR/${img/.image./.smaky.}"
done

rm -rf "$TEMP_DIR"
