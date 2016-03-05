#ifndef include_PV_ELEMENT_DATAS_H
#define include_PV_ELEMENT_DATAS_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libxml/xmlwriter.h>
#include <stdbool.h>
#include "pv_element_general.h"
#include "pv_element.h"

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

typedef struct PvElementInfo{
	PvElementKind kind;
	const char *name;
	PvElementFuncNewData		func_new_data;
	PvElementFuncDeleteData		func_delete_data;
	PvElementFuncCopyNewData	func_copy_new_data;
	PvElementFuncWriteSvg		func_write_svg;
}PvElementInfo;

extern const PvElementInfo _pv_element_infos[];

const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_DATAS_H
