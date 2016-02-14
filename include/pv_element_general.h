#ifndef __PV_ELEMENT_GLOBAL_H__
#define __PV_ELEMENT_GLOBAL_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

struct _PvRect;
typedef struct _PvRect PvRect;
struct _PvRect{
	double x;
	double y;
	double w;
	double h;
};

struct _PvPoint;
typedef struct _PvPoint PvPoint;
struct _PvPoint{
	double x;
	double y;
};

// ** 各 ElementKindのdata構造

// 兼ねるLayer
struct _PvElementGroupData;
typedef struct _PvElementGroupData PvElementGroupData;
struct _PvElementGroupData{
	char *name;
};

struct _PvElementRasterData;
typedef struct _PvElementRasterData PvElementRasterData;
struct _PvElementRasterData{
	char *path;
	GdkPixbuf *pixbuf;
};

typedef enum _PvAnchorPointIndex{
	PvAnchorPointIndex_HandlePrev = 0,
	PvAnchorPointIndex_Point = 1,
	PvAnchorPointIndex_HandleNext = 2,
}PvAnchorPointIndex;

struct _PvAnchorPoint;
typedef struct _PvAnchorPoint PvAnchorPoint;
struct _PvAnchorPoint{
	PvPoint points[3];
};

struct _PvElementBezierData;
typedef struct _PvElementBezierData PvElementBezierData;
struct _PvElementBezierData{
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
}PvElementKind;

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_ELEMENT_GLOBAL_H__
