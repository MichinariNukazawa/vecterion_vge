#include "pv_element_infos.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"


/** @brief 無効なindexを引いた際に埋め込まれているダミー関数 */
gpointer _pv_element_error_return_null_new()
{
	pv_error("");
	return NULL;
}

bool _pv_element_error_return_null_delete(void *_data)
{
	pv_error("");
	return false;
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

bool _pv_element_group_data_delete(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementGroupData *data = (PvElementGroupData *)_data;

	if(NULL != data->name){
		free(data->name);
	}

	free(data);

	return true;
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

bool _pv_element_bezier_data_delete(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementBezierData *data = (PvElementBezierData *)_data;

	if(NULL != data->anchor_points){
		free(data->anchor_points);
	}

	free(data);

	return true;
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

bool _pv_element_raster_data_delete(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementRasterData *data = (PvElementRasterData *)_data;

	if(NULL != data->path){
		free(data->path);
	}

	if(NULL != data->pixbuf){
		g_object_unref(G_OBJECT(data->pixbuf));
	}

	free(data);

	return true;
}

// ** ElementInfo配列の定義

const PvElementInfo _pv_element_infos[] = {
	{PvElementKind_NotDefined, "NotDefined",
			_pv_element_error_return_null_new,
			_pv_element_error_return_null_delete,
	},
	{PvElementKind_Root, "Root",
			_pv_element_group_data_new,
			_pv_element_group_data_delete,
	},
	{PvElementKind_Layer, "Layer",
			_pv_element_group_data_new,
			_pv_element_group_data_delete,
	},
	{PvElementKind_Group, "Group",
			_pv_element_group_data_new,
			_pv_element_group_data_delete,
	},
	{PvElementKind_Bezier, "Bezier",
			_pv_element_bezier_data_new,
			_pv_element_bezier_data_delete,
	},
	{PvElementKind_Raster, "Raster",
			_pv_element_raster_data_new,
			_pv_element_raster_data_delete,
	},
	/* 番兵 */
	{PvElementKind_EndOfKind, "EndOfKind",
			_pv_element_error_return_null_new,
			_pv_element_error_return_null_delete,
	},
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
