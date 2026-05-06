# -*- coding: utf-8 -*-
"""
Aquarium Theme OLED Booting Animation Preview
Resolution: 128x64 pixels, monochrome display
"""

from PIL import Image, ImageDraw
import math
import os

OLED_W = 128
OLED_H = 64
SCALE = 4

BG_COLOR = 0       # Black background
FG_COLOR = 255     # White foreground

def create_oled_canvas():
    img = Image.new('L', (OLED_W * SCALE, OLED_H * SCALE), BG_COLOR)
    draw = ImageDraw.Draw(img)
    return img, draw

def draw_pixel(draw, x, y, color=FG_COLOR):
    if 0 <= x < OLED_W and 0 <= y < OLED_H:
        draw.rectangle(
            [x * SCALE, y * SCALE, (x + 1) * SCALE - 1, (y + 1) * SCALE - 1],
            fill=color
        )

def draw_line(draw, x1, y1, x2, y2, color=FG_COLOR):
    dx = abs(x2 - x1)
    dy = abs(y2 - y1)
    sx = 1 if x1 < x2 else -1
    sy = 1 if y1 < y2 else -1
    err = dx - dy
    
    while True:
        draw_pixel(draw, x1, y1, color)
        if x1 == x2 and y1 == y2:
            break
        e2 = 2 * err
        if e2 > -dy:
            err -= dy
            x1 += sx
        if e2 < dx:
            err += dx
            y1 += sy

def draw_circle(draw, cx, cy, r, color=FG_COLOR, fill=False):
    x = r
    y = 0
    err = 0
    
    while x >= y:
        if fill:
            for i in range(cx - x, cx + x + 1):
                draw_pixel(draw, i, cy + y, color)
                draw_pixel(draw, i, cy - y, color)
            for i in range(cx - y, cx + y + 1):
                draw_pixel(draw, i, cy + x, color)
                draw_pixel(draw, i, cy - x, color)
        else:
            draw_pixel(draw, cx + x, cy + y, color)
            draw_pixel(draw, cx + y, cy + x, color)
            draw_pixel(draw, cx - y, cy + x, color)
            draw_pixel(draw, cx - x, cy + y, color)
            draw_pixel(draw, cx - x, cy - y, color)
            draw_pixel(draw, cx - y, cy - x, color)
            draw_pixel(draw, cx + y, cy - x, color)
            draw_pixel(draw, cx + x, cy - y, color)
        
        if err <= 0:
            y += 1
            err += 2 * y + 1
        if err > 0:
            x -= 1
            err -= 2 * x + 1

def draw_rect(draw, x, y, w, h, color=FG_COLOR, fill=False):
    if fill:
        for i in range(x, x + w):
            for j in range(y, y + h):
                draw_pixel(draw, i, j, color)
    else:
        for i in range(x, x + w):
            draw_pixel(draw, i, y, color)
            draw_pixel(draw, i, y + h - 1, color)
        for j in range(y, y + h):
            draw_pixel(draw, x, j, color)
            draw_pixel(draw, x + w - 1, j, color)

def draw_round_rect(draw, x, y, w, h, r, color=FG_COLOR):
    draw_line(draw, x + r, y, x + w - r - 1, y, color)
    draw_line(draw, x + r, y + h - 1, x + w - r - 1, y + h - 1, color)
    draw_line(draw, x, y + r, x, y + h - r - 1, color)
    draw_line(draw, x + w - 1, y + r, x + w - 1, y + h - r - 1, color)
    for i in range(r):
        for j in range(r):
            if i * i + j * j <= r * r:
                draw_pixel(draw, x + r - i, y + r - j, color)
                draw_pixel(draw, x + w - r - 1 + i, y + r - j, color)
                draw_pixel(draw, x + r - i, y + h - r - 1 + j, color)
                draw_pixel(draw, x + w - r - 1 + i, y + h - r - 1 + j, color)

def draw_text_simple(draw, text, x, y, color=FG_COLOR):
    font = {
        'B': [[1,1,1,1,0],[1,0,0,0,1],[1,1,1,1,0],[1,0,0,0,1],[1,1,1,1,0],[1,0,0,0,1],[1,1,1,1,0]],
        'O': [[0,1,1,1,0],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0]],
        'T': [[1,1,1,1,1],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0]],
        'I': [[0,1,1,1,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,1,1,1,0]],
        'N': [[1,0,0,0,1],[1,1,0,0,1],[1,0,1,0,1],[1,0,0,1,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1]],
        'G': [[0,1,1,1,0],[1,0,0,0,1],[1,0,0,0,0],[1,0,1,1,1],[1,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0]],
        '.': [[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,1,0,0]],
        'F': [[1,1,1,1,1],[1,0,0,0,0],[1,1,1,1,0],[1,0,0,0,0],[1,0,0,0,0],[1,0,0,0,0],[1,0,0,0,0]],
        'S': [[0,1,1,1,1],[1,0,0,0,0],[1,0,0,0,0],[0,1,1,1,0],[0,0,0,0,1],[0,0,0,0,1],[1,1,1,1,0]],
        'H': [[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[1,1,1,1,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1]],
        'A': [[0,1,1,1,0],[1,0,0,0,1],[1,0,0,0,1],[1,1,1,1,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1]],
        'R': [[1,1,1,1,0],[1,0,0,0,1],[1,0,0,0,1],[1,1,1,1,0],[1,0,1,0,0],[1,0,0,1,0],[1,0,0,0,1]],
        'J': [[0,0,0,0,1],[0,0,0,0,1],[0,0,0,0,1],[0,0,0,0,1],[0,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0]],
        'L': [[1,0,0,0,0],[1,0,0,0,0],[1,0,0,0,0],[1,0,0,0,0],[1,0,0,0,0],[1,0,0,0,0],[1,1,1,1,1]],
        'K': [[1,0,0,0,1],[1,0,0,1,0],[1,0,1,0,0],[1,1,0,0,0],[1,0,1,0,0],[1,0,0,1,0],[1,0,0,0,1]],
        'E': [[1,1,1,1,1],[1,0,0,0,0],[1,0,0,0,0],[1,1,1,1,0],[1,0,0,0,0],[1,0,0,0,0],[1,1,1,1,1]],
        'M': [[1,0,0,0,1],[1,1,0,1,1],[1,0,1,0,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1]],
        'P': [[1,1,1,1,0],[1,0,0,0,1],[1,0,0,0,1],[1,1,1,1,0],[1,0,0,0,0],[1,0,0,0,0],[1,0,0,0,0]],
        'U': [[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0]],
        '1': [[0,0,1,0,0],[0,1,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,1,1,1,0]],
        '2': [[0,1,1,1,0],[1,0,0,0,1],[0,0,0,0,1],[0,0,0,1,0],[0,0,1,0,0],[0,1,0,0,0],[1,1,1,1,1]],
        '3': [[0,1,1,1,0],[1,0,0,0,1],[0,0,0,0,1],[0,0,1,1,0],[0,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0]],
        '4': [[0,0,0,1,0],[0,0,1,1,0],[0,1,0,1,0],[1,0,0,1,0],[1,1,1,1,1],[0,0,0,1,0],[0,0,0,1,0]],
        '5': [[1,1,1,1,1],[1,0,0,0,0],[1,1,1,1,0],[0,0,0,0,1],[0,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0]],
        '6': [[0,1,1,1,0],[1,0,0,0,0],[1,1,1,1,0],[1,0,0,0,1],[1,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0]],
        '7': [[1,1,1,1,1],[0,0,0,0,1],[0,0,0,1,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0],[0,0,1,0,0]],
        '8': [[0,1,1,1,0],[1,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0],[1,0,0,0,1],[1,0,0,0,1],[0,1,1,1,0]],
        '9': [[0,1,1,1,0],[1,0,0,0,1],[1,0,0,0,1],[0,1,1,1,1],[0,0,0,0,1],[0,0,0,0,1],[0,1,1,1,0]],
        '0': [[0,1,1,1,0],[1,0,0,1,1],[1,0,1,0,1],[1,1,0,0,1],[1,0,1,0,1],[1,0,0,1,1],[0,1,1,1,0]],
        '-': [[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[1,1,1,1,1],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0]],
        '/': [[0,0,0,0,1],[0,0,0,1,0],[0,0,1,0,0],[0,1,0,0,0],[0,1,0,0,0],[1,0,0,0,0],[1,0,0,0,0]],
        ' ': [[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0],[0,0,0,0,0]],
    }
    
    cx = x
    for ch in text:
        if ch in font:
            data = font[ch]
            for row in range(7):
                for col in range(5):
                    if data[row][col]:
                        draw_pixel(draw, cx + col, y + row, color)
        cx += 6

def draw_fish(draw, x, y, direction=1, tail_phase=0):
    body = [
        "  XX  ",
        " XXXX ",
        "XXXXXX",
        "XXXXXX",
        " XXXX ",
        "  XX  ",
    ]
    
    if direction < 0:
        for row_idx, row in enumerate(body):
            for col_idx, pixel in enumerate(row):
                if pixel == 'X':
                    draw_pixel(draw, x + (5 - col_idx), y + row_idx, FG_COLOR)
    else:
        for row_idx, row in enumerate(body):
            for col_idx, pixel in enumerate(row):
                if pixel == 'X':
                    draw_pixel(draw, x + col_idx, y + row_idx, FG_COLOR)
    
    eye_x = x + (4 if direction > 0 else 1)
    draw_pixel(draw, eye_x, y + 2, BG_COLOR)
    
    tail_patterns = [
        ["X", "X", "X"],
        [" X", "X", "X "],
        ["X", "X", "X"],
        ["X ", "X", " X"],
    ]
    
    tail = tail_patterns[tail_phase % 4]
    tail_base_x = x + (6 if direction > 0 else -1)
    
    if direction > 0:
        for i, t in enumerate(tail):
            for j, c in enumerate(t):
                if c == 'X':
                    draw_pixel(draw, tail_base_x + j, y + 1 + i, FG_COLOR)
    else:
        for i, t in enumerate(tail):
            for j, c in enumerate(t):
                if c == 'X':
                    draw_pixel(draw, tail_base_x - len(t) + j, y + 1 + i, FG_COLOR)
    
    fin_y = y - 1
    draw_pixel(draw, x + 2, fin_y, FG_COLOR)
    draw_pixel(draw, x + 3, fin_y, FG_COLOR)

def draw_wave(draw, y, phase, width=100):
    x_start = (OLED_W - width) // 2
    for x in range(width):
        wave_y = int(y + math.sin((x + phase) * 0.15) * 2)
        draw_pixel(draw, x_start + x, wave_y, FG_COLOR)
        draw_pixel(draw, x_start + x, wave_y + 1, FG_COLOR)

def draw_bubbles(draw, bubbles, frame):
    for bx, base_y, speed, offset in bubbles:
        rise = ((frame * speed + offset) % 25)
        by = base_y - rise
        if by < 15:
            continue
        
        if rise % 2 == 0:
            draw_circle(draw, bx, by, 1, FG_COLOR)
        else:
            draw_pixel(draw, bx, by, FG_COLOR)
            draw_pixel(draw, bx + 1, by, FG_COLOR)
            draw_pixel(draw, bx, by + 1, FG_COLOR)
            draw_pixel(draw, bx + 1, by + 1, FG_COLOR)

def draw_aquarium_frame(draw):
    draw_round_rect(draw, 20, 10, 88, 46, 4, FG_COLOR)
    draw_line(draw, 22, 12, 105, 12, FG_COLOR)
    
    sand_y = 50
    for x in range(24, 104, 2):
        draw_pixel(draw, x, sand_y, FG_COLOR)
        if x % 4 == 0:
            draw_pixel(draw, x + 1, sand_y + 1, FG_COLOR)
    
    for weed_x in [35, 75, 90]:
        for i in range(8):
            wx = weed_x + int(math.sin(i * 0.8) * 2)
            draw_pixel(draw, wx, 50 - i, FG_COLOR)
            if i % 2 == 0:
                draw_pixel(draw, wx + 1, 50 - i, FG_COLOR)

def draw_progress_dots(draw, frame):
    text_x = 38
    text_y = 56
    draw_text_simple(draw, "Booting", text_x, text_y, FG_COLOR)
    
    dot_x = text_x + 42
    dot_positions = [
        (0, -2), (1, -1), (2, 0), (1, 1), (0, 2), (-1, 1), (-2, 0), (-1, -1)
    ]
    
    for i, (dx, dy) in enumerate(dot_positions):
        if i == (frame % 8):
            draw_pixel(draw, dot_x + dx, text_y + 3 + dy, FG_COLOR)

def draw_frame(frame_idx, total_frames=8):
    img, draw = create_oled_canvas()
    
    # Top title bar (inverted)
    draw_rect(draw, 0, 0, OLED_W, 10, FG_COLOR, fill=True)
    draw_text_simple(draw, "BOOTING", 44, 2, BG_COLOR)
    
    # Aquarium frame
    draw_aquarium_frame(draw)
    
    # Water waves
    wave_phase = frame_idx * 3
    draw_wave(draw, 18, wave_phase, 84)
    draw_wave(draw, 22, wave_phase + 10, 80)
    
    # Swimming fish
    fish_x = 35 + int(math.sin(frame_idx * 0.5) * 15)
    fish_y = 28
    fish_dir = 1 if math.cos(frame_idx * 0.5) > 0 else -1
    draw_fish(draw, fish_x, fish_y, fish_dir, frame_idx % 4)
    
    # Bubbles
    bubbles = [
        (45, 48, 1, 0),
        (55, 50, 2, 5),
        (70, 49, 1, 10),
        (85, 51, 2, 15),
        (95, 48, 1, 20),
    ]
    draw_bubbles(draw, bubbles, frame_idx)
    
    # Progress indicator
    draw_progress_dots(draw, frame_idx)
    
    return img

def generate_preview():
    output_dir = os.path.dirname(os.path.abspath(__file__))
    
    frames = []
    total_frames = 8
    
    for i in range(total_frames):
        frame = draw_frame(i, total_frames)
        frames.append(frame)
        frame.save(os.path.join(output_dir, "booting_frame_{}.png".format(i)))
    
    # Create combined preview (4x2 grid)
    preview_w = OLED_W * SCALE * 4 + 20 * 5
    preview_h = OLED_H * SCALE * 2 + 20 * 3
    preview = Image.new('L', (preview_w, preview_h), 50)
    
    for i, frame in enumerate(frames):
        col = i % 4
        row = i // 4
        x = 20 + col * (OLED_W * SCALE + 20)
        y = 20 + row * (OLED_H * SCALE + 20)
        preview.paste(frame, (x, y))
    
    preview.save(os.path.join(output_dir, "booting_animation_preview.png"))
    
    # Vertical sequence
    doc_h = OLED_H * SCALE * total_frames + 10 * (total_frames + 1)
    doc_img = Image.new('L', (OLED_W * SCALE + 20, doc_h), 50)
    for i, frame in enumerate(frames):
        y = 10 + i * (OLED_H * SCALE + 10)
        doc_img.paste(frame, (10, y))
    doc_img.save(os.path.join(output_dir, "booting_animation_sequence.png"))
    
    print("Generated {} frames:".format(total_frames))
    for i in range(total_frames):
        print("  - booting_frame_{}.png".format(i))
    print("Preview saved to: booting_animation_preview.png")
    print("Sequence saved to: booting_animation_sequence.png")

if __name__ == "__main__":
    generate_preview()
