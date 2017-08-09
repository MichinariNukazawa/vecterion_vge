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
#include "pv_color.h"
#include "pv_stroke.h"
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
	int dummy;
}PvElementCurveData;

typedef struct{
	PvBasicShapeKind kind;
	void *data;

	// default payload appearances (void **) type is easy access
	PvAppearance **basic_shape_appearances;
}PvElementBasicShapeData;


// ******** ********
// PvElement
// ******** ********
#define NUM_WORK_APPEARANCE (3)
struct PvElement;
typedef struct PvElement PvElement;
struct PvElement{
	PvElement *parent;
	PvElement **childs; // I know "children".

	PvColorPair color_pair;
	PvStroke stroke;
	bool is_invisible;
	bool is_locked;			//!< Lock for Edit

	//! can't direct access, use to PvElementInfo functions
	PvAnchorPath *anchor_path;

	PvElementKind kind;
	// kind固有の情報を格納した型のオブジェクト
	gpointer data;

	PvAppearance **etaion_work_appearances;
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_TYPE_H

