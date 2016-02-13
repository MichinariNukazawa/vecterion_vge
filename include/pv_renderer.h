#ifndef __PV_RENDERER_H__
#define __PV_RENDERER_H__
#include "pv_vg.h"
#include "pv_render_context.h"
#include "pv_focus.h"

GdkPixbuf *pv_renderer_pixbuf_from_vg(PvVg * const vg,
		const PvRenderContext render_context,
		const PvFocus focus);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_RENDERER_H__
