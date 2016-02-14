#ifndef __PV_ELEMENT_DATAS_H__
#define __PV_ELEMENT_DATAS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element_general.h"

typedef struct _PvElementInfo{
	PvElementKind kind;
	const char *name;
}PvElementInfo;

extern const PvElementInfo _pv_element_infos[];

const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_ELEMENT_DATAS_H__
