/*******************************************************************************************************************************
 * Aquarium Theme Booting Animation for SmartFishJar
 * Resolution: 128x64, SSD1106/SH1106 OLED, Monochrome
 * Designed for v_disp_draw_booting_page() in md_display_api.c
 *******************************************************************************************************************************/

#include "MD_Display/md_display_api.h"

/*====================================================================
 * Configuration
 *===================================================================*/
#define BOOT_FISH_X_CENTER      62      /* Fish center X */
#define BOOT_FISH_Y             28      /* Fish Y position */
#define BOOT_FISH_SWING_AMP     18      /* Fish horizontal swing amplitude */
#define BOOT_AQUA_X             20      /* Aquarium frame X */
#define BOOT_AQUA_Y             10      /* Aquarium frame Y */
#define BOOT_AQUA_W             88      /* Aquarium frame width */
#define BOOT_AQUA_H             46      /* Aquarium frame height */
#define BOOT_ANIM_FRAME_MAX     8       /* Animation cycle frames */

/*====================================================================
 * Static State
 *===================================================================*/
static u8 s_boot_frame = 0U;

/*====================================================================
 * Helper: Draw a single pixel (wrapper for U8g2)
 *===================================================================*/
static void boot_draw_pixel(u8 x, u8 y)
{
    u8g2_DrawPixel(&u8g2, x, y);
}

/*====================================================================
 * Helper: Draw line (Bresenham)
 *===================================================================*/
static void boot_draw_line(u8 x1, u8 y1, u8 x2, u8 y2)
{
    s16 dx = (s16)abs((s16)x2 - (s16)x1);
    s16 dy = (s16)abs((s16)y2 - (s16)y1);
    s16 sx = (x1 < x2) ? 1 : -1;
    s16 sy = (y1 < y2) ? 1 : -1;
    s16 err = dx - dy;
    s16 x = (s16)x1;
    s16 y = (s16)y1;

    while(1)
    {
        boot_draw_pixel((u8)x, (u8)y);
        if(x == (s16)x2 && y == (s16)y2)
            break;
        s16 e2 = 2 * err;
        if(e2 > -dy)
        {
            err -= dy;
            x += sx;
        }
        if(e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

/*====================================================================
 * Helper: Draw circle outline
 *===================================================================*/
static void boot_draw_circle(u8 cx, u8 cy, u8 r)
{
    s16 x = (s16)r;
    s16 y = 0;
    s16 err = 0;

    while(x >= y)
    {
        boot_draw_pixel((u8)(cx + x), (u8)(cy + y));
        boot_draw_pixel((u8)(cx + y), (u8)(cy + x));
        boot_draw_pixel((u8)(cx - y), (u8)(cy + x));
        boot_draw_pixel((u8)(cx - x), (u8)(cy + y));
        boot_draw_pixel((u8)(cx - x), (u8)(cy - y));
        boot_draw_pixel((u8)(cx - y), (u8)(cy - x));
        boot_draw_pixel((u8)(cx + y), (u8)(cy - x));
        boot_draw_pixel((u8)(cx + x), (u8)(cy - y));

        if(err <= 0)
        {
            y++;
            err += 2 * y + 1;
        }
        if(err > 0)
        {
            x--;
            err -= 2 * x + 1;
        }
    }
}

/*====================================================================
 * Helper: Draw filled rectangle
 *===================================================================*/
static void boot_draw_rect_fill(u8 x, u8 y, u8 w, u8 h)
{
    u8g2_DrawBox(&u8g2, x, y, w, h);
}

/*====================================================================
 * Helper: Draw round-corner aquarium frame
 *===================================================================*/
static void boot_draw_aquarium_frame(void)
{
    u8 x = BOOT_AQUA_X;
    u8 y = BOOT_AQUA_Y;
    u8 w = BOOT_AQUA_W;
    u8 h = BOOT_AQUA_H;
    u8 r = 4;

    /* Top edge */
    boot_draw_line((u8)(x + r), y, (u8)(x + w - r - 1), y);
    /* Bottom edge */
    boot_draw_line((u8)(x + r), (u8)(y + h - 1), (u8)(x + w - r - 1), (u8)(y + h - 1));
    /* Left edge */
    boot_draw_line(x, (u8)(y + r), x, (u8)(y + h - r - 1));
    /* Right edge */
    boot_draw_line((u8)(x + w - 1), (u8)(y + r), (u8)(x + w - 1), (u8)(y + h - r - 1));

    /* Four corners */
    for(u8 i = 0; i < r; i++)
    {
        for(u8 j = 0; j < r; j++)
        {
            if((u16)(i * i + j * j) <= (u16)(r * r))
            {
                boot_draw_pixel((u8)(x + r - i), (u8)(y + r - j));
                boot_draw_pixel((u8)(x + w - r - 1 + i), (u8)(y + r - j));
                boot_draw_pixel((u8)(x + r - i), (u8)(y + h - r - 1 + j));
                boot_draw_pixel((u8)(x + w - r - 1 + i), (u8)(y + h - r - 1 + j));
            }
        }
    }

    /* Top rim line */
    boot_draw_line((u8)(x + 2), (u8)(y + 2), (u8)(x + w - 3), (u8)(y + 2));

    /* Sand at bottom */
    for(u8 sx = (u8)(x + 4); sx < (u8)(x + w - 4); sx += 2)
    {
        boot_draw_pixel(sx, (u8)(y + h - 6));
        if((sx % 4) == 0)
        {
            boot_draw_pixel((u8)(sx + 1), (u8)(y + h - 5));
        }
    }

    /* Seaweed (3 plants) */
    const u8 weed_x[3] = {35, 75, 90};
    for(u8 widx = 0; widx < 3; widx++)
    {
        for(u8 i = 0; i < 8; i++)
        {
            s16 wx = (s16)weed_x[widx] + (s16)(sinf((float)i * 0.8f) * 2.0f);
            boot_draw_pixel((u8)wx, (u8)(y + h - 6 - i));
            if((i % 2) == 0)
            {
                boot_draw_pixel((u8)(wx + 1), (u8)(y + h - 6 - i));
            }
        }
    }
}

/*====================================================================
 * Helper: Draw water wave
 *===================================================================*/
static void boot_draw_wave(u8 y_base, u8 phase, u8 width)
{
    u8 x_start = (u8)((OLED_WIDTH_PIXELS - width) / 2);
    for(u8 x = 0; x < width; x++)
    {
        float rad = (float)(x + phase) * 0.15f;
        s16 wave_y = (s16)y_base + (s16)(sinf(rad) * 2.0f);
        if(wave_y >= 0 && wave_y < OLED_HEIGHT_PIXELS)
        {
            boot_draw_pixel((u8)(x_start + x), (u8)wave_y);
            if((u8)(wave_y + 1) < OLED_HEIGHT_PIXELS)
            {
                boot_draw_pixel((u8)(x_start + x), (u8)(wave_y + 1));
            }
        }
    }
}

/*====================================================================
 * Helper: Draw small fish with animated tail
 * direction: 1=right, -1=left
 * tail_phase: 0..3 tail animation phase
 *===================================================================*/
static void boot_draw_fish(u8 x, u8 y, s8 direction, u8 tail_phase)
{
    const u8 body[6][6] = {
        {0, 0, 1, 1, 0, 0},
        {0, 1, 1, 1, 1, 0},
        {1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1},
        {0, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 0, 0},
    };

    u8 row, col;

    /* Draw body */
    for(row = 0; row < 6; row++)
    {
        for(col = 0; col < 6; col++)
        {
            if(body[row][col])
            {
                u8 px;
                if(direction < 0)
                    px = (u8)(x + (5 - col));
                else
                    px = (u8)(x + col);
                boot_draw_pixel(px, (u8)(y + row));
            }
        }
    }

    /* Eye (inverse color dot) */
    u8 eye_x = (direction > 0) ? (u8)(x + 4) : (u8)(x + 1);
    /* Erase body pixel for eye using XOR or simply draw background */
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawPixel(&u8g2, eye_x, (u8)(y + 2));
    u8g2_SetDrawColor(&u8g2, 1);

    /* Tail patterns [3 rows] x [phases] */
    const u8 tail_p0[3] = {1, 1, 1};           /* flat  */
    const u8 tail_p1[3] = {0, 1, 1};           /* up    */
    const u8 tail_p2[3] = {1, 1, 1};           /* flat  */
    const u8 tail_p3[3] = {1, 1, 0};           /* down  */
    const u8* tails[4] = {tail_p0, tail_p1, tail_p2, tail_p3};
    const u8* tail = tails[tail_phase % 4];

    s8 tail_base_x = (direction > 0) ? (s8)(x + 6) : (s8)(x - 1);

    for(u8 i = 0; i < 3; i++)
    {
        if(tail[i])
        {
            if(direction > 0)
            {
                boot_draw_pixel((u8)tail_base_x, (u8)(y + 1 + i));
            }
            else
            {
                boot_draw_pixel((u8)(tail_base_x - 1), (u8)(y + 1 + i));
            }
        }
    }

    /* Dorsal fin */
    boot_draw_pixel((u8)(x + 2), (u8)(y - 1));
    boot_draw_pixel((u8)(x + 3), (u8)(y - 1));
}

/*====================================================================
 * Helper: Draw rising bubbles
 *===================================================================*/
static void boot_draw_bubbles(u8 frame)
{
    const struct {
        u8 x;
        u8 base_y;
        u8 speed;
        u8 offset;
    } bubbles[5] = {
        {45, 48, 1, 0},
        {55, 50, 2, 5},
        {70, 49, 1, 10},
        {85, 51, 2, 15},
        {95, 48, 1, 20},
    };

    for(u8 i = 0; i < 5; i++)
    {
        u8 rise = (u8)((frame * bubbles[i].speed + bubbles[i].offset) % 25);
        s16 by = (s16)bubbles[i].base_y - (s16)rise;
        if(by < 15)
            continue;  /* Above water surface, skip */

        if((rise % 2) == 0)
        {
            boot_draw_circle(bubbles[i].x, (u8)by, 1);
        }
        else
        {
            boot_draw_pixel(bubbles[i].x, (u8)by);
            boot_draw_pixel((u8)(bubbles[i].x + 1), (u8)by);
            boot_draw_pixel(bubbles[i].x, (u8)(by + 1));
            boot_draw_pixel((u8)(bubbles[i].x + 1), (u8)(by + 1));
        }
    }
}

/*====================================================================
 * Helper: Draw "Booting" text with spinning dot
 *===================================================================*/
static void boot_draw_progress(u8 frame)
{
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawStr(&u8g2, 38, 56, "Booting");

    /* Spinning dot around text */
    const s8 dot_pos[8][2] = {
        {0, -2}, {1, -1}, {2, 0}, {1, 1},
        {0, 2},  {-1, 1}, {-2, 0}, {-1, -1}
    };
    u8 idx = frame % 8;
    u8 dx = (u8)(80 + dot_pos[idx][0]);
    u8 dy = (u8)(59 + dot_pos[idx][1]);
    boot_draw_pixel(dx, dy);
}

/*====================================================================
 * MAIN: Draw Booting Page with Aquarium Animation
 * Replace the existing v_disp_draw_booting_page() with this
 *===================================================================*/
static void v_disp_draw_booting_page(void)
{
    /* Clear screen */
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetDrawColor(&u8g2, 1);

    /* 1. Top inverted title bar */
    boot_draw_rect_fill(0, 0, OLED_WIDTH_PIXELS, 10);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tr);
    u8g2_DrawStr(&u8g2, 44, 7, "BOOTING");
    u8g2_SetDrawColor(&u8g2, 1);

    /* 2. Aquarium glass frame */
    boot_draw_aquarium_frame();

    /* 3. Water surface waves */
    u8 wave_phase = (u8)(s_boot_frame * 3);
    boot_draw_wave(18, wave_phase, 84);
    boot_draw_wave(22, (u8)(wave_phase + 10), 80);

    /* 4. Swimming fish */
    float fish_angle = (float)s_boot_frame * 0.5f;
    s16 fish_offset = (s16)(sinf(fish_angle) * (float)BOOT_FISH_SWING_AMP);
    u8 fish_x = (u8)((s16)BOOT_FISH_X_CENTER + fish_offset);
    s8 fish_dir = (cosf(fish_angle) > 0.0f) ? 1 : -1;
    boot_draw_fish(fish_x, BOOT_FISH_Y, fish_dir, s_boot_frame % 4);

    /* 5. Rising bubbles */
    boot_draw_bubbles(s_boot_frame);

    /* 6. Bottom progress indicator */
    boot_draw_progress(s_boot_frame);

    /* Advance animation frame */
    s_boot_frame++;
    if(s_boot_frame >= BOOT_ANIM_FRAME_MAX)
    {
        s_boot_frame = 0;
    }
}

/*====================================================================
 * USAGE: Call this function repeatedly in your display task loop.
 * The calling period (e.g. 100~200ms) controls animation speed.
 *===================================================================*/
