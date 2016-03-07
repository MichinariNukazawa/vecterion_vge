#include "pv_element.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"
#include "pv_element_infos.h"

bool _pv_element_delete_single(PvElement *this);

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
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		int *level,
		PvElementRecursiveError *error)
{
	if(NULL != func_before){
		if(!func_before(element, data, *level)){
			// cancel function this childs
			return true;
		}
	}

	(*level)++;

	bool ret = true;

	int num = pv_general_get_parray_num((void **)element->childs);
	for(int i = num - 1; 0 <= i; i--){
		if(!_pv_element_recursive_inline(element->childs[i],
					func_before,
					func_after,
					data,
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

	if(NULL != func_after){
		if(!func_after(element, data, *level)){
			// this point "return false" is not mean.
			return true;
		}
	}

	return ret;
}


bool pv_element_recursive_before(PvElement *element,
		PvElementRecursiveFunc func_before,
		gpointer data,
		PvElementRecursiveError *error)
{
	if(NULL == func_before){
		pv_error("");
		return false;
	}

	return pv_element_recursive(element,
		func_before,
		NULL,
		data,
		error);
}

bool pv_element_recursive(PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		PvElementRecursiveError *error)
{
	if(NULL == element){
		pv_error("");
		return false;
	}
	if(NULL == func_before && NULL == func_after){
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
			func_before,
			func_after,
			data,
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

PvElement *_pv_element_copy_single(const PvElement *this)
{
	if(NULL == this){
		pv_error("");
		return NULL;
	}

	PvElement *new_element = (PvElement *)malloc(sizeof(PvElement));
	if(NULL == new_element){
		pv_error("");
		return NULL;
		}

	new_element->kind = this->kind;
	new_element->parent = NULL;
	new_element->childs = NULL;
	new_element->data = NULL;

	const PvElementInfo *info = pv_element_get_info_from_kind(this->kind);
	if(NULL == info || NULL == info->func_copy_new_data){
		pv_error("");
		return NULL;
	}
	void *new_data = info->func_copy_new_data(this->data);
	if(NULL == new_data){
		pv_error("");
		goto error1;
	}
	new_element->data = new_data;

	return new_element;

error1:
	free(new_element);

	return NULL;
}

PvElement *_pv_element_copy_recursive_inline(const PvElement *this,
		int *level, PvElementRecursiveError *error)
{
	if(NULL == this){
		error->is_error =true;
		error->level = *level;
		error->element = this;
		return NULL;
	}

	// copy this
	PvElement *new_element = _pv_element_copy_single(this);
	if(NULL == new_element){
		error->is_error =true;
		error->level = *level;
		error->element = this;
		return NULL;
	}

	// ** copy childs
	(*level)++;

	int num = pv_general_get_parray_num((void **)this->childs);
	if(0 > num){
		error->is_error = true;
		error->level = *level;
		error->element = this;
		goto error1;
	}

	pv_debug("copy childs:%3d:%d\n", *level, num);

	PvElement **childs = NULL;
	if(0 < num){
 		childs = malloc((num + 1) * sizeof(PvElement *));
		if(NULL == childs){
			error->is_error = true;
			error->level = *level;
			error->element = this;
			goto error1;
		}

		for(int i = 0; i < num; i++){
			PvElement *child = _pv_element_copy_recursive_inline(
					this->childs[i],
					level,
					error);
			if(NULL == child){
				if(!error->is_error){
					error->is_error =true;
					error->level = *level;
					error->element = this;
					goto error2;
				}
			}
			child->parent = new_element;
			childs[i] = child;
			childs[i + 1] = NULL;
		}

	}
	new_element->childs = childs;

	(*level)--;

	return new_element;

error2:
	pv_element_remove_delete_recursive(new_element);

	return NULL;

error1:
	_pv_element_delete_single(new_element);

	return NULL;
}

PvElement *pv_element_copy_recursive(const PvElement *this)
{
	if(NULL == this){
		pv_error("");
		return NULL;
	}

	PvElementRecursiveError error = {
		.is_error = false,
		.level = -1,
		.element = NULL,
	};
	int level = 0;
	PvElement *new_element_tree = _pv_element_copy_recursive_inline( this, &level, &error);
	if(NULL == new_element_tree){
		pv_error("");
		return NULL;
	}

	return new_element_tree;
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

bool _pv_element_delete_single(PvElement *this)
{
	if(NULL == this){
		pv_bug("");
		return false;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(this->kind);
	if(NULL == info){
		pv_bug("");
		return false;
	}

	if(!info->func_delete_data(this->data)){
		pv_bug("");
		return false;
	}

	free(this);

	return true;
}

bool _pv_element_remove_delete_recursive_inline(PvElement *this,
		int *level, PvElementRecursiveError *error)
{
	int num = pv_general_get_parray_num((void **)this->childs);
	if(0 > num){
		pv_bug("");
		return false;
	}

	(*level)++;

	pv_debug("delete childs:%3d:%d\n", *level, num);

	for(int i = (num - 1); 0 <= i; i--){
		if(!_pv_element_remove_delete_recursive_inline(
					this->childs[i],
					level,
					error)){
			if(!error->is_error){
				error->is_error =true;
				error->level = *level;
				error->element = this->childs[i];
				break;
			}
		}
	}

	(*level)--;

	if(!_pv_element_delete_single(this)){
		error->is_error =true;
		error->level = *level;
		error->element = this;
		return false;
	}

	return true;
}

bool _pv_element_detouch_parent(PvElement * const this)
{
	if(NULL == this){
		pv_error("");
		return false;
	}

	if(NULL == this->parent){
		pv_error("");
		return false;
	}

	PvElement **childs = this->parent->childs;
	int num = pv_general_get_parray_num((void **)childs);
	bool is_exist = false;
	for(int i = 0; i < num; i++){
		if(this == childs[i]){
			is_exist = true;
			memmove(&childs[i], &childs[i + 1], sizeof(PvElement*) * (num - i));
			break;
		}
	}
	if(!is_exist){
		pv_error("");
		return false;
	}

	if(0 == num){
		free(childs);
		childs = NULL;
	}else{
		childs = (PvElement **)realloc(childs,
				sizeof(PvElement*) * (num));
		childs[num - 1] = NULL;
	}

	this->parent->childs = childs;
	this->parent = NULL;

	return true;
}

bool pv_element_remove_delete_recursive(PvElement * const this)
{
	if(NULL == this){
		pv_error("");
		return false;
	}

	// 親がいるなら連結を外す
	if(NULL != this->parent){
		if(!_pv_element_detouch_parent(this)){
			pv_error("");
			return false;
		}
	}

	PvElementRecursiveError error = {
		.is_error = false,
		.level = -1,
		.element = NULL,
	};
	int level = 0;
	if(!_pv_element_remove_delete_recursive_inline(this, &level, &error)){
		pv_error("%d\n", error.level);
		return false;
	}

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
	(data->anchor_points_num) += 1;
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

void pv_element_debug_print(const PvElement *element)
{
	if(NULL == element){
		pv_debug("");
		return;
	}
	if(PvElementKind_Bezier != element->kind){
		pv_debug("%d", element->kind);
		return;
	}


	PvElementBezierData *data = element->data;
	if(NULL == data){
		pv_debug("");
		return;
	}

	pv_debug("anchor:%d(%s)\n", data->anchor_points_num, (data->is_close)? "true":"false");

	for(int i = 0; i < data->anchor_points_num; i++){
		const PvAnchorPoint *ap = &data->anchor_points[i];
		pv_debug("%d:% 3.2f,% 3.2f, % 3.2f,% 3.2f, % 3.2f,% 3.2f, \n",
				i,
				ap->points[PvAnchorPointIndex_HandlePrev].x,
				ap->points[PvAnchorPointIndex_HandlePrev].y,
				ap->points[PvAnchorPointIndex_Point].x,
				ap->points[PvAnchorPointIndex_Point].y,
				ap->points[PvAnchorPointIndex_HandleNext].x,
				ap->points[PvAnchorPointIndex_HandleNext].y);
	}
}

