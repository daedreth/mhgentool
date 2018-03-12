# pp2d

![License](https://img.shields.io/badge/License-GPLv3-blue.svg)

Plug & Play 2D is an unofficial graphics rendering helper to use with libctru and citro3D.

## Notice

This repository was made private due to pp2d not working as good as it should work and not using the hardware resources with parsimony. I recently made some intense optimization work, and pp2d uses hardware resources nowadays more appropriately than in the past. For this reason, I felt this repository is ready to be public again. 

Anyways, this code is not supported by the devkitPro team and may stop working if there are breaking changes in the libraries on which pp2d depends. Use citro2d when (and if) it becomes a thing.

## Using pp2d

In order to initialize pp2d's working variables, you'll need to call `void pp2d_init(void);`. Note that this already calls `gfxInitDefault();` and `C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);` by default. `void pp2d_exit(void);` frees all the pp2d variables instead.

Here are functions you're required to actually render things to the screens.

* `void pp2d_frame_begin(gfxScreen_t target, gfx3dSide_t side);` begins a frame.
* `void pp2d_frame_draw_on(gfxScreen_t target, gfx3dSide_t side);` changes the screen target.
* `void pp2d_frame_end(void);` ends a frame.

In order to start rendering a texture, you'll need to use `void pp2d_texture_select_part(size_t id, int x, int y, int xbegin, int ybegin, int width, int height);`. Note that this discourages using separate textures.

After you used the said function, you can use the following functions in any order:

* `void pp2d_texture_blend(u32 color);`
* `void pp2d_texture_depth(float depth);`
* `void pp2d_texture_flip(flipType_t fliptype);`
* `void pp2d_texture_position(int x, int y);`
* `void pp2d_texture_rotate(float angle);`
* `void pp2d_texture_scale(float scaleX, float scaleY);`

You can use none of them or each of them, depeding what you need to do. After that, using `void pp2d_texture_queue(void);` will add the vertices into the linear buffer the GPU will use to render them.

If you're an advanced user and know what you're doing, you can use `void pp2d_draw_arrays(void);` once you filled the linear memory with the vertices you need to draw from a single spritesheet. You can also avoid calling this though, it will be handled automatically from pp2d, in particular when calling `void pp2d_frame_draw_on(gfxScreen_t target, gfx3dSide_t side);` and `void pp2d_frame_end(void);`.

Check the [example](https://github.com/BernardoGiordano/pp2d/blob/master/example/source/main.c) for more details.

![example](https://i.imgur.com/Q6dVlK6.png)

## Which issues had pp2d in the past?

pp2d used to really misuse the command buffer, the CPU and the GPU. Let's see in details:

### Texture binding

Texture binding was performed *each time* `pp2d_texture_draw()` was called. This has been solved by storing the last used spritesheet's infos inside pp2d, in order to bind a new texture *only when it's necessary*.

**This doesn't solve texture binding issues alone**. A behaviour that was encouraged in the old pp2d was to have multiple single texture loaded in memory, rather than a single spritesheet containing them all.

Relying on a single spritesheet to contain all the textures (or at least all the static ones) will increase performance and reduce CPU usage, because you're not going to bind different textures every time you want to draw a different sprite on screen.

To use the new pp2d in the best way, you'll be required to write your own spritesheet handler. You can find a really simple and barebone example of spritesheet handler in the example. More advanced examples will come in the future.

In order to discourage having multiple textures rather than just one spritesheet, `PP2D_MAX_TEXTURES`'s default value is `1`. You can change this by sending a different value through the Makefile if you need more, though.

### Texture blending

Same as texture binding, texture blending was performed each time you used `pp2d_texture_draw()`, making you waste lots of power. Now, pp2d only changes blend parameters when you use a different blend color than the past one.

This also applies to the text color. It was changed each time a function from the `pp2d_draw_text` family was called, instead it's now called only the first time you start rendering text after you rendered something different, like a sprite or a rectangle.

```
// blending
if (changeColor)
{
    prevColor = pp2dBuffer.color;
    ...
}
```

### Texture rendering

In the old pp2d, `C3D_DrawArrays` was used in each `pp2d_texture_draw()` call because the last used spritesheet wasn't stored somewhere into pp2d, causing the command buffer to be misused.

Now, when pp2d recognizes you just need to render from the same spritesheet, it will render everything at once before binding a new texture.

### Texture tiling

In order to convert textures to the proper tiled format, the old pp2d used some weird operations relying on the CPU. It now uses the proper citro3D functions to do that.

## Known issues

The new pp2d has some minor problems that will hopefully be fixed soon. In case you want to help, Pull Requests are highly appreciated.

* The white color `RGBA8(0xFF, 0xFF, 0xFF, 0xFF)` causes blending issues. This could be fixed by changing the blending portions to modulate the colors better. There are simple workarounds though, like using `RGBA8(0xFE, 0xFE, 0xFE, 0xFF)` as white color.
* Texture rotation is actually done CPU-side. This could waste some CPU usage in case you really need to rotate multiple sprites per frame.
* Report other issues in case you find more.

You can receive real-time support by joining PKSM's Discord server.

[![Discord](https://discordapp.com/api/guilds/278222834633801728/widget.png?style=banner3&time-)](https://discord.gg/bGKEyfY)

## License

pp2d is licensed under the [GPLv3 License](https://github.com/BernardoGiordano/pp2d/blob/master/LICENSE).

## Credits

* all the contributors for [citro3d](https://github.com/fincs/citro3d)
* all the contributors for [libctru](https://github.com/smealum/ctrulib)

You can [support me](https://www.patreon.com/bernardogiordano) if you like my work.