#ifndef include_PV_ELEMENT_GLOBAL_H
#define include_PV_ELEMENT_GLOBAL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include <errno.h>
#include <string.h>
#include "pv_type.h"

struct PvRect;
typedef struct PvRect PvRect;
struct PvRect{
	double x;
	double y;
	double w;
	double h;
};

// ** 各 ElementKindのdata構造

// 兼ねるLayer
struct PvElementGroupData;
typedef struct PvElementGroupData PvElementGroupData;
struct PvElementGroupData{
	char *name;
};

struct PvElementRasterData;
typedef struct PvElementRasterData PvElementRasterData;
struct PvElementRasterData{
	char *path;
	GdkPixbuf *pixbuf;
};

typedef enum _PvAnchorPointIndex{
	PvAnchorPointIndex_HandlePrev = 0,
	PvAnchorPointIndex_Point = 1,
	PvAnchorPointIndex_HandleNext = 2,
}PvAnchorPointIndex;

struct PvAnchorPoint;
typedef struct PvAnchorPoint PvAnchorPoint;
struct PvAnchorPoint{
	PvPoint points[3];
};
static const PvAnchorPoint PvAnchorPoint_default = {
	.points = {{0,0}, {0,0}, {0,0}, },
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

/** @brief pointer arrayの内容数を返す
 * (実長さは番兵のNULL終端があるため、return+1)
 */
int pv_general_get_parray_num(void **pointers);


bool pv_general_strtod(double *value, const char *str,
		char **endptr, const char **str_error);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_GLOBAL_H

