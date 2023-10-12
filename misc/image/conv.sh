#!/bin/bash

INPUT_DIR="$1"
OUTPUT_DIR="$2"
TEMP_DIR="$1/.out"

# Table de correspondance
pastel=(\
    -fill '#FFFFFF' -opaque '#FFFFFF' \
    -fill '#FFFF69' -opaque '#FFFF00' \
    -fill '#FFCE49' -opaque '#FFCC40' \
    -fill '#FF9191' -opaque '#FF0000' \
    -fill '#CDCDCD' -opaque '#DCDCDC' \
    -fill '#B4B4B4' -opaque '#BEBEBE' \
    -fill '#96FFFF' -opaque '#00FFFF' \
    -fill '#B9B7FF' -opaque '#0000FF' \
    -fill '#ACFFAC' -opaque '#00FF00' \
    -fill '#8DCD8D' -opaque '#00CD00' \
    -fill '#E9B2FF' -opaque '#E0A1FF' \
    -fill '#FF98FA' -opaque '#FF00FF' \
    -fill '#F2AE8C' -opaque '#DB9561' \
    -fill '#CD8585' -opaque '#B96B34' \
    -fill '#A7D3FF' -opaque '#A9D8FF')

sombre=(\
    -fill '#D0D0D0' -opaque '#FFFFFF' \
    -fill '#D3D300' -opaque '#FFFF00' \
    -fill '#CBA233' -opaque '#FFCC40' \
    -fill '#B90000' -opaque '#FF0000' \
    -fill '#A0A0A0' -opaque '#DCDCDC' \
    -fill '#787878' -opaque '#BEBEBE' \
    -fill '#00B7B7' -opaque '#00FFFF' \
    -fill '#0300CB' -opaque '#0000FF' \
    -fill '#00D500' -opaque '#00FF00' \
    -fill '#00A300' -opaque '#00CD00' \
    -fill '#BA85D3' -opaque '#E0A1FF' \
    -fill '#BB00B8' -opaque '#FF00FF' \
    -fill '#B16B36' -opaque '#DB9561' \
    -fill '#8D3B00' -opaque '#B96B34' \
    -fill '#8DB4D5' -opaque '#A9D8FF')

rose=(\
    -fill '#FFFFFF' -opaque '#FFFFFF' \
    -fill '#FFFF00' -opaque '#FFFF00' \
    -fill '#FFD86E' -opaque '#FFCC40' \
    -fill '#FF00B7' -opaque '#FF0000' \
    -fill '#DCC7C7' -opaque '#DCDCDC' \
    -fill '#BEADAD' -opaque '#BEBEBE' \
    -fill '#FFEEFF' -opaque '#00FFFF' \
    -fill '#EB85FF' -opaque '#0000FF' \
    -fill '#00FF00' -opaque '#00FF00' \
    -fill '#00CD00' -opaque '#00CD00' \
    -fill '#F4DAFF' -opaque '#E0A1FF' \
    -fill '#FFA6FE' -opaque '#FF00FF' \
    -fill '#D39769' -opaque '#DB9561' \
    -fill '#D16F0D' -opaque '#B96B34' \
    -fill '#D8B1FF' -opaque '#A9D8FF')

bleute=(\
    -fill '#FFFFFF' -opaque '#FFFFFF' \
    -fill '#E9FC3F' -opaque '#FFFF00' \
    -fill '#EDC75E' -opaque '#FFCC40' \
    -fill '#ED007B' -opaque '#FF0000' \
    -fill '#C6C6DC' -opaque '#DCDCDC' \
    -fill '#A8A5BE' -opaque '#BEBEBE' \
    -fill '#B9DDFF' -opaque '#00FFFF' \
    -fill '#8883D4' -opaque '#0000FF' \
    -fill '#00FFD5' -opaque '#00FF00' \
    -fill '#00CDB2' -opaque '#00CD00' \
    -fill '#D0A1FF' -opaque '#E0A1FF' \
    -fill '#B600ED' -opaque '#FF00FF' \
    -fill '#BE9A72' -opaque '#DB9561' \
    -fill '#C55A1C' -opaque '#B96B34' \
    -fill '#ABAFFF' -opaque '#A9D8FF')

mkdir -p "$TEMP_DIR"

for img in "$INPUT_DIR"/blupix*.color.png; do
    img="$(basename "$img")"
    out="${img/.color/}"

    cp "$INPUT_DIR/$img"               "$TEMP_DIR/0.$out"
    cp "$INPUT_DIR/${img/color/smaky}" "$TEMP_DIR/1.$out"

    num=$(echo "$out" | grep -o '[0-9]\+')
    if [ "$num" -le 20 ]; then
        convert "$INPUT_DIR/$img" "${pastel[@]}" "$TEMP_DIR/2.$out"
        convert "$INPUT_DIR/$img" "${sombre[@]}" "$TEMP_DIR/3.$out"
        convert "$INPUT_DIR/$img" "${rose[@]}"   "$TEMP_DIR/4.$out"
        convert "$INPUT_DIR/$img" "${bleute[@]}" "$TEMP_DIR/5.$out"
    fi

    webp="$OUTPUT_DIR/${out/.png/.webp}"
    rm -f "$webp"
    ffmpeg -framerate 1 -i "$TEMP_DIR/%d.$out" -c:v libwebp -lossless 1 -loop 0 "$webp"
    rm -f "$TEMP_DIR/0.$out" "$TEMP_DIR/1.$out" "$TEMP_DIR/2.$out" "$TEMP_DIR/3.$out" "$TEMP_DIR/4.$out" "$TEMP_DIR/5.$out"
done

rm -rf "$TEMP_DIR"
