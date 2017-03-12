/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_RENDERER_H
#define include_PV_RENDERER_H
#include "pv_vg.h"
#include "pv_render_context.h"
#include "pv_focus.h"

/*!
 * @param focus: NULL is disable focusing decorations(ex. frame line of select elements).
 */
GdkPixbuf *pv_renderer_pixbuf_from_vg(
		const PvVg *vg,
		const PvRenderContext render_context,
		const PvFocus *focus,
		const PvElement *element_overwrite);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_RENDERER_H
