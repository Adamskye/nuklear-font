# Nuklear Font
Simple font handler for applications using [nuklear](https://github.com/Immediate-Mode-UI/Nuklear), the single-header ANSI C immediate mode cross-platform GUI library.
This is a work-in-progress.

## Requirements
- [nuklear](https://github.com/Immediate-Mode-UI/Nuklear)
- [stb_truetype.h](https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h)
- [stb_rect_pack.h](https://raw.githubusercontent.com/nothings/stb/master/stb_rect_pack.h)
- [stb_image_write.h](https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h) _(Optional - for debugging)_

- Currently uses OpenGL with GLAD (though this can be changed to whatever loader you want in `nkf_font.h`).

## Usage
Compile your program with `nk_font.c` and `nk_font.h`. Make sure the dependencies are in an included directory and that they have been implemented in one source file.

### Example
```
// example nuklear options
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_NO_STB_RECT_PACK_IMPLEMENTATION
#define NK_NO_STB_TRUETYPE_IMPLEMENTATION
#define NK_IMPLEMENTATION
#include "nuklear.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#include "nk_font.h"
// ... other includes

int main() {
    struct nk_context nk;
    // ... set up nuklear as usual
    
    /* Creating a font */
    struct nkf_font font;
    nkf_init_font(&font, nk, "FONT_NAME.ttf", 24, 2, 2);
    // see `nk_font.h` for the list of parameters
    
    /* Using a font */
    nk_style_set_font(nk, &font.handle);
    
    /* Cleaning up a font */
    nkf_clean_font(&font);
    
    // ... clean up everything else
    return 0;
}
```

### Options
From `nkf_font.h`:
```
/**
 * Options
 * =======
 * NKF_CHARS_PER_GROUP      Number of characters in a group (default 128).
 * NKF_ATLAS_WIDTH          Fixed width of atlas (default 512).
 * NKF_MAX_GROUP_HEIGHT     Maximum height of a group (default 512).
 * NKF_DEBUG                If defined, store font atlas as "atlas.png".
 *
 * Characters may not show correctly if there isn't enough space to store a
 * group.
 *
 * If you have this issue, either reduce the font size, reduce oversampling,
 * increase NKF_ATLAS_WIDTH, increase NKF_MAX_GROUP_HEIGHT or decrease
 * NKF_CHARS_PER_GROUP.
 */
```
Define these macros at compile time to override the defaults.

## How it works & advantages over nuklear's font baking API
Nuklear's font baking API works fine if you are using a small range of glyphs, but causes problems with a large number of glyphs. This is because nuklear will create a font atlas containing every single character within the ranges you define, so including CJK characters, for instance, would result in a huge font atlas being created, which takes a long time and uses a lot of memory.

This font handler splits characters into groups of 128. Initially, the font atlas contains nothing, but whenever a character is queried from a group that hasn't been queried yet, an atlas is created of that group and then added to the main font atlas. This way, only characters that have been used will be included in the atlas.

## Limitations
- There is currently no way to define the range of codepoints you want to use for each font. For now, you can just use a tool like `pyftsubset` to strip away any codepoints that won't be used.
- There is no way to combine multiple fonts together (i.e. using a list of fallback fonts) so you will instead need to manually combine fonts using a program like [FontForge](https://fontforge.org/).

## Debug
When compiling your project, define the macro `NKF_DEBUG` if you want this library to save the atlas to `atlas.png` which is updated whenever the atlas is.

## To do
- [ ] Show a placeholder character when a character isn't found (without storing the placeholder multiple times in the atlas).
- [ ] Allow using this library without OpenGL/GLAD.
