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

	this->element_root = pv_element_new(PvElementKind_Root);
	if(NULL == this->element_root){
		pv_error("");
		return NULL;
	}

	PvElement *layer = pv_element_new(PvElementKind_Layer);
	if(NULL == layer){
		pv_error("");
		return NULL;
	}

	if(!pv_element_append_child(this->element_root, NULL, layer)){
		pv_error("");
		return NULL;
	}

	PvRect _rect = {0,0,1,1};
	this->rect = _rect;

	return this;
}

void pv_vg_free(PvVg *vg)
{
	// TODO: not implement.
}

PvElement *pv_vg_get_layer_top(const PvVg *vg)
{
	if(NULL == vg){
		pv_error("");
		return NULL;
	}

	if(NULL == vg->element_root){
		pv_error("");
		return NULL;
	}

	if(1 > pv_general_get_parray_num((void **)(vg->element_root->childs))){
		pv_error("");
		return NULL;
	}

	return vg->element_root->childs[0];
}

bool pv_vg_copy_overwrite(PvVg *dst, const PvVg *src)
{
	if(NULL == dst){
		pv_error("");
		return false;
	}
	if(NULL == src){
		pv_error("");
		return false;
	}

	if(!pv_element_remove_delete_recursive(dst->element_root)){
		pv_error("");
		return false;
	}
	dst->element_root = NULL;

	pv_debug("%f\n", src->rect.x);
	PvRect rect = src->rect;
	dst->rect = rect;
//	dst->rect = src->rect;

	PvElement *new_element_root = pv_element_copy_recursive(src->element_root);
	if(NULL == new_element_root){
		pv_error("");
		return false;
	}
	dst->element_root = new_element_root;

return true;
}

