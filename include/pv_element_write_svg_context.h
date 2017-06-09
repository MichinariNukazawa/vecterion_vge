/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_ELEMENT_WRITE_SVG_CONTEXT_H
#define include_PV_ELEMENT_WRITE_SVG_CONTEXT_H

#include "pv_element_group_info.h"


struct PvElementWriteSvgContext;
typedef struct PvElementWriteSvgContext PvElementWriteSvgContext;
struct PvElementWriteSvgContext{
	PvElementGroupKind element_group_kind;
};

static const PvElementWriteSvgContext PvElementWriteSvgContext_Default = {
	PvElementGroupKind_Normal,
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_WRITE_SVG_CONTEXT_H

