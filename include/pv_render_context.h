#ifndef include_PV_RENDER_CONTEXT_H
#define include_PV_RENDER_CONTEXT_H

struct PvRenderContext;
typedef struct PvRenderContext PvRenderContext;
struct PvRenderContext{
	bool is_focus;
	double scale;
};
static const PvRenderContext PvRenderContext_default = {
	.is_focus	= false,
	.scale		= 1.0,
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_RENDER_CONTEXT_H
