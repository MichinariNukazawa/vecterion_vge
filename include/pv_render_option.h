/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_RENDER_OPTION_H
#define include_PV_RENDER_OPTION_H
#include "pv_render_context.h"
#include "pv_focus.h"

typedef struct PvRenderOption{
	PvRenderContext render_context;
	const PvFocus *focus;
}PvRenderOption;

static inline PvRenderOption PvRenderOption_Default()
{
	PvRenderOption render_option;
	render_option.render_context = PvRenderContext_Default;
	render_option.focus = NULL;

	return render_option;
}

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_RENDER_OPTION_H

