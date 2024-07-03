#ifndef FONT_H
#define FONT_H

#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#include "nuklear.h"
#include "glad/glad.h"

#include "stb_rect_pack.h"
#include "stb_truetype.h"

#ifdef NKF_DEBUG
#include "stb_image_write.h"
#endif // NKF_DEBUG

/**
 * Options
 * =======
 * NKF_CHARS_PER_GROUP		Number of characters in a group (default 128).
 * NKF_ATLAS_WIDTH			Fixed width of atlas (default 512).
 * NKF_MAX_GROUP_HEIGHT		Maximum height of a group (default 512).
 * NKF_DEBUG				If defined, store font atlas as "atlas.png".
 *
 * Characters may not show correctly if there isn't enough space to store a
 * group.
 *
 * If you have this issue, either reduce the font size, reduce oversampling,
 * increase NKF_ATLAS_WIDTH, increase NKF_MAX_GROUP_HEIGHT or decrease
 * NKF_CHARS_PER_GROUP.
 */

#ifndef NKF_CHARS_PER_GROUP 
#define NKF_CHARS_PER_GROUP 128
#endif // NKF_CHARS_PER_GROUP 
#ifndef NKF_ATLAS_WIDTH 
#define NKF_ATLAS_WIDTH 512
#endif // NKF_ATLAS_WIDTH 
#ifndef NKF_MAX_GROUP_HEIGHT 
#define NKF_MAX_GROUP_HEIGHT 512
#endif // NKF_MAX_GROUP_HEIGHT 

#define NK_FONT_NUM_GROUPS (150000 / NKF_CHARS_PER_GROUP) + NKF_CHARS_PER_GROUP
struct nkf_font_group {
	stbtt_packedchar chars[NKF_CHARS_PER_GROUP];
	unsigned y_begin;
};

struct nkf_font {
	struct nk_user_font handle;
	struct nkf_font_group *groups[NK_FONT_NUM_GROUPS];
	unsigned tex_height;
	unsigned char *font_buf;
	unsigned oversample_x;
	unsigned oversample_y;
	int ascent;
	struct nk_context *ctx;
};

void nkf_init_font(
	struct nkf_font *dst,
	struct nk_context *ctx,
	const char *font_filename,
	float px_height,
	unsigned oversample_x,
	unsigned oversample_y
);

static void nkf_init_group(struct nkf_font *font, unsigned atlas_idx);
void nkf_clean_font(struct nkf_font *font);
void nkf_clear_atlas(struct nkf_font *font);
static float nkf_get_text_width(nk_handle handle, float height, const char *text, int len);
static void nkf_query_font(nk_handle handle, float font_height, struct nk_user_font_glyph *glyph, nk_rune codepoint, nk_rune next_codepoint);

#endif // FONT_H
