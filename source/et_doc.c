#include "et_doc.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_doc_manager.h"
#include "et_doc_history.h"

struct EtDocSlotChangeInfo;
typedef struct EtDocSlotChangeInfo EtDocSlotChangeInfo;
struct EtDocSlotChangeInfo{
	int id;
	EtDocSlotChange slot;
	gpointer data;
};

typedef struct EtDoc{
	EtDocId id;
	const char *filepath;

	EtDocHistory *hist;

	EtDocSlotChangeInfo *slot_change_infos;
}EtDoc;



EtDoc *et_doc_new()
{
	PvVg *vg = pv_vg_new();
	EtDoc *doc = et_doc_new_from_vg(vg);
	pv_vg_free(vg);

	return doc;
}

EtDoc *et_doc_new_from_vg(const PvVg *vg)
{
	if(NULL == vg){
		PvVg *_vg = pv_vg_new();
		if(NULL == _vg){
			et_error("");
			return NULL;
		}
		_vg->rect.w = 1;
		_vg->rect.h = 1;
		vg = _vg;
	}

	EtDoc *self = (EtDoc *)malloc(sizeof(EtDoc));
	if(NULL == self){
		et_error("");
		return NULL;
	}

	self->id = et_doc_id_new();
	if(self->id < 0){
		et_error("");
		return NULL;
	}

	self->filepath = NULL;

	self->hist = et_doc_history_new(vg);
	if(NULL == self->hist){
		et_error("");
		return NULL;
	}

	self->slot_change_infos =
		(EtDocSlotChangeInfo *)malloc(sizeof(EtDocSlotChangeInfo) * 1);
	if(NULL == self->slot_change_infos){
		et_error("");
		return NULL;
	}
	self->slot_change_infos[0].id = -1;

	return self;
}

EtDocId et_doc_get_id(EtDoc *self)
{
	if(NULL == self){
		et_bug("");
		return -1;
	}

	return self->id;
}

bool et_doc_get_filepath(char **filepath, EtDocId doc_id)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == self){
		et_error("");
		return false;
	}

	*filepath = g_strdup(self->filepath);
	return true;
}

bool et_doc_set_filepath(EtDocId doc_id, const char *filepath)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == self){
		et_error("");
		return false;
	}

	self->filepath = g_strdup(filepath);
	return true;
}

PvVg *et_doc_get_vg_ref_from_id(EtDocId doc_id)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == self){
		et_error("%d\n", doc_id);
		return NULL;
	}

	return et_doc_get_vg_ref(self);
}

PvVg *et_doc_get_vg_ref(EtDoc *self)
{
	if(NULL == self){
		et_bug("");
		return NULL;
	}

	EtDocHistoryItem *hist_item = et_doc_history_get_from_relative(self->hist, 0);
	if(NULL == hist_item){
		et_bug("");
		return NULL;
	}
	if(NULL == hist_item->vg){
		et_bug("");
		return NULL;
	}

	return hist_item->vg;
}

int _et_doc_get_num_slot_change_infos(EtDocSlotChangeInfo *slot_change_infos){
	int i = 0;
	while(0 <= slot_change_infos[i].id){
		i++;
	}

	return i;
}

bool et_doc_signal_update(EtDoc *self)
{
	int num = _et_doc_get_num_slot_change_infos(self->slot_change_infos);
	if(0 == num){
		et_error("");
		return false;
	}

	for(int i = 0; i < num; i++){
		self->slot_change_infos[i].slot(self, self->slot_change_infos[i].data);
	}

	return true;
}

bool et_doc_signal_update_from_id(EtDocId id)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(id);
	if(NULL == self){
		et_error("");
		return false;
	}

	return et_doc_signal_update(self);
}


bool et_doc_set_image_from_file(EtDoc *self, const char *filepath)
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

	EtDocHistoryItem *hist_item = et_doc_history_get_from_relative(self->hist, 0);
	if(NULL == hist_item){
		et_bug("");
		return NULL;
	}
	PvElement *layer_top = pv_vg_get_layer_top(hist_item->vg);
	if(NULL == layer_top){
		et_error("");
		return false;
	}

	if(!pv_element_append_child(layer_top, NULL, element)){
		et_error("");
		return false;
	}

	return true;
}

EtCallbackId et_doc_add_slot_change(EtDocId doc_id, EtDocSlotChange slot, gpointer data)
{

	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == self){
		et_bug("");
		return -1;
	}

	int num = _et_doc_get_num_slot_change_infos(self->slot_change_infos);
	EtDocSlotChangeInfo *new = realloc(self->slot_change_infos,
			sizeof(EtDocSlotChangeInfo) * (num + 2));
	if(NULL == new){
		et_critical("");
		return -1;
	}
	new[num + 1].id = -1;
	new[num].id = 1; // Todo: identific number.
	new[num].slot = slot;
	new[num].data = data;

	self->slot_change_infos = new;

	// Todo: create EtCallbackId
	return new[num].id;
}

PvFocus *et_doc_get_focus_ref_from_id(EtDocId id)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(id);
	if(NULL == self){
		et_error("%d\n", id);
		goto error;
	}

	EtDocHistoryItem *hist_item = et_doc_history_get_from_relative(self->hist, 0);
	if(NULL == hist_item){
		et_bug("");
		goto error;
	}

	return &hist_item->focus;
error:
	return NULL;
}

bool et_doc_save_from_id(EtDocId doc_id)
{
	et_debug("%d", doc_id);

	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		return false;
	}

	// ** check exist difference
	if(0 < et_doc_history_get_num_undo(doc->hist)){
		EtDocHistoryItem *hist_now = et_doc_history_get_from_relative(doc->hist, 0);
		if(NULL == hist_now || NULL == hist_now->vg){
			et_bug("%p", hist_now);
			return false;
		}
		EtDocHistoryItem *hist_prev = et_doc_history_get_from_relative(doc->hist, -1);
		if(NULL == hist_prev || NULL == hist_prev->vg){
			et_bug("%p", hist_prev);
			return false;
		}
		if(!pv_vg_is_diff(hist_now->vg, hist_prev->vg)){
			return true;
		}
	}

	// ** save
	if(!et_doc_history_save_with_focus(doc->hist)){
		et_error("");
		return false;
	}

	return true;
}

bool et_doc_undo_from_id(EtDocId doc_id)
{
	et_debug("");

	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		return false;
	}

	if(!et_doc_history_undo(doc->hist)){
		et_error("");
		return false;
	}

	return true;
}

bool et_doc_redo_from_id(EtDocId doc_id)
{
	et_debug("");

	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		return false;
	}

	if(!et_doc_history_redo(doc->hist)){
		et_error("");
		return false;
	}

	return true;
}

