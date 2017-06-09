/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_ELEMENT_RENDER_CONTEXT_H
#define include_PV_ELEMENT_RENDER_CONTEXT_H

#include "pv_element_group_info.h"


struct PvElementRenderContext;
typedef struct PvElementRenderContext PvElementRenderContext;
struct PvElementRenderContext{
	PvElementGroupKind element_group_kind;
	int nest_mask_curve_simple;
};

static const PvElementRenderContext PvElementRenderContext_Default = {
	PvElementGroupKind_Normal,
	0,
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_RENDER_CONTEXT_H

