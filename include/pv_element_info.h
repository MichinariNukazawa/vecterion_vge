#ifndef include_PV_ELEMENT_DATAS_H
#define include_PV_ELEMENT_DATAS_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libxml/xmlwriter.h>
#include <stdbool.h>
#include "pv_element_general.h"
#include "pv_element.h"
#include "pv_render_option.h"

typedef struct {
	int dummy;
}ConfWriteSvg;

typedef struct {
	xmlNodePtr *xml_parent_nodes;
	xmlNodePtr xml_parent_node;
	xmlNodePtr xml_new_node;
}InfoTargetSvg;

typedef gpointer (*PvElementFuncNewData)(void);
typedef bool (*PvElementFuncDeleteData)(void *data);
typedef gpointer (*PvElementFuncCopyNewData)(void *data);
/** @brief
 * @return 0正常終了 負の値:異常終了
 */
typedef int (*PvElementFuncWriteSvg)(
		InfoTargetSvg *target,
		const PvElement *element, const ConfWriteSvg *conf);
typedef bool (*PvElementFuncDraw)(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element);
typedef PvElementFuncDraw PvElementFuncDrawFocusing;
typedef bool (*PvElementFuncIsTouchElement)(
		bool *is_touch,
		const PvElement *element,
		int offset,
		double gx,
		double gy);
typedef bool (*PvElementFuncIsDiffOne)(
		bool *is_diff,
		const PvElement *element0,
		const PvElement *element1);
typedef PvPoint (*PvElementFuncGetPointByAnchorPoints)(
		const PvElement *element);
typedef void (*PvElementFuncSetPointByAnchorPoints)(
		PvElement *element,
		const PvPoint);
typedef bool (*PvElementFuncMoveElement)(
		const PvElement *element,
		double gx,
		double gy);
typedef int (*PvElementFuncGetNumAnchorPoint)(
		const PvElement *element);
typedef PvAnchorPoint *(*PvElementFuncNewAnchorPoints)(
		const PvElement *element);
typedef PvAnchorPoint (*PvElementFuncGetAnchorPoint)(
		const PvElement *element,
		const int index);
typedef bool (*PvElementFuncSetAnchorPointPoint)(
		const PvElement *element,
		const int index,
		const PvPoint point);
typedef bool (*PvElementFuncMoveAnchorPointPoint)(
		const PvElement *element,
		const int index,
		const PvPoint move);
typedef PvRect (*PvElementFuncGetRectByAnchorPoints)(
		const PvElement *element);
typedef bool (*PvElementFuncSetRectByAnchorPoints)(
		PvElement *element,
		PvRect rect);
typedef PvRect (*PvElementFuncGetRectByDraw)(
		const PvElement *element);

typedef struct PvElementInfo{
	PvElementKind kind;
	const char *name;
	PvElementFuncNewData			func_new_data;
	PvElementFuncDeleteData			func_delete_data;
	PvElementFuncCopyNewData		func_copy_new_data;
	PvElementFuncWriteSvg			func_write_svg;
	PvElementFuncDraw			func_draw;
	PvElementFuncDrawFocusing		func_draw_focusing;
	PvElementFuncIsTouchElement		func_is_touch_element;
	PvElementFuncIsDiffOne			func_is_diff_one;
	PvElementFuncGetPointByAnchorPoints	func_get_point_by_anchor_points;
	PvElementFuncSetPointByAnchorPoints	func_set_point_by_anchor_points;
	PvElementFuncMoveElement		func_move_element;
	PvElementFuncGetNumAnchorPoint		func_get_num_anchor_point;
	PvElementFuncNewAnchorPoints		func_new_anchor_points;
	PvElementFuncGetAnchorPoint		func_get_anchor_point;
	PvElementFuncSetAnchorPointPoint	func_set_anchor_point_point;
	PvElementFuncMoveAnchorPointPoint	func_move_anchor_point_point;
	PvElementFuncGetRectByAnchorPoints	func_get_rect_by_anchor_points;
	PvElementFuncSetRectByAnchorPoints	func_set_rect_by_anchor_points;
	PvElementFuncGetRectByDraw		func_get_rect_by_draw;

}PvElementInfo;

extern const PvElementInfo _pv_element_info[];

const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_DATAS_H
