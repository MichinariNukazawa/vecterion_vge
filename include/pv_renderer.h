#ifndef include_PV_RENDERER_H
#define include_PV_RENDERER_H
#include "pv_vg.h"
#include "pv_render_context.h"
#include "pv_focus.h"

GdkPixbuf *pv_renderer_pixbuf_from_vg(PvVg * const vg,
		const PvRenderContext render_context,
		const PvFocus focus);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_RENDERER_H
