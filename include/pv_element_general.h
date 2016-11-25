#ifndef include_PV_ELEMENT_GLOBAL_H
#define include_PV_ELEMENT_GLOBAL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "pv_type.h"
#include "pv_general.h"
#include "pv_anchor_point.h"
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
	PvElementRasterAppearanceIndex_Translate,
	PvElementRasterAppearanceIndex_Resize,
	PvElementRasterAppearanceIndex_Rotate,
}PvElementRasterAppearanceIndex;
#define Num_PvElementRasterAppearance (3)

struct PvElementRasterData;
typedef struct PvElementRasterData PvElementRasterData;
struct PvElementRasterData{
	char *path;
	GdkPixbuf *pixbuf;

	// default payload appearances (void **) type is easy access
	PvAppearance **raster_appearances;
};

struct PvElementBezierData;
typedef struct PvElementBezierData PvElementBezierData;
struct PvElementBezierData{
	bool is_close;
	int anchor_points_num;
	PvAnchorPoint *anchor_points;
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
	PvElementKind_Bezier,
	PvElementKind_Raster, /* Raster image */

	/* 番兵 */
	PvElementKind_EndOfKind,
}PvElementKind;


#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_GLOBAL_H

