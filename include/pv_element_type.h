/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_ELEMENT_TYPE_H
#define include_PV_ELEMENT_TYPE_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "pv_type.h"
#include "pv_general.h"
#include "pv_anchor_path.h"
#include "pv_appearance.h"
#include "pv_element_group_info.h"
#include "pv_basic_shape_type.h"

typedef enum{
	PvElementKind_NotDefined,
	/* special element document root */
	PvElementKind_Root,
	/* complex element kinds (group) */
	PvElementKind_Layer,
	PvElementKind_Group,
	/* simple element kinds */
	PvElementKind_Curve,
	PvElementKind_BasicShape, /* BasicShape image */

	PvElementKind_EndOfKind, //!< Sentinel
}PvElementKind;


// **
// data struct for ElementKinds
// **

// using to Root, Layer, Group
typedef struct{
	char *name;
	PvElementGroupKind kind;
	cairo_fill_rule_t cairo_fill_rule;
}PvElementGroupData;

typedef struct{
	PvAnchorPath *anchor_path;
}PvElementCurveData;

typedef struct{
	PvBasicShapeKind kind;
	void *data;

	PvAnchorPath *anchor_path;

	// default payload appearances (void **) type is easy access
	PvAppearance **basic_shape_appearances;
}PvElementBasicShapeData;


#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_TYPE_H

