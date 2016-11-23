#include "pv_vg.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"

PvVg *pv_vg_new()
{
	PvVg *self = (PvVg *)malloc(sizeof(PvVg));
	if(NULL == self){
		pv_error("");
		return NULL;
	}

	self->element_root = pv_element_new(PvElementKind_Root);
	if(NULL == self->element_root){
		pv_error("");
		return NULL;
	}

	PvElement *layer = pv_element_new(PvElementKind_Layer);
	if(NULL == layer){
		pv_error("");
		return NULL;
	}

	if(!pv_element_append_child(self->element_root, NULL, layer)){
		pv_error("");
		return NULL;
	}

	PvRect _rect = {0,0,1,1};
	self->rect = _rect;

	return self;
}

void pv_vg_free(PvVg *self)
{
	pv_assert(pv_element_remove_free_recursive(self->element_root));
	free(self);
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

	if(!pv_element_remove_free_recursive(dst->element_root)){
		pv_error("");
		return false;
	}
	dst->element_root = NULL;

	dst->rect = src->rect;

	PvElement *new_element_root = pv_element_copy_recursive(src->element_root);
	if(NULL == new_element_root){
		pv_error("");
		return false;
	}
	dst->element_root = new_element_root;

	return true;
}

PvVg *pv_vg_copy_new(const PvVg *src)
{
	if(NULL == src){
		pv_bug("");
		return NULL;
	}

	PvVg *vg_new = pv_vg_new();
	if(NULL == vg_new){
		pv_bug("");
		return NULL;
	}

	if(!pv_vg_copy_overwrite(vg_new, src)){
		pv_error("");
		return NULL;
	}

	// TODO not implement.
	return vg_new;
}

static bool _pv_vg_is_diff_rect(PvRect rect0, PvRect rect1)
{
	return !(rect0.x == rect1.x
			&& rect0.y == rect1.y
			&& rect0.w == rect1.w
			&& rect0.h == rect1.h
		);
}

bool pv_vg_is_diff(const PvVg *vg0, const PvVg *vg1)
{
	if(NULL == vg0 && NULL == vg1){
		pv_bug("");
		return false; // not difference.
	}

	if(NULL == vg0 || NULL == vg1){
		pv_bug("%p,%p", vg0, vg1);
		return false; // not difference.
	}

	if(vg0 == vg1){
		pv_debug("");
		return false;
	}

	if(_pv_vg_is_diff_rect(vg0->rect, vg1->rect)){
		pv_debug("");
		return true;
	}

	if(pv_element_is_diff_recursive(vg0->element_root, vg1->element_root)){
		return true;
	}else{
		return false;
	}
}

