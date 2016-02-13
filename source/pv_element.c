#include "pv_element.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"

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

bool _pv_element_recursive_inline(PvElement *element,
		PvElementRecursiveFunc func, gpointer data,
		int *level,
		PvElementRecursiveError *error)
{
	if(!func(element, data, *level)){
		// cancel function this childs
		return true;
	}

	(*level)++;

	bool ret = true;

	int num = pv_general_get_parray_num((void **)element->childs);
	for(int i = 0; i < num; i++){
		if(!_pv_element_recursive_inline(element->childs[i],
					func, data,
					level,
					error)){
			if(!error->is_error){
				// logging first error.
				error->is_error = true;
				error->level = *level;
				error->element = element->childs[i];
			}
			ret = false;
			goto end;
		}
	}

end:
	(*level)--;

	return ret;
}


bool pv_element_recursive(PvElement *element,
		PvElementRecursiveFunc func, gpointer data,
		PvElementRecursiveError *error)
{
	if(NULL == element){
		pv_error("");
		return false;
	}
	if(NULL == func){
		pv_error("");
		return false;
	}
	if(NULL == error){
		pv_error("");
		return false;
	}

	// initialize.
	error->is_error = false;
	error->level = -1;
	error->element = NULL;

	int level = 0;
	bool ret = _pv_element_recursive_inline(element,
			func, data,
			&level,
			error);

	return ret;
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
		case PvElementKind_Root:
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
	PvAnchorPoint *anchor_points = (PvAnchorPoint *)realloc(data->anchor_points,
			sizeof(PvAnchorPoint) * (data->anchor_points_num + 1));
	anchor_points[data->anchor_points_num] = anchor_point;
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
