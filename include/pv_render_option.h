#ifndef include_PV_RENDER_OPTION_H
#define include_PV_RENDER_OPTION_H
#include "pv_render_context.h"
#include "pv_focus.h"

typedef struct PvRenderOption{
	PvRenderContext render_context;
	PvFocus focus;
}PvRenderOption;

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_RENDER_OPTION_H

