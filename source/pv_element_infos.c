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

gpointer _pv_element_error_return_null_copy_new(void *_data)
{
	pv_error("");
	return NULL;
}

/** @brief write_svg未実装箇所に挿入する */
int _pv_element_write_svg_notimplement(
		InfoTargetSvg *target,
		const PvElement *element, const ConfWriteSvg *conf)
{
	if(NULL == target){
		pv_bug("");
		return -1;
	}

	if(NULL == target->xml_parent_node){
		pv_bug("");
		return -1;
	}

	if(NULL == element){
		pv_bug("");
		return -1;
	}

	if(NULL == conf){
		pv_bug("");
		return -1;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	if(NULL == info){
		pv_bug("%d\n", element->kind);
		return -1;
	}

	xmlNodePtr node = NULL;
	// node = xmlNewNode(NULL, BAD_CAST info->name);
	node = xmlNewComment(BAD_CAST info->name);

	xmlAddChild(target->xml_parent_node, node);
	target->xml_new_node = node;

	return 0;
}

/** @brief 
 * arg1 NULL -> return NULL and not error(is_error == false)
 */
char *pv_general_new_str(const char *src, bool *is_error)
{
	*is_error = true;

	if(NULL == src){
		*is_error = false;
		return NULL;
	}

	if(20 < strlen(src)){
		pv_debug("len:%lu '%s'\n", strlen(src), src);
	}

	char *dst = (char *)malloc(sizeof(char) * (strlen(src) + 1));
	if(NULL == dst){
		pv_critical("");
		exit(-1);
	}

	strcpy(dst, src);

	*is_error = false;
	return dst;
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

gpointer _pv_element_group_data_copy_new(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementGroupData *data = (PvElementGroupData *)_data;

	PvElementGroupData *new_data = (PvElementGroupData *)malloc(sizeof(PvElementGroupData));
	if(NULL == new_data){
		pv_critical("");
		exit(-1);
	}

	bool is_error = true;
	new_data->name = pv_general_new_str(data->name, &is_error);
	if(is_error){
		pv_critical("");
		exit(-1);
	}

	return (gpointer)new_data;
}

int _pv_element_group_write_svg(
		InfoTargetSvg *target,
		const PvElement *element, const ConfWriteSvg *conf)
{
	if(NULL == target){
		pv_bug("");
		return -1;
	}

	if(NULL == target->xml_parent_node){
		pv_bug("");
		return -1;
	}

	if(NULL == element){
		pv_bug("");
		return -1;
	}

	if(NULL == conf){
		pv_bug("");
		return -1;
	}

	if(PvElementKind_Root == element->kind){
		return true;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	if(NULL == info){
		pv_bug("%d\n", element->kind);
		return -1;
	}

	xmlNodePtr node = NULL;
	node = xmlNewNode(NULL, BAD_CAST "g");

	// ** To Inkscape
	if(PvElementKind_Layer == element->kind){
		xmlNewProp(node, BAD_CAST "inkscape:groupmode", BAD_CAST "layer");
	}
	xmlNewProp(node, BAD_CAST "inkscape:label", BAD_CAST info->name);

	xmlAddChild(target->xml_parent_node, node);
	target->xml_parent_node = node;
	target->xml_new_node = node;

	return 0;
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

gpointer _pv_element_bezier_data_copy_new(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementBezierData *data = (PvElementBezierData *)_data;

	PvElementBezierData *new_data = (PvElementBezierData *)malloc(sizeof(PvElementBezierData));
	if(NULL == new_data){
		pv_critical("");
		exit(-1);
	}

	new_data->anchor_points_num = 0;
	new_data->anchor_points = NULL;
	if(0 < data->anchor_points_num && NULL != data->anchor_points){
		size_t size = data->anchor_points_num * sizeof(PvAnchorPoint);
		new_data->anchor_points = malloc(size);
		if(NULL == new_data->anchor_points){
			pv_critical("");
			exit(-1);
		}
		memcpy(new_data->anchor_points, data->anchor_points, size);
		new_data->anchor_points_num = data->anchor_points_num;
	}

	return (gpointer)new_data;
}

int _pv_element_bezier_write_svg(
		InfoTargetSvg *target,
		const PvElement *element, const ConfWriteSvg *conf)
{
	if(NULL == target){
		pv_bug("");
		return -1;
	}

	if(NULL == target->xml_parent_node){
		pv_bug("");
		return -1;
	}

	if(NULL == element){
		pv_bug("");
		return -1;
	}

	if(NULL == conf){
		pv_bug("");
		return -1;
	}

	PvElementBezierData *data = (PvElementBezierData *)element->data;

	char *str_current = NULL;
	for(int i = 0; i < data->anchor_points_num; i++){
		PvAnchorPoint ap = data->anchor_points[i];
		
		char *str_point = g_strdup_printf("%c %f %f",
				(0 == i)? 'M':'L',
				ap.points[PvAnchorPointIndex_Point].x, 
				ap.points[PvAnchorPointIndex_Point].y
				);
		if(NULL == str_point){
			pv_critical("");
			return -1;
		}

		char *str_prev = str_current;
		str_current = g_strdup_printf("%s %s",
				((NULL == str_prev)? "":str_prev),
				str_point
				);
		if(NULL == str_current){
			pv_critical("");
			return -1;
		}

		g_free(str_point);
		g_free(str_prev);
	}

	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "path");
	xmlNewProp(node, BAD_CAST "fill", BAD_CAST "none");
	xmlNewProp(node, BAD_CAST "stroke", BAD_CAST "black");
	xmlNewProp(node, BAD_CAST "stroke-width", BAD_CAST "1");
	xmlNewProp(node, BAD_CAST "d", BAD_CAST str_current);

	g_free(str_current);

	xmlAddChild(target->xml_parent_node, node);
	//target->xml_parent_node = node;
	target->xml_new_node = node;

	return 0;
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

gpointer _pv_element_raster_data_copy_new(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementRasterData *data = (PvElementRasterData *)_data;

	PvElementRasterData *new_data = (PvElementRasterData *)malloc(sizeof(PvElementRasterData));
	if(NULL == new_data){
		pv_critical("");
		exit(-1);
	}

	bool is_error = true;
	new_data->path = pv_general_new_str(data->path, &is_error);
	if(is_error){
		pv_critical("");
		exit(-1);
	}
	
	if(NULL == data->pixbuf){
		new_data->pixbuf = NULL;
	}else{
		new_data->pixbuf = gdk_pixbuf_copy(data->pixbuf);
		if(NULL == new_data->pixbuf){
			pv_critical("");
			exit(-1);
		}
	}

	return (gpointer)new_data;
}

// ** ElementInfo配列の定義

const PvElementInfo _pv_element_infos[] = {
	{PvElementKind_NotDefined, "NotDefined",
			_pv_element_error_return_null_new,
			_pv_element_error_return_null_delete,
			_pv_element_error_return_null_copy_new,
			_pv_element_write_svg_notimplement,
	},
	{PvElementKind_Root, "Root",
			_pv_element_group_data_new,
			_pv_element_group_data_delete,
			_pv_element_group_data_copy_new,
			_pv_element_group_write_svg,
	},
	{PvElementKind_Layer, "Layer",
			_pv_element_group_data_new,
			_pv_element_group_data_delete,
			_pv_element_group_data_copy_new,
			_pv_element_group_write_svg,
	},
	{PvElementKind_Group, "Group",
			_pv_element_group_data_new,
			_pv_element_group_data_delete,
			_pv_element_group_data_copy_new,
			_pv_element_group_write_svg,
	},
	{PvElementKind_Bezier, "Bezier",
			_pv_element_bezier_data_new,
			_pv_element_bezier_data_delete,
			_pv_element_bezier_data_copy_new,
			_pv_element_bezier_write_svg,
	},
	{PvElementKind_Raster, "Raster",
			_pv_element_raster_data_new,
			_pv_element_raster_data_delete,
			_pv_element_raster_data_copy_new,
			_pv_element_write_svg_notimplement,
	},
	/* 番兵 */
	{PvElementKind_EndOfKind, "EndOfKind",
			_pv_element_error_return_null_new,
			_pv_element_error_return_null_delete,
			_pv_element_error_return_null_copy_new,
			_pv_element_write_svg_notimplement,
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
