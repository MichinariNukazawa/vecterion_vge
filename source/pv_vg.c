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

	PvRect _rect = {0,0,0,0};
	this->rect = _rect;

	// Todo:ドキュメントサイズは読み込みor新規作成時に決まるので、これは仮コード
	this->rect.w = 500;
	this->rect.h = 500;
	

	return this;
}

PvElement *pv_vg_get_layer_top(PvVg *vg)
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
