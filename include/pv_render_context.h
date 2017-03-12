#ifndef include_PV_RENDER_CONTEXT_H
#define include_PV_RENDER_CONTEXT_H

typedef enum{
	PvBackgroundKind_Transparent,
	PvBackgroundKind_White,
	PvBackgroundKind_Checkboard,
}PvBackgroundKind;

struct PvRenderContext;
typedef struct PvRenderContext PvRenderContext;
struct PvRenderContext{
	bool is_focus;
	bool is_extent_view;
	bool is_frame_line;
	bool is_transparent_grid;
	double scale;
	int margin;
	PvBackgroundKind background_kind;
};
static const PvRenderContext PvRenderContext_Default = {
	.is_focus	= false,
	.is_extent_view	= false,
	.is_frame_line	= false,
	.is_transparent_grid = false,
	.scale		= 1.0,
	.margin		= 0,
	.background_kind = PvBackgroundKind_Checkboard,
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_RENDER_CONTEXT_H
