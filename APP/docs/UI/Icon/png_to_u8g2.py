#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Convert PNG icons to U8g2 XBMP C arrays.
Assumes 16x16 RGBA PNGs.
"""

from PIL import Image
import os
import glob

ICON_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_FILE = os.path.join(ICON_DIR, "icon_bitmaps.h")
ALPHA_THRESHOLD = 128

# Mapping Chinese filenames to C-safe identifiers
NAME_MAP = {
    "\u706F\u5149": "light",
    "\u70ED\u7BA1\u7406": "heat",
    "\u6C34\u6CF5": "wpump",
    "\u6C27\u6C14\u6CF5": "o2pump",
    "\u7CFB\u7EDF\u8BBE\u7F6E": "setting",
    "\u91C7\u6837\u53C2\u6570": "adc",
}

def png_to_xbmp(path):
    """Convert a 16x16 RGBA PNG to U8g2 XBMP bytes (LSB first)."""
    img = Image.open(path).convert("RGBA")
    width, height = img.size
    if width != 16 or height != 16:
        raise ValueError("Expected 16x16, got {}x{}: {}".format(width, height, path))

    pixels = img.load()
    xbmp = bytearray()

    for y in range(height):
        row_bits = 0
        bit_pos = 0  # U8g2 XBMP reads bit0 as the leftmost pixel.
        for x in range(width):
            r, g, b, a = pixels[x, y]
            # Non-transparent pixels become lit (1) on monochrome OLED
            if a > ALPHA_THRESHOLD:
                pixel = 1
            else:
                pixel = 0

            if pixel:
                row_bits |= (1 << bit_pos)
            bit_pos += 1
            if bit_pos > 7:
                xbmp.append(row_bits)
                row_bits = 0
                bit_pos = 0
        # Append remaining bits for the row
        if bit_pos != 0:
            xbmp.append(row_bits)

    return bytes(xbmp)

def bytes_to_c_array(name, b):
    """Format bytes as a C static const array."""
    lines = ["static const unsigned char icon_{}_16x16[] U8X8_PROGMEM = {{".format(name)]
    hex_str = ", ".join("0x{:02X}".format(byte) for byte in b)
    lines.append("    {}".format(hex_str))
    lines.append("};")
    lines.append("")
    return "\n".join(lines)

def main():
    png_files = sorted(glob.glob(os.path.join(ICON_DIR, "*.png")))
    if not png_files:
        print("No PNG files found.")
        return

    header_parts = [
        "#ifndef ICON_BITMAPS_H",
        "#define ICON_BITMAPS_H",
        "",
        "/* Auto-generated from docs/UI/Icon/*.png */",
        "/* For use with u8g2_DrawXBMP() */",
        "/* Each icon is 16x16 pixels, 32 bytes (LSB first, bit0 is leftmost) */",
        "",
        '#include "u8g2.h"',
        "",
    ]

    for path in png_files:
        basename = os.path.splitext(os.path.basename(path))[0]
        c_name = NAME_MAP.get(basename, basename)
        xbmp = png_to_xbmp(path)
        header_parts.append(bytes_to_c_array(c_name, xbmp))

    header_parts.append("#endif /* ICON_BITMAPS_H */")
    header_parts.append("")

    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write("\n".join(header_parts))

    print("Generated: {}".format(OUTPUT_FILE))
    for path in png_files:
        basename = os.path.splitext(os.path.basename(path))[0]
        c_name = NAME_MAP.get(basename, basename)
        print("  [{}] -> icon_{}_16x16".format(basename, c_name))

if __name__ == "__main__":
    main()
