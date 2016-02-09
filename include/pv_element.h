#ifndef __PV_ELEMENT_H__
#define __PV_ELEMENT_H__
/** ******************************
 * @brief PhotonVector Vector Graphics Format.
 *
 ****************************** */

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

typedef enum _PvElementKind{
	PvElementKind_NotDefined,
	/* complex element kinds (group) */
	PvElementKind_Layer,
	PvElementKind_Group,
	/* simple element kinds */
	PvElementKind_Bezier,
	PvElementKind_Raster, /* Raster image */
}PvElementKind;

// 兼ねるLayer
struct _PvElementGroupData;
typedef struct _PvElementGroupData PvElementGroupData;
struct _PvElementGroupData{
	char *name;
};

struct _PvPoint;
typedef struct _PvPoint PvPoint;
struct _PvPoint{
	double x;
	double y;
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

struct _PvElementRasterData;
typedef struct _PvElementRasterData PvElementRasterData;
struct _PvElementRasterData{
	char *path;
	GdkPixbuf *pixbuf;
};

struct _PvElement;
typedef struct _PvElement PvElement;
struct _PvElement{
	PvElement *parent;
	PvElement **childs; // I know "children".

	PvElementKind kind;
	// kind固有の情報を格納した型のオブジェクト
	gpointer data;
};

PvElement *pv_element_new(const PvElementKind kind);
/** @brief 
 *
 * @param parent
 *		NULL: return Error.
 *		Not Layer(Group): return Error.
 * @param prev
 *		NULL: append toplevel element in parent.
 *		Element: append under the prev.
 */
bool pv_element_append_child(PvElement * const parent,
		PvElement * const prev, PvElement * const element);
/** @brief 
 *
 * @param this
 *		Not Bezier: return Error.
 */
bool pv_element_bezier_add_anchor_point(PvElement * const this,
					const PvAnchorPoint anchor_point);
bool pv_element_raster_read_file(PvElement * const this,
					const char * const path);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_ELEMENT_H__
