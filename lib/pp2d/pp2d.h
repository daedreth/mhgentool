/*  This file is part of pp2d
>   Copyright (C) 2017/2018 Bernardo Giordano
>
>   This program is free software: you can redistribute it and/or modify
>   it under the terms of the GNU General Public License as published by
>   the Free Software Foundation, either version 3 of the License, or
>   (at your option) any later version.
>
>   This program is distributed in the hope that it will be useful,
>   but WITHOUT ANY WARRANTY; without even the implied warranty of
>   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>   GNU General Public License for more details.
>
>   You should have received a copy of the GNU General Public License
>   along with this program.  If not, see <http://www.gnu.org/licenses/>.
>   See LICENSE for information.
> 
>   https://discord.gg/bGKEyfY
*/
 
/**
 * Plug & Play 2D
 * @file pp2d.c
 * @author Bernardo Giordano
 * @date 25 February 2018
 * @brief pp2d header
 */

#ifndef PP2D_H
#define PP2D_H

#include "lodepng.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <3ds.h>
#include <citro3d.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vshader_shbin.h"

/// Used to transfer the final rendered display to the framebuffer
#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

/// Used to convert the texture to 3DS tiled format
#define TEXTURE_TRANSFER_FLAGS(fmt) \
    (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(fmt) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

/**
 * @brief Creates a 8 byte RGBA color
 * @param r red component of the color
 * @param g green component of the color
 * @param b blue component of the color
 * @param a alpha component of the color
 */
#define RGBA8(r, g, b, a) ((((r)&0xFF)<<0) | (((g)&0xFF)<<8) | (((b)&0xFF)<<16) | (((a)&0xFF)<<24))

/**
 * @brief Creates a 8 byte ABGR color
 * @param a alpha component of the color
 * @param b blue component of the color
 * @param g green component of the color
 * @param r red component of the color
 */
#define ABGR8(a, b, g, r) ((((a)&0xFF)<<0) | (((b)&0xFF)<<8) | (((g)&0xFF)<<16) | (((r)&0xFF)<<24))

/**
 * @brief Converts a RGB565 color to RGBA8 color
 * @param rgb 565 to be converted
 * @param a alpha
 */
#define RGB565_TO_RGBA8(rgb, a) \
    (RGBA8(((rgb>>11)&0x1F)*0xFF/0x1F, ((rgb>>5)&0x3F)*0xFF/0x3F, (rgb&0x1F)*0xFF/0x1F, a&0xFF))
    
/**
 * @brief Converts a RGB565 color to ABGR8 color
 * @param rgb 565 to be converted
 * @param a alpha
 */
#define RGB565_TO_ABGR8(rgb, a) \
    (RGBA8(a&0xFF, (rgb&0x1F)*0xFF/0x1F, ((rgb>>5)&0x3F)*0xFF/0x3F, ((rgb>>11)&0x1F)*0xFF/0x1F))

#define PP2D_SCREEN_TOP_WIDTH 400
#define PP2D_SCREEN_BOTTOM_WIDTH 320
#define PP2D_SCREEN_HEIGHT 240
#define PP2D_DEFAULT_COLOR_BG ABGR8(255, 0, 0, 0)
#define PP2D_DEFAULT_COLOR_NEUTRAL RGBA8(255, 255, 255, 255)
#define PP2D_DEFAULT_DEPTH 0.5f
#define PP2D_MAX_VERTICES 12288

#ifndef PP2D_MAX_TEXTURES 
#define PP2D_MAX_TEXTURES 1
#endif

typedef enum {
    PP2D_FLIP_NONE,
    PP2D_FLIP_HORI,
    PP2D_FLIP_VERT,
    PP2D_FLIP_BOTH
} flipType_t;

typedef struct { 
    float x, y, z; 
    float u, v;
} vertex_s;

/// Draws queued verticies 
void pp2d_draw_arrays(void);

/**
 * @brief Draws a rectangle
 * @param x of the top left corner
 * @param y of the top left corner
 * @param width on the rectangle
 * @param height of the rectangle
 * @param color RGBA8 to fill the rectangle
 */
void pp2d_draw_rectangle(int x, int y, int width, int height, u32 color);

/**
 * @brief Prints a char pointer
 * @param x position to start drawing
 * @param y position to start drawing
 * @param scaleX multiplier for the text width
 * @param scaleY multiplier for the text height
 * @param color RGBA8 the text will be drawn
 * @param text to be printed on the screen
 */
void pp2d_draw_text(float x, float y, float scaleX, float scaleY, u32 color, const char* text);

/**
 * @brief Prints a char pointer in the middle of the target screen
 * @param target screen to draw the text to
 * @param y position to start drawing
 * @param scaleX multiplier for the text width
 * @param scaleY multiplier for the text height
 * @param color RGBA8 the text will be drawn
 * @param text to be printed on the screen
 */
void pp2d_draw_text_center(gfxScreen_t target, float y, float scaleX, float scaleY, u32 color, const char* text);

/**
 * @brief Prints a char pointer in the middle of the target screen
 * @param x position to start drawing
 * @param y position to start drawing
 * @param scaleX multiplier for the text width
 * @param scaleY multiplier for the text height
 * @param color RGBA8 the text will be drawn
 * @param wrapX wrap width
 * @param text to be printed on the screen
 */
void pp2d_draw_text_wrap(float x, float y, float scaleX, float scaleY, u32 color, float wrapX, const char* text);

/**
 * @brief Prints a char pointer with arguments
 * @param x position to start drawing
 * @param y position to start drawing
 * @param scaleX multiplier for the text width
 * @param scaleY multiplier for the text height
 * @param color RGBA8 the text will be drawn
 * @param text to be printed on the screen
 * @param ... arguments
 */
void pp2d_draw_textf(float x, float y, float scaleX, float scaleY, u32 color, const char* text, ...); 

/// Frees the pp2d environment
void pp2d_exit(void);

/**
 * @brief Starts a new frame on the specified screen
 * @param target GFX_TOP or GFX_BOTTOM
 * @param side GFX_LEFT or GFX_RIGHT
 */
void pp2d_frame_begin(gfxScreen_t target, gfx3dSide_t side);

/**
 * @brief Changes target screen to the specified target
 * @param target GFX_TOP or GFX_BOTTOM
 * @param side GFX_LEFT or GFX_RIGHT
 */
void pp2d_frame_draw_on(gfxScreen_t target, gfx3dSide_t side);

/// Ends a frame
void pp2d_frame_end(void);

/**
 * @brief Frees a texture
 * @param id of the texture to free
 */
void pp2d_free_texture(size_t id);

/**
 * @brief Calculates a char pointer height
 * @param text char pointer to calculate the height of
 * @param scaleX multiplier for the text width 
 * @param scaleY multiplier for the text height
 * @return height the text will have if rendered in the supplied conditions
 */
float pp2d_get_text_height(const char* text, float scaleX, float scaleY);

/**
 * @brief Calculates a char pointer height
 * @param text char pointer to calculate the height of
 * @param scaleX multiplier for the text width 
 * @param scaleY multiplier for the text height
 * @param wrapX wrap width
 * @return height the text will have if rendered in the supplied conditions
 */
float pp2d_get_text_height_wrap(const char* text, float scaleX, float scaleY, int wrapX);

/**
 * @brief Calculates width and height for a char pointer
 * @param width pointer to the width to return
 * @param height pointer to the height to return
 * @param scaleX multiplier for the text width 
 * @param scaleY multiplier for the text height
 * @param text to calculate dimensions of
 */
void pp2d_get_text_size(float* width, float* height, float scaleX, float scaleY, const char* text);

/**
 * @brief Calculates a char pointer width
 * @param text char pointer to calculate the width of
 * @param scaleX multiplier for the text width 
 * @param scaleY multiplier for the text height
 * @return width the text will have if rendered in the supplied conditions
 */
float pp2d_get_text_width(const char* text, float scaleX, float scaleY);

/**
 * @brief Inits the pp2d environment
 * @note This will trigger gfxInitDefault by default 
 */
void pp2d_init(void);

/**
 * @brief Loads a texture from a a buffer in memory
 * @param id of the texture 
 * @param buf buffer where the texture is stored
 * @param width of the texture
 * @param height of the 
 * @param fmt GX_TRANSFER_FORMAT to use
 */
void pp2d_load_texture_memory(size_t id, void* buf, u32 width, u32 height, GX_TRANSFER_FORMAT fmt);

/**
 * @brief Loads a texture from a png file
 * @param id of the texture 
 * @param path where the png file is located 
 */
void pp2d_load_texture_png(size_t id, const char* path);

/**
 * @brief Loads a texture from a buffer in memory
 * @param id of the texture
 * @param buf buffer where the png is stored
 * @param buf_size size of buffer
 */
void pp2d_load_texture_png_memory(size_t id, void* buf, size_t buf_size);

/**
 * @brief Enables 3D service
 * @param enable integer
 */
void pp2d_set_3D(bool enable);

/**
 * @brief Sets a background color for the specified screen
 * @param target GFX_TOP or GFX_BOTTOM
 * @param color ABGR8 which will be the background one
 */
void pp2d_set_screen_color(gfxScreen_t target, u32 color);

/**
 * @brief Sets filters to load texture with
 * @param magFilter GPU_NEAREST or GPU_LINEAR
 * @param minFilter GPU_NEAREST or GPU_LINEAR
 */
void pp2d_set_texture_filter(GPU_TEXTURE_FILTER_PARAM magFilter, GPU_TEXTURE_FILTER_PARAM minFilter);

/**
 * @brief Inits a portion of a texture to be drawn
 * @param id of the texture 
 * @param x position on the screen to draw the texture
 * @param y position on the screen to draw the texture
 * @param xbegin position to start drawing
 * @param ybegin position to start drawing
 * @param width to draw from the xbegin position 
 * @param height to draw from the ybegin position
 */
void pp2d_texture_select_part(size_t id, int x, int y, int xbegin, int ybegin, int width, int height);

/**
 * @brief Sets a color to blend textures with
 * @param color to modulate the texture
 */
void pp2d_texture_blend(u32 color);

/**
 * @brief Sets the depth of a texture
 * @param depth factor of the texture
 */
void pp2d_texture_depth(float depth);

/**
 * @brief Flips a texture
 * @param fliptype HORIZONTAL, VERTICAL or BOTH
 */
void pp2d_texture_flip(flipType_t fliptype);

/// Queues a texture
void pp2d_texture_queue(void);

/**
 * @brief Sets the position to draw the texture to
 * @param x position
 * @param y position
 */
void pp2d_texture_position(int x, int y);

/**
 * @brief Rotates a texture
 * @param angle in degrees to rotate the texture
 */
void pp2d_texture_rotate(float angle);

/**
 * @brief Scales a texture
 * @param scaleX width scale factor
 * @param scaleY height scale factor
 */
void pp2d_texture_scale(float scaleX, float scaleY);

#ifdef __cplusplus
}
#endif

#endif /* PP2D_H */