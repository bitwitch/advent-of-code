import colorsys
import sys

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} filepath")
        sys.exit(1);

    filepath = sys.argv[1]
    with open(filepath, "r") as f:
        file_data = f.read()

    kinds = set()
    for c in file_data:
        if c != '.' and c != '\n':
            kinds.add(c)

    print(f"{len(kinds)} kinds of antenna")

    # NOTE(shaw): not 255, because dont want to wrap all the way back around
    # to same color as 0
    hue_step = 240.0 / len(kinds)

    colors = []
    for i, kind in enumerate(kinds):
        hue = (i * hue_step) / 255.0
        rgb = colorsys.hsv_to_rgb(hue, .92, .69)
        r = int(rgb[0] * 255)
        g = int(rgb[1] * 255)
        b = int(rgb[2] * 255)
        color = 0xFF000000 | b << 16 | g << 8 | r
        colors.append(color)
        print(hex(color))

    
