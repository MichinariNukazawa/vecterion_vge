#include "pv_element.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"
#include "pv_element_infos.h"


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
	for(int i = num - 1; 0 <= i; i--){
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

PvElement *pv_element_new(const PvElementKind kind)
{
	PvElement *this = (PvElement *)malloc(sizeof(PvElement));
	if(NULL == this){
		pv_critical("");
		exit(-1);
	}

	this->parent = NULL;
	this->childs = NULL;

	const PvElementInfo *info = pv_element_get_info_from_kind(kind);
	if(NULL == info || NULL == info->func_new_data){
		pv_error("");
		return NULL;
	}

	gpointer data = info->func_new_data();
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
	if(NULL == parent){
		pv_bug("");
		return false;
	}
	if(NULL == element){
		pv_bug("");
		return false;
	}

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
				memmove(&childs[i + 1], &childs[i], sizeof(PvElement*) * (num - i));
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
	if(NULL == this){
		pv_error("");
		return false;
	}

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

const char *pv_element_get_name_from_kind(PvElementKind kind)
{
	const PvElementInfo *info = pv_element_get_info_from_kind(kind);
	if(NULL == info){
		pv_error("");
		return NULL;
	}
	
	return info->name;
}
