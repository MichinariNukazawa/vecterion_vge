#include "et_doc.h"

#include <stdlib.h>
#include "et_error.h"

EtDoc *et_doc_new()
{
	EtDoc *this = (EtDoc *)malloc(sizeof(EtDoc));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	this->id = et_doc_id_new();
	if(this->id < 0){
		et_error("");
		return NULL;
	}

	this->vg = pv_vg_new();
	if(NULL == this->vg){
		et_error("");
		return NULL;
	}

	this->slot_change_infos =
		(EtDocSlotChangeInfo *)malloc(sizeof(EtDocSlotChangeInfo) * 1);
	if(NULL == this->slot_change_infos){
		et_error("");
		return NULL;
	}
	this->slot_change_infos[0].id = -1;

	return this;
}

int _et_doc_get_num_slot_change_infos(EtDocSlotChangeInfo *slot_change_infos){
	int i = 0;
	while(0 <= slot_change_infos[i].id){
		i++;
	}

	return i;
}

bool et_doc_draw_canvas(EtDoc *this)
{
	int num = _et_doc_get_num_slot_change_infos(this->slot_change_infos);
	if(0 == num){
		et_error("");
		return false;
	}

	for(int i = 0; i < num; i++){
		this->slot_change_infos[i].slot(this, this->slot_change_infos[i].data);
	}

	return true;
}

bool et_doc_set_image_from_file(EtDoc *this, const char *filepath)
{
	PvElement *element = pv_element_new(PvElementKind_Raster);
	if(NULL == element){
		et_error("");
		return false;
	}

	if(!pv_element_raster_read_file(element, filepath)){
		et_error("");
		return false;
	}

	if(!pv_element_append_child(this->vg->element_root, 
		NULL, element)){
		et_error("");
		return false;
	}

	if(!et_doc_draw_canvas(this)){
		et_error("");
		return false;
	}

	return true;
}

EtCallbackId et_doc_add_slot_change(EtDoc *this, EtDocSlotChange slot, gpointer data)
{
	int num = _et_doc_get_num_slot_change_infos(this->slot_change_infos);
	EtDocSlotChangeInfo *new = realloc(this->slot_change_infos,
			sizeof(EtDocSlotChangeInfo) * (num + 2));
	if(NULL == new){
		return -1;
	}
	new[num + 1].id = -1;
	new[num].id = 1; // Todo: identific number.
	new[num].slot = slot;
	new[num].data = data;

	this->slot_change_infos = new;

	// Todo: create EtCallbackId
	return new[num].id;
}

bool et_doc_add_point(EtDoc *this, double x, double y)
{
	PvElement *element = pv_element_new(PvElementKind_Bezier);
	if(NULL == element){
		et_error("");
		return false;
	}

	PvAnchorPoint anchor_point = {
		.points = {{0,0}, {x, y,}, {0,0}},
	};
	if(!pv_element_bezier_add_anchor_point(element, anchor_point)){
		et_error("");
		return false;
	}

	if(!pv_element_append_child(this->vg->element_root, 
		NULL, element)){
		et_error("");
		return false;
	}

	return true;
}
