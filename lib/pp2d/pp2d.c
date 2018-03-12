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
 * @brief pp2d implementation
 */

#include "pp2d.h"

// shader
static DVLB_s* vshader_dvlb;
static shaderProgram_s program;
static int uLoc_projection;

// targets
static C3D_RenderTarget* topLeft;
static C3D_RenderTarget* topRight;
static C3D_RenderTarget* bot;

// projection matrices
static C3D_Mtx projectionTopLeft;
static C3D_Mtx projectionTopRight;
static C3D_Mtx projectionBot;

// text data
static C3D_Tex* glyphSheets;
static float s_textScale;
static int lastSheet = -1;

// vbo buffer and positions
static struct {
    size_t cur;
    size_t old;
    vertex_s* vbo;
} vertexData;

// texture buffer
static struct {
    C3D_Tex tex;
    u32 width;
    u32 height;
    bool allocated;
} textures[PP2D_MAX_TEXTURES];

static struct {
    size_t id;
    int x;
    int y;
    int xbegin;
    int ybegin;
    int width;
    int height;
    u32 color;
    flipType_t fliptype;
    float scaleX;
    float scaleY;
    float angle;
    float depth;
    bool initialized;
} pp2dBuffer;

static u32 prevColor;
static size_t prevSpritesheet;
static bool renderedText;
static bool renderedRectangle;
static bool renderedTexture;

// texture filters
static struct {
    GPU_TEXTURE_FILTER_PARAM magFilter;
    GPU_TEXTURE_FILTER_PARAM minFilter;
} textureFilters;

static void pp2d_add_text_vertex(float vx, float vy, float vz, float tx, float ty);
static void pp2d_draw_unprocessed_queue(void);
static void pp2d_get_text_size_internal(float* width, float* height, float scaleX, float scaleY, int wrapX, const char* text);
static void pp2d_set_rendered_flags(bool texture, bool text, bool rectangle);
static void pp2d_set_text_color(u32 color);

static void pp2d_add_text_vertex(float vx, float vy, float vz, float tx, float ty)
{
    vertex_s* vtx = &vertexData.vbo[vertexData.cur++];
    vtx->x = vx;
    vtx->y = vy;
    vtx->z = vz;
    vtx->u = tx;
    vtx->v = ty;
}

void pp2d_draw_arrays(void)
{
    C3D_DrawArrays(GPU_TRIANGLES, vertexData.old, vertexData.cur - vertexData.old);
    vertexData.old = vertexData.cur;
}

void pp2d_draw_rectangle(int x, int y, int width, int height, u32 color)
{
    pp2d_draw_unprocessed_queue();

    if (vertexData.cur + 6 > PP2D_MAX_VERTICES)
    {
        return;
    }

    if (color != prevColor || renderedText || renderedTexture)
    {
        prevColor = color;
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvSrc(env, C3D_Both, GPU_CONSTANT, GPU_CONSTANT, 0);
        C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_INTERPOLATE);
        C3D_TexEnvColor(env, color);
    }

    pp2d_add_text_vertex(        x,          y, PP2D_DEFAULT_DEPTH, 0, 0);
    pp2d_add_text_vertex(        x, y + height, PP2D_DEFAULT_DEPTH, 0, 0);
    pp2d_add_text_vertex(x + width,          y, PP2D_DEFAULT_DEPTH, 0, 0);
    pp2d_add_text_vertex(x + width,          y, PP2D_DEFAULT_DEPTH, 0, 0);
    pp2d_add_text_vertex(        x, y + height, PP2D_DEFAULT_DEPTH, 0, 0);
    pp2d_add_text_vertex(x + width, y + height, PP2D_DEFAULT_DEPTH, 0, 0);
    pp2d_draw_arrays();

    pp2d_set_rendered_flags(false, false, true);
}

void pp2d_draw_text(float x, float y, float scaleX, float scaleY, u32 color, const char* text)
{
    pp2d_draw_text_wrap(x, y, scaleX, scaleY, color, -1, text);
}

void pp2d_draw_text_center(gfxScreen_t target, float y, float scaleX, float scaleY, u32 color, const char* text)
{
    float width = pp2d_get_text_width(text, scaleX, scaleY);
    float x = ((target == GFX_TOP ? PP2D_SCREEN_TOP_WIDTH : PP2D_SCREEN_BOTTOM_WIDTH) - width) / 2;
    pp2d_draw_text(x, y, scaleX, scaleY, color, text);
}

void pp2d_draw_text_wrap(float x, float y, float scaleX, float scaleY, u32 color, float wrapX, const char* text)
{
    if (text == NULL)
    {
        return;
    }

    pp2d_draw_unprocessed_queue();

    ssize_t  units;
    uint32_t code;
    const uint8_t* p = (const uint8_t*)text;
    float firstX = x;
    
    scaleX *= s_textScale;
    scaleY *= s_textScale;
    
    do
    {
        if (!*p) 
        {
            break;
        }
        
        units = decode_utf8(&code, p);
        if (units == -1)
        {
            break;
        }
        p += units;
        
        if (code == '\n' || (wrapX != -1 && x + scaleX * fontGetCharWidthInfo(fontGlyphIndexFromCodePoint(code))->charWidth >= firstX + wrapX))
        {
            x = firstX;
            y += scaleY*fontGetInfo()->lineFeed;
            p -= code == '\n' ? 0 : 1;
        }
        else if (code > 0)
        {
            if (vertexData.cur + 6 > PP2D_MAX_VERTICES)  
            {
                break;
            }
            
            int glyphIdx = fontGlyphIndexFromCodePoint(code);
            fontGlyphPos_s data;
            fontCalcGlyphPos(&data, glyphIdx, GLYPH_POS_CALC_VTXCOORD, scaleX, scaleY);

            if (data.sheetIndex != lastSheet)
            {
                lastSheet = data.sheetIndex;
                C3D_TexBind(0, &glyphSheets[lastSheet]);
            }

            if (color != prevColor || renderedRectangle || renderedTexture)
            {
                prevColor = color;
                pp2d_set_text_color(color);
            }

            pp2d_add_text_vertex(x+data.vtxcoord.left,  y+data.vtxcoord.top,    PP2D_DEFAULT_DEPTH, data.texcoord.left,  data.texcoord.top);
            pp2d_add_text_vertex(x+data.vtxcoord.left,  y+data.vtxcoord.bottom, PP2D_DEFAULT_DEPTH, data.texcoord.left,  data.texcoord.bottom);
            pp2d_add_text_vertex(x+data.vtxcoord.right, y+data.vtxcoord.top,    PP2D_DEFAULT_DEPTH, data.texcoord.right, data.texcoord.top);
            pp2d_add_text_vertex(x+data.vtxcoord.right, y+data.vtxcoord.top,    PP2D_DEFAULT_DEPTH, data.texcoord.right, data.texcoord.top);
            pp2d_add_text_vertex(x+data.vtxcoord.left,  y+data.vtxcoord.bottom, PP2D_DEFAULT_DEPTH, data.texcoord.left,  data.texcoord.bottom);
            pp2d_add_text_vertex(x+data.vtxcoord.right, y+data.vtxcoord.bottom, PP2D_DEFAULT_DEPTH, data.texcoord.right, data.texcoord.bottom);
            pp2d_draw_arrays();

            x += data.xAdvance;
        }
    } while (code > 0);

    pp2d_set_rendered_flags(false, true, false);
}

void pp2d_draw_textf(float x, float y, float scaleX, float scaleY, u32 color, const char* text, ...) 
{
    char buffer[256];
    va_list args;
    va_start(args, text);
    vsnprintf(buffer, 256, text, args);
    pp2d_draw_text(x, y, scaleX, scaleY, color, buffer);
    va_end(args);
}

static void pp2d_draw_unprocessed_queue(void)
{
    if (vertexData.cur != vertexData.old)
    {
        pp2d_draw_arrays();
    }
}

void pp2d_exit(void)
{
    for (size_t id = 0; id < PP2D_MAX_TEXTURES; id++)
    {
        pp2d_free_texture(id);
    }
    
    linearFree(vertexData.vbo);
    free(glyphSheets);
    
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
    
    C3D_Fini();
    gfxExit();
}

void pp2d_frame_begin(gfxScreen_t target, gfx3dSide_t side)
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    vertexData.cur = 0;
    vertexData.old = 0;
    pp2d_frame_draw_on(target, side);
}

void pp2d_frame_draw_on(gfxScreen_t target, gfx3dSide_t side)
{
    pp2d_draw_unprocessed_queue();
    
    if (target == GFX_TOP)
    {
        C3D_FrameDrawOn(side == GFX_LEFT ? topLeft : topRight);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, side == GFX_LEFT ? &projectionTopLeft : &projectionTopRight);
    } 
    else
    {
        C3D_FrameDrawOn(bot);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projectionBot);
    }
}

void pp2d_frame_end(void)
{
    pp2d_draw_unprocessed_queue();
    C3D_FrameEnd(0);
}

void pp2d_free_texture(size_t id)
{
    if (id >= PP2D_MAX_TEXTURES)
    {
        return;
    }
    
    if (!textures[id].allocated)
    {
        return;
    }
    
    C3D_TexDelete(&textures[id].tex);
    textures[id].width = 0;
    textures[id].height = 0;
    textures[id].allocated = false;
}

float pp2d_get_text_height(const char* text, float scaleX, float scaleY)
{
    float height;
    pp2d_get_text_size_internal(NULL, &height, scaleX, scaleY, -1, text);
    return height;
}

float pp2d_get_text_height_wrap(const char* text, float scaleX, float scaleY, int wrapX)
{
    float height;
    pp2d_get_text_size_internal(NULL, &height, scaleX, scaleY, wrapX, text);
    return height;
}

void pp2d_get_text_size(float* width, float* height, float scaleX, float scaleY, const char* text)
{
    pp2d_get_text_size_internal(width, height, scaleX, scaleY, -1, text);
}

static void pp2d_get_text_size_internal(float* width, float* height, float scaleX, float scaleY, int wrapX, const char* text)
{
    float maxW = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    
    ssize_t  units;
    uint32_t code;
    float x = 0;
    float firstX = x;
    const uint8_t* p = (const uint8_t*)text;
    
    scaleX *= s_textScale;
    scaleY *= s_textScale;
    
    do
    {
        if (!*p)
        {
            break;
        }
        
        units = decode_utf8(&code, p);
        if (units == -1)
        {
            break;
        }
        p += units;
        
        if (code == '\n' || (wrapX != -1 && x + scaleX * fontGetCharWidthInfo(fontGlyphIndexFromCodePoint(code))->charWidth >= firstX + wrapX))
        {
            x = firstX;
            h += scaleY*fontGetInfo()->lineFeed;
            p -= code == '\n' ? 0 : 1;
            if (w > maxW)
            {
                maxW = w;
            }
            w = 0.f;
        }
        else if (code > 0)
        {
            float len = (scaleX * fontGetCharWidthInfo(fontGlyphIndexFromCodePoint(code))->charWidth);
            w += len;
            x += len;
        }
    } while (code > 0);
    
    if (width)
    {
        *width = w > maxW ? w : maxW;
    }
    
    if (height)
    {
        h += scaleY*fontGetInfo()->lineFeed;
        *height = h;
    }
}

float pp2d_get_text_width(const char* text, float scaleX, float scaleY)
{
    float width;
    pp2d_get_text_size_internal(&width, NULL, scaleX, scaleY, -1, text);
    return width;
}

void pp2d_init(void)
{
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    
    topLeft = C3D_RenderTargetCreate(PP2D_SCREEN_HEIGHT, PP2D_SCREEN_TOP_WIDTH, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(topLeft, C3D_CLEAR_ALL, PP2D_DEFAULT_COLOR_BG, 0);
    C3D_RenderTargetSetOutput(topLeft, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    
    topRight = C3D_RenderTargetCreate(PP2D_SCREEN_HEIGHT, PP2D_SCREEN_TOP_WIDTH, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(topRight, C3D_CLEAR_ALL, PP2D_DEFAULT_COLOR_BG, 0);
    C3D_RenderTargetSetOutput(topRight, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);
    
    bot = C3D_RenderTargetCreate(PP2D_SCREEN_HEIGHT, PP2D_SCREEN_BOTTOM_WIDTH, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetClear(bot, C3D_CLEAR_ALL, PP2D_DEFAULT_COLOR_BG, 0);
    C3D_RenderTargetSetOutput(bot, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    
    pp2d_set_texture_filter(GPU_NEAREST, GPU_NEAREST);

    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    C3D_BindProgram(&program);
    uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2);

    Mtx_OrthoTilt(&projectionTopLeft, 0, PP2D_SCREEN_TOP_WIDTH, PP2D_SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, true);
    Mtx_OrthoTilt(&projectionTopRight, 0, PP2D_SCREEN_TOP_WIDTH, PP2D_SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, true);
    Mtx_OrthoTilt(&projectionBot, 0, PP2D_SCREEN_BOTTOM_WIDTH, PP2D_SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f, true);
    
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

    fontEnsureMapped();
    TGLP_s* glyphInfo = fontGetGlyphInfo();
    glyphSheets = malloc(sizeof(C3D_Tex)*glyphInfo->nSheets);
    for (int i = 0; i < glyphInfo->nSheets; i++)
    {
        C3D_Tex* tex = &glyphSheets[i];
        tex->data = fontGetGlyphSheetTex(i);
        tex->fmt = glyphInfo->sheetFmt;
        tex->size = glyphInfo->sheetSize;
        tex->width = glyphInfo->sheetWidth;
        tex->height = glyphInfo->sheetHeight;
        tex->param = GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)
            | GPU_TEXTURE_WRAP_S(GPU_CLAMP_TO_EDGE) | GPU_TEXTURE_WRAP_T(GPU_CLAMP_TO_EDGE);
        tex->border = 0;
        tex->lodParam = 0;
    }

    charWidthInfo_s* cwi = fontGetCharWidthInfo(fontGlyphIndexFromCodePoint(0x3042));
    s_textScale = 20.0f / (cwi->glyphWidth); // 20 is glyphWidth in J machines

    vertexData.vbo = (vertex_s*)linearAlloc(sizeof(vertex_s)*PP2D_MAX_VERTICES);
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vertexData.vbo, sizeof(vertex_s), 2, 0x10);

    prevColor = 0;
    prevSpritesheet = PP2D_MAX_TEXTURES;
    renderedText = false;
}

void pp2d_load_texture_memory(size_t id, void* buf, u32 width, u32 height, GX_TRANSFER_FORMAT fmt)
{
    GSPGPU_FlushDataCache(buf, width * height * 4);
    C3D_TexInit(&textures[id].tex, (u16)width, (u16)height, GPU_RGBA8);
    C3D_SafeDisplayTransfer((u32*)buf, GX_BUFFER_DIM(width, height), (u32*)textures[id].tex.data, GX_BUFFER_DIM(width, height), TEXTURE_TRANSFER_FLAGS(fmt));
    gspWaitForPPF();
    C3D_TexSetFilter(&textures[id].tex, textureFilters.magFilter, textureFilters.minFilter);
    C3D_TexFlush(&textures[id].tex);

    textures[id].allocated = true;
    textures[id].width = width;
    textures[id].height = height;
}

void pp2d_load_texture_png(size_t id, const char* path)
{
    if (id >= PP2D_MAX_TEXTURES)
    {
        return;
    }
    
    u8* image;
    unsigned width, height;

    lodepng_decode32_file(&image, &width, &height, path);
    u8* gpusrc = linearAlloc(width*height*4);

    for (u32 i = 0; i < width; i++) 
    {
        for (u32 j = 0; j < height; j++) 
        {
            const u32 p = (i + j*width) * 4;

            u8 r = *(u8*)(image + p);
            u8 g = *(u8*)(image + p + 1);
            u8 b = *(u8*)(image + p + 2);
            u8 a = *(u8*)(image + p + 3);

            *(gpusrc + p) = a;
            *(gpusrc + p + 1) = b;
            *(gpusrc + p + 2) = g;
            *(gpusrc + p + 3) = r;
        }
    }
    
    pp2d_load_texture_memory(id, gpusrc, width, height, GX_TRANSFER_FMT_RGBA8);
    
    free(image);
    linearFree(gpusrc);
}

void pp2d_load_texture_png_memory(size_t id, void* buf, size_t buf_size)
{
    if (id >= PP2D_MAX_TEXTURES)
    {
        return;
    }
    
    u8* image;
    unsigned width, height;

    lodepng_decode32(&image, &width, &height, buf, buf_size);
    u8* gpusrc = linearAlloc(width*height*4);

    for (u32 i = 0; i < width; i++) 
    {
        for (u32 j = 0; j < height; j++) 
        {
            const u32 p = (i + j*width) * 4;

            u8 r = *(u8*)(image + p);
            u8 g = *(u8*)(image + p + 1);
            u8 b = *(u8*)(image + p + 2);
            u8 a = *(u8*)(image + p + 3);

            *(gpusrc + p) = a;
            *(gpusrc + p + 1) = b;
            *(gpusrc + p + 2) = g;
            *(gpusrc + p + 3) = r;
        }
    }
    
    pp2d_load_texture_memory(id, gpusrc, width, height, GX_TRANSFER_FMT_RGBA8);
    
    free(image);
    linearFree(gpusrc);
}

void pp2d_set_3D(bool enable)
{
    gfxSet3D(enable);
}

static void pp2d_set_rendered_flags(bool texture, bool text, bool rectangle)
{
    renderedTexture = texture;
    renderedText = text;
    renderedRectangle = rectangle;
}

void pp2d_set_screen_color(gfxScreen_t target, u32 color)
{
    if (target == GFX_TOP)
    {
        C3D_RenderTargetSetClear(topLeft, C3D_CLEAR_ALL, color, 0);
        C3D_RenderTargetSetClear(topRight, C3D_CLEAR_ALL, color, 0);
    }
    else
    {
        C3D_RenderTargetSetClear(bot, C3D_CLEAR_ALL, color, 0);
    }
}

static void pp2d_set_text_color(u32 color)
{
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, 0, 0);
    C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_CONSTANT, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
    C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
    C3D_TexEnvColor(env, color);
}

void pp2d_set_texture_filter(GPU_TEXTURE_FILTER_PARAM magFilter, GPU_TEXTURE_FILTER_PARAM minFilter)
{
    textureFilters.magFilter = magFilter;
    textureFilters.minFilter = minFilter;
}

void pp2d_texture_select_part(size_t id, int x, int y, int xbegin, int ybegin, int width, int height)
{
    if (id >= PP2D_MAX_TEXTURES)
    {
        pp2dBuffer.initialized = false;
        return;
    }
    
    pp2dBuffer.id = id;
    pp2dBuffer.x = x;
    pp2dBuffer.y = y;
    pp2dBuffer.xbegin = xbegin;
    pp2dBuffer.ybegin = ybegin;
    pp2dBuffer.width = width;
    pp2dBuffer.height = height;
    pp2dBuffer.color = PP2D_DEFAULT_COLOR_NEUTRAL;
    pp2dBuffer.fliptype = PP2D_FLIP_NONE;
    pp2dBuffer.scaleX = 1;
    pp2dBuffer.scaleY = 1;
    pp2dBuffer.angle = 0;
    pp2dBuffer.depth = PP2D_DEFAULT_DEPTH;
    pp2dBuffer.initialized = true;
}

void pp2d_texture_blend(u32 color)
{
    pp2dBuffer.color = color;
}

void pp2d_texture_depth(float depth)
{
    pp2dBuffer.depth = depth;
}

void pp2d_texture_flip(flipType_t fliptype)
{
    pp2dBuffer.fliptype = fliptype;
}

void pp2d_texture_queue(void)
{
    if (!pp2dBuffer.initialized || vertexData.cur + 6 > PP2D_MAX_VERTICES)
    {
        return;
    }
    
    size_t id = pp2dBuffer.id;
    
    float left = (float)pp2dBuffer.xbegin / (float)textures[id].tex.width;
    float right = (float)(pp2dBuffer.xbegin + pp2dBuffer.width) / (float)textures[id].tex.width;
    float top = (float)(textures[id].tex.height - pp2dBuffer.ybegin) / (float)textures[id].tex.height;
    float bottom = (float)(textures[id].tex.height - pp2dBuffer.ybegin - pp2dBuffer.height) / (float)textures[id].tex.height;
    
    // scaling
    pp2dBuffer.height *= pp2dBuffer.scaleY;
    pp2dBuffer.width *= pp2dBuffer.scaleX;
    float vert[6][2] = {
        {                   pp2dBuffer.x,                     pp2dBuffer.y},
        {                   pp2dBuffer.x, pp2dBuffer.y + pp2dBuffer.height},
        {pp2dBuffer.width + pp2dBuffer.x,                     pp2dBuffer.y},
        {pp2dBuffer.width + pp2dBuffer.x,                     pp2dBuffer.y},
        {                   pp2dBuffer.x, pp2dBuffer.y + pp2dBuffer.height},
        {pp2dBuffer.width + pp2dBuffer.x, pp2dBuffer.y + pp2dBuffer.height},
    };

    // flipping
    if (pp2dBuffer.fliptype == PP2D_FLIP_BOTH || pp2dBuffer.fliptype == PP2D_FLIP_HORI)
    {
        float tmp = left;
        left = right;
        right = tmp;
    }
    if (pp2dBuffer.fliptype == PP2D_FLIP_BOTH || pp2dBuffer.fliptype == PP2D_FLIP_VERT)
    {
        float tmp = top;
        top = bottom;
        bottom = tmp;
    }
    
    // rotating
    pp2dBuffer.angle = fmod(pp2dBuffer.angle, 360);
    if (pp2dBuffer.angle != 0)
    {
        const float rad = pp2dBuffer.angle/(180/M_PI);
        const float c = cosf(rad);
        const float s = sinf(rad);
        
        const float xcenter = pp2dBuffer.x + pp2dBuffer.width/2.0f;
        const float ycenter = pp2dBuffer.y + pp2dBuffer.height/2.0f;
        
        for (int i = 0; i < 6; i++)
        {
            float oldx = vert[i][0];
            float oldy = vert[i][1];
            
            vert[i][0] = c * (oldx - xcenter) - s * (oldy - ycenter) + xcenter;
            vert[i][1] = s * (oldx - xcenter) + c * (oldy - ycenter) + ycenter;
        }
    }

    const bool changeSheet = id != prevSpritesheet;
    const bool changeColor = pp2dBuffer.color != prevColor;
    // draw the remaining vertices in the queue before changing data
    if ((changeSheet || changeColor) && (vertexData.cur > 0))
    {
        pp2d_draw_arrays();
    }

    // binding
    if (changeSheet || renderedText)
    {
        prevSpritesheet = id;
        C3D_TexBind(0, &textures[id].tex);
        renderedText = false;
    }

    // blending
    if (changeColor)
    {
        prevColor = pp2dBuffer.color;
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_CONSTANT, 0);
        C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
        C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
        C3D_TexEnvColor(env, pp2dBuffer.color);
    }

    // rendering
    pp2d_add_text_vertex(vert[0][0], vert[0][1], pp2dBuffer.depth, left, top);
    pp2d_add_text_vertex(vert[1][0], vert[1][1], pp2dBuffer.depth, left, bottom);
    pp2d_add_text_vertex(vert[2][0], vert[2][1], pp2dBuffer.depth, right, top);
    pp2d_add_text_vertex(vert[3][0], vert[3][1], pp2dBuffer.depth, right, top);
    pp2d_add_text_vertex(vert[4][0], vert[4][1], pp2dBuffer.depth, left, bottom);
    pp2d_add_text_vertex(vert[5][0], vert[5][1], pp2dBuffer.depth, right, bottom);

    pp2d_set_rendered_flags(true, false, false);
}

void pp2d_texture_position(int x, int y)
{
    pp2dBuffer.x = x;
    pp2dBuffer.y = y;
}

void pp2d_texture_rotate(float angle)
{
    pp2dBuffer.angle = angle;
}

void pp2d_texture_scale(float scaleX, float scaleY)
{
    pp2dBuffer.scaleX = scaleX;
    pp2dBuffer.scaleY = scaleY;
}