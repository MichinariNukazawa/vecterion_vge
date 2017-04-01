/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_BASIC_SHAPE_DATAS_H
#define include_PV_BASIC_SHAPE_DATAS_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libxml/xmlwriter.h>
#include <stdbool.h>
#include "pv_element_general.h"
#include "pv_element.h"
#include "pv_element_info.h"

typedef gpointer (*PvBasicShapeFuncNewData)();
typedef void (*PvBasicShapeFuncFreeData)(
		void *);
typedef gpointer (*PvBasicShapeFuncCopyNewData)(
		const void *);
typedef bool (*PvBasicShapeFuncWriteSvg)(
		InfoTargetSvg *target,
		const PvElement *element,
		const ConfWriteSvg *conf);
typedef void (*PvBasicShapeFuncDraw)(
		cairo_t *cr,
		const void *data);
typedef PvPoint (*PvBasicShapeFuncGetSize)(
		const void *);
typedef bool (*PvBasicShapeFuncIsDiffOne)(
		const void *data0,
		const void *data1);

typedef struct{
	PvBasicShapeKind kind;
	const char *name;
	PvBasicShapeFuncNewData		func_new_data;
	PvBasicShapeFuncFreeData	func_free_data;
	PvBasicShapeFuncCopyNewData	func_copy_new_data;
	PvBasicShapeFuncWriteSvg	func_write_svg;
	PvBasicShapeFuncDraw		func_draw;
	PvBasicShapeFuncGetSize		func_get_size;
	PvBasicShapeFuncIsDiffOne	func_is_diff_one;
}PvBasicShapeInfo;

extern const PvBasicShapeInfo _pv_basic_shape_info[];

const PvBasicShapeInfo *pv_basic_shape_info_get_from_kind(PvBasicShapeKind kind);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_BASIC_SHAPE_DATAS_H

