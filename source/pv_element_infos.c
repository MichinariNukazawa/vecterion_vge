#include "pv_element_infos.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"

const PvElementInfo _pv_element_infos[] = {
	{PvElementKind_NotDefined, "NotDefined",},
	{PvElementKind_Root, "Root",},
	{PvElementKind_Layer, "Layer",},
	{PvElementKind_Group, "Group",},
	{PvElementKind_Bezier, "Bezier",},
	{PvElementKind_Raster, "Raster",},
};

const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind)
{
	int num = sizeof(_pv_element_infos) / sizeof(PvElementInfo);
	for(int i = 0; i < num; i++){
		if(kind == _pv_element_infos[i].kind){
			return &_pv_element_infos[i];
		}
	}

	pv_bug("");
	return NULL;
}
