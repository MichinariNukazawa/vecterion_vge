/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_ELEMENT_GLOBAL_H
#define include_PV_ELEMENT_GLOBAL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "pv_type.h"
#include "pv_general.h"
#include "pv_anchor_path.h"
#include "pv_appearance.h"


// ** 各 ElementKindのdata構造

// 兼ねるLayer
struct PvElementGroupData;
typedef struct PvElementGroupData PvElementGroupData;
struct PvElementGroupData{
	char *name;
};

//! @todo this is not true matrix, do rename.
typedef struct PvMatrix{
	double x;
	double y;
	double scale_x;
	double scale_y;
	double degree;
}PvMatrix;
static const PvMatrix PvMatrix_Default = {
	.x = 0,
	.y = 0,
	.scale_x = 1.0,
	.scale_y = 1.0,
	.degree = 0,
};

typedef enum{
	PvElementBasicShapeAppearanceIndex_Translate,
	PvElementBasicShapeAppearanceIndex_Resize,
	PvElementBasicShapeAppearanceIndex_Rotate,
}PvElementBasicShapeAppearanceIndex;
#define Num_PvElementBasicShapeAppearance (3)

typedef enum{
	PvBasicShapeKind_Raster,
}PvBasicShapeKind;

typedef struct{
	char		*path;
	GdkPixbuf	*pixbuf;
	GByteArray	*urischeme_byte_array;
}PvBasicShapeRasterData;

struct PvElementBasicShapeData;
typedef struct PvElementBasicShapeData PvElementBasicShapeData;
struct PvElementBasicShapeData{
	PvBasicShapeKind kind;
	void *data;

	PvAnchorPath *anchor_path;

	// default payload appearances (void **) type is easy access
	PvAppearance **basic_shape_appearances;
};

struct PvElementCurveData;
typedef struct PvElementCurveData PvElementCurveData;
struct PvElementCurveData{
	PvAnchorPath *anchor_path;
};

// ** ElementKind定数

typedef enum _PvElementKind{
	PvElementKind_NotDefined,
	/* special element document root */
	PvElementKind_Root,
	/* complex element kinds (group) */
	PvElementKind_Layer,
	PvElementKind_Group,
	/* simple element kinds */
	PvElementKind_Curve,
	PvElementKind_BasicShape, /* BasicShape image */

	/* 番兵 */
	PvElementKind_EndOfKind,
}PvElementKind;


#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_GLOBAL_H

