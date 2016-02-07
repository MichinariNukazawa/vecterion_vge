#include "pv_vg.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"

PvVg *pv_vg_new()
{
	PvVg *this = (PvVg *)malloc(sizeof(PvVg));
	if(NULL == this){
		pv_error("");
		return NULL;
	}

	this->element_root = pv_element_new(PvElementKind_Layer);
	if(NULL == this->element_root){
		pv_error("");
		return NULL;
	}

	PvRect _rect = {0,0,0,0};
	this->rect = _rect;

	// Todo:ドキュメントサイズは読み込みor新規作成時に決まるので、これは仮コード
	this->rect.w = 500;
	this->rect.h = 500;
	

	return this;
}

/** @brief pointer arrayの内容数を返す
 * (実長さは番兵のNULL終端があるため、return+1)
 */
int pv_general_get_parray_num(void **pointers)
{
	if(NULL == pointers){
		return 0;
	}

	int i = 0;
	while(NULL != pointers[i]){
		i++;
	}

	return i;
}

char *pv_general_str_new(const char * const src)
{
	if(NULL == src){
		pv_error("");
		return NULL;
	}

	char *dst = (char *)malloc(sizeof(char) * (strlen(src) + 1));
	if(NULL == dst){
		pv_critical("");
		exit(-1);
	}

	strcpy(dst, src);

	return dst;
}

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

PvElement *pv_element_new(const PvElementKind kind)
{
	PvElement *this = (PvElement *)malloc(sizeof(PvElement));
	if(NULL == this){
		pv_critical("");
		exit(-1);
	}

	this->parent = NULL;
	this->childs = NULL;

	gpointer data;
	switch(kind){
		case PvElementKind_Layer:
		case PvElementKind_Group:
			data = _pv_element_group_data_new();
			break;
		case PvElementKind_Bezier:
			data = _pv_element_bezier_data_new();
			break;
		case PvElementKind_Raster:
			data = _pv_element_raster_data_new();
			break;
		default:
			pv_bug("%d\n", kind);
	}
	if(NULL == data){
		pv_bug("");
		return NULL;
	}
	
	this->kind = kind;
	this->data = data;

	return this;
}



bool pv_element_append_child(PvElement * const parent,
		PvElement * const prev, PvElement * const element)
{
	int num = pv_general_get_parray_num((void **)parent->childs);
	if(0 > num){
		pv_bug("");
		return false;
	}

	PvElement **childs = (PvElement **)realloc(parent->childs,
						sizeof(PvElement*) * (num + 2));
	if(NULL == childs){
		pv_error("");
		return false;
	}
	if(NULL == prev){
		childs[num] = element;
	}else{
		bool isExist = false;
		for(int i = 0; i < num; i++){
			if(prev == childs[i]){
				isExist = true;
				memmove(&childs[i + 1], &childs[i], num - i);
				childs[i] = element;
				break;
			}
		}
		if(!isExist){
			pv_error("");
			return false;
		}
	}
	childs[num + 1] = NULL;

	parent->childs = childs;
	element->parent = parent;

	return true;
}

bool pv_element_bezier_add_anchor_point(PvElement * const this,
					const PvAnchorPoint anchor_point)
{
	if(PvElementKind_Bezier != this->kind){
		pv_bug("");
		return false;
	}

	if(NULL == this->data){
		pv_bug("");
		return false;
	}

	PvElementBezierData *data = this->data;
	int num = pv_general_get_parray_num((void **)data->anchor_points);
	if(0 > num){
		pv_error("");
		return false;
	}
	PvAnchorPoint *anchor_points = (PvAnchorPoint *)realloc(data->anchor_points,
			sizeof(PvAnchorPoint) * (data->anchor_points_num + 1));
	anchor_points[num] = anchor_point;
	data->anchor_points_num += 1;
	data->anchor_points = anchor_points;

	return true;
}

bool pv_element_raster_read_file(PvElement * const this,
					const char * const path)
{
	if(PvElementKind_Raster != this->kind){
		pv_bug("");
		return false;
	}

	if(NULL == this->data){
		pv_bug("");
		return false;
	}

	PvElementRasterData *data = this->data;
	if(NULL != data->path){
		pv_fixme("");
		return false;
	}
	data->path = pv_general_str_new(path);
	if(NULL == data->path){
		pv_error("");
		return false;
	}
	data->pixbuf = gdk_pixbuf_new_from_file(data->path, NULL);
	if(NULL == data->pixbuf){
		pv_warning("%s\n", data->path);
		return false;
	}

	return true;
}
