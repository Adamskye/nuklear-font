#include "nk_font.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void nkf_init_font(
	struct nkf_font *font,
	struct nk_context *ctx,
	const char *font_filename,
	float px_height,
	unsigned oversample_x,
	unsigned oversample_y
) {
	if (font) nkf_clean_font(font);
	assert(ctx && font_filename && "Not initialising font correctly");


	/* Loading font file */
    long size;
    FILE *font_file = fopen(font_filename, "rb");
	if (!font_file) {
		fprintf(stderr, "Failed to open font file: %s\n", font_filename);
		return;
	}
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    font->font_buf = malloc(size);
    fread(font->font_buf, size, 1, font_file);
    fclose(font_file);


	/* Initialising variables */
	font->ctx = ctx;
	font->handle.userdata.ptr = font;
	font->handle.query = nkf_query_font;
	font->handle.width = nkf_get_text_width;
	font->handle.height = px_height;
	font->tex_height = 0;
	font->oversample_x = oversample_x;
	font->oversample_y = oversample_y;

	// calculate ascent
	stbtt_fontinfo font_info;
	stbtt_InitFont(&font_info, font->font_buf, 0);
	stbtt_GetFontVMetrics(&font_info, &font->ascent, NULL, NULL);
	font->ascent = roundf(font->ascent * stbtt_ScaleForPixelHeight(&font_info, px_height));
	

	/* Init texture */
	glDeleteTextures(1, (GLuint[]){font->handle.texture.id});
	glGenTextures(1, (GLuint*)&font->handle.texture.id);
	glBindTexture(GL_TEXTURE_2D, font->handle.texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
}

static void nkf_init_group(struct nkf_font *font, unsigned group_idx) {
	/* Checks */
	if (group_idx >= NK_FONT_NUM_GROUPS || !font) return;
	if (font->groups[group_idx]) {
		fprintf(stderr, "struct nkf_font group already initialised\n");
		return;
	}


	/* Init variables */
	unsigned char combined_data[NKF_ATLAS_WIDTH * (font->tex_height + NKF_MAX_GROUP_HEIGHT)];
	memset(combined_data, 0, sizeof(combined_data));


	/* Packing */
	stbtt_pack_context spc;
	if (!stbtt_PackBegin(&spc, &combined_data[NKF_ATLAS_WIDTH * font->tex_height], NKF_ATLAS_WIDTH, NKF_MAX_GROUP_HEIGHT, 0, 1, &font->ctx->pool.alloc)) {
		fprintf(stderr, "Failed to initialise packing context\n");
		return;
	}
	stbtt_PackSetOversampling(&spc, font->oversample_x, font->oversample_y);
	stbtt_PackSetSkipMissingCodepoints(&spc, 1);
	font->groups[group_idx] = calloc(1, sizeof(struct nkf_font_group));
	struct nkf_font_group *group = font->groups[group_idx];
	group->y_begin = font->tex_height;
	stbtt_PackFontRange(&spc, font->font_buf, 0, font->handle.height, NKF_CHARS_PER_GROUP * group_idx, NKF_CHARS_PER_GROUP, group->chars);
	stbtt_PackEnd(&spc);


	/* Cropping group */
	unsigned short group_height = 0;
	for (unsigned i = 0; i < NKF_CHARS_PER_GROUP; ++i) {
		if (group->chars[i].y1 > group_height) {
			group_height = group->chars[i].y1;
		}
	}
	unsigned combined_height = font->tex_height + group_height;


	/* Fetching current atlas and combining it with new group */
	glBindTexture(GL_TEXTURE_2D, font->handle.texture.id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, combined_data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, NKF_ATLAS_WIDTH, combined_height, 0, GL_RED, GL_UNSIGNED_BYTE, combined_data);
	glGenerateMipmap(GL_TEXTURE_2D);


	/* Updating texture height */
	font->tex_height = combined_height;

#ifdef NKF_DEBUG
	stbi_write_png("atlas.png", NKF_ATLAS_WIDTH, combined_height, 1, combined_data, NKF_ATLAS_WIDTH);
#endif // NKF_DEBUG
}

void nkf_clean_font(struct nkf_font *font) {
	nkf_clear_atlas(font);
	if (font->font_buf) free(font->font_buf);
}

void nkf_clear_atlas(struct nkf_font *font) {
	for (unsigned i_group = 0; i_group < NK_FONT_NUM_GROUPS; ++i_group) {
		if (font->groups[i_group]) {
			free(font->groups[i_group]);
		}
	}
}

static float nkf_get_text_width(nk_handle handle, float height, const char *text, int len) {
	unsigned codepoint;
	int text_len = 0;
	float text_width = 0;
	int glyph_len = 0;
	float scale = 0;

	struct nkf_font *font = handle.ptr;
	if (!font || !text || !len) return 0.0f;

	scale = height / font->handle.height;
	glyph_len = text_len = nk_utf_decode(text, &codepoint, (int)len);
	if (!glyph_len) return 0.0f;
	while (text_len <= (int)len && glyph_len) {
        struct nk_user_font_glyph g;
        if (codepoint == NK_UTF_INVALID) break;

        /* Query currently drawn glyph information */
		nkf_query_font(handle, font->handle.height, &g, codepoint, 0);
        text_width += g.xadvance * scale;

        /* Offset next glyph */
        glyph_len = nk_utf_decode(text + text_len, &codepoint, (int)len - text_len);
        text_len += glyph_len;
    }
    return text_width;
}

static void nkf_query_font(nk_handle handle, float font_height, struct nk_user_font_glyph *glyph, nk_rune codepoint, nk_rune next_codepoint) {
	struct nkf_font *font = (struct nkf_font*)handle.ptr;
	if (!font || !glyph) return;

	float scale;
	unsigned group_idx = codepoint / NKF_CHARS_PER_GROUP;
	unsigned char_idx = codepoint % NKF_CHARS_PER_GROUP;
	stbtt_packedchar *ch;
	unsigned y_begin;

	if (!font->groups[group_idx]) {
		nkf_init_group(font, group_idx);
	}
	if (!font->groups[group_idx]) return;

	ch = &font->groups[group_idx]->chars[char_idx];
	y_begin = font->groups[group_idx]->y_begin;
	scale = font_height / font->handle.height;

	glyph->width = (ch->x1 - ch->x0) * scale / font->oversample_x;
	glyph->height = (ch->y1 - ch->y0) * scale / font->oversample_y;
	glyph->xadvance = ch->xadvance * scale;
	glyph->offset = nk_vec2(ch->xoff * scale, (ch->yoff + (font->ascent)) * scale);
	glyph->uv[0] = nk_vec2((float)ch->x0 / NKF_ATLAS_WIDTH, (float)(ch->y0 + y_begin) / font->tex_height);
	glyph->uv[1] = nk_vec2((float)ch->x1 / NKF_ATLAS_WIDTH, (float)(ch->y1 + y_begin) / font->tex_height);
}
