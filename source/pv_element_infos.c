#include "pv_element_infos.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"


/** @brief 無効なindexを引いた際に埋め込まれているダミー関数 */
gpointer _pv_element_error_return_null()
{
	pv_error("");
	return NULL;
}

// ** PvElementKindごとのdataのnew関数群

gpointer _pv_element_group_data_new()
{
	PvElementGroupData *data = (PvElementGroupData *)malloc(sizeof(PvElementGroupData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->name = NULL;

	return (gpointer)data;
}

gpointer _pv_element_bezier_data_new()
{
	PvElementBezierData *data = (PvElementBezierData *)malloc(sizeof(PvElementBezierData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->anchor_points_num = 0;
	data->anchor_points = NULL;

	return (gpointer)data;
}

gpointer _pv_element_raster_data_new()
{
	PvElementRasterData *data = (PvElementRasterData *)malloc(sizeof(PvElementRasterData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->path = NULL;
	data->pixbuf = NULL;

	return (gpointer)data;
}


// ** ElementInfo配列の定義

const PvElementInfo _pv_element_infos[] = {
	{PvElementKind_NotDefined, "NotDefined", _pv_element_error_return_null},
	{PvElementKind_Root, "Root", _pv_element_group_data_new,},
	{PvElementKind_Layer, "Layer", _pv_element_group_data_new,},
	{PvElementKind_Group, "Group", _pv_element_group_data_new,},
	{PvElementKind_Bezier, "Bezier", _pv_element_bezier_data_new,},
	{PvElementKind_Raster, "Raster", _pv_element_raster_data_new,},
	/* 番兵 */
	{PvElementKind_EndOfKind, "EndOfKind", _pv_element_error_return_null},
};


const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind)
{

	if(PvElementKind_NotDefined == kind || PvElementKind_EndOfKind == kind){
		pv_error("%d\n", kind);
		return NULL;
	}

	int num = sizeof(_pv_element_infos) / sizeof(PvElementInfo);
	for(int i = 0; i < num; i++){
		if(kind == _pv_element_infos[i].kind){
			return &_pv_element_infos[i];
		}
	}

	pv_bug("%d\n", kind);
	return NULL;
}
