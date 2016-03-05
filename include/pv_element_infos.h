#ifndef include_PV_ELEMENT_DATAS_H
#define include_PV_ELEMENT_DATAS_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element_general.h"

typedef gpointer (*PvElementFuncNewData)(void);
typedef bool (*PvElementFuncDeleteData)(void *data);
typedef gpointer (*PvElementFuncCopyNewData)(void *data);

typedef struct PvElementInfo{
	PvElementKind kind;
	const char *name;
	PvElementFuncNewData		func_new_data;
	PvElementFuncDeleteData		func_delete_data;
	PvElementFuncCopyNewData	func_copy_new_data;
}PvElementInfo;

extern const PvElementInfo _pv_element_infos[];

const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_DATAS_H
