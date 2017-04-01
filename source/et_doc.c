#include "et_doc.h"

#include <stdlib.h>
#include <string.h>
#include "et_error.h"
#include "et_doc_manager.h"
#include "et_doc_history_hive.h"

struct EtDocSlotChangeInfo;
typedef struct EtDocSlotChangeInfo EtDocSlotChangeInfo;
struct EtDocSlotChangeInfo{
	int id;
	EtDocSlotChange slot;
	gpointer data;
};

typedef struct EtDoc{
	EtDocId id;
	char *filepath;
	PvVg *latest_saved_vg;

	PvElement *element_group_edit_draw;

	EtDocHistoryHive *history_hive;

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
	self->element_group_edit_draw = NULL;

	self->history_hive = et_doc_history_hive_new(vg);
	if(NULL == self->history_hive){
		et_error("");
		return NULL;
	}

	// new document has not save needed difference
	const EtDocHistory *hist = et_doc_history_hive_get_current(self->history_hive);
	et_assert(hist);
	et_assert(hist->vg);
	self->latest_saved_vg = pv_vg_copy_new(hist->vg);

	self->slot_change_infos =
		(EtDocSlotChangeInfo *)malloc(sizeof(EtDocSlotChangeInfo) * 1);
	if(NULL == self->slot_change_infos){
		et_error("");
		return NULL;
	}
	self->slot_change_infos[0].id = -1;

	return self;
}

void et_doc_delete(EtDoc *self)
{
	if(NULL != self->slot_change_infos){
		free(self->slot_change_infos);
	}
	et_doc_history_hive_free(self->history_hive);
	if(NULL != self->filepath){
		g_free(self->filepath);
	}
	free(self);
}

EtDocId et_doc_get_id(EtDoc *self)
{
	if(NULL == self){
		et_bug("");
		return -1;
	}

	return self->id;
}

char *et_doc_get_new_filename_from_id(EtDocId doc_id)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(self, "%d", doc_id);

	if(NULL == self->filepath){
		return NULL;
	}

	char *sep = strrchr(self->filepath, '/');
	if(NULL == sep){
		return NULL;
	}

	sep++;
	return g_strdup(sep);
}

bool et_doc_get_saved_filepath(char **filepath, EtDocId doc_id)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(self, "%d", doc_id);

	*filepath = g_strdup(self->filepath);
	return true;
}

bool et_doc_set_saved_filepath(EtDocId doc_id, const char *filepath)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == self){
		et_error("");
		return false;
	}

	self->filepath = g_strdup(filepath);

	et_doc_save_from_id(doc_id);

	if(NULL != self->filepath){
		const EtDocHistory *hist = et_doc_history_hive_get_current(self->history_hive);
		if(NULL != self->latest_saved_vg){
			pv_vg_free(self->latest_saved_vg);
			self->latest_saved_vg = NULL;
		}
		self->latest_saved_vg = pv_vg_copy_new(hist->vg);
	}

	et_debug("%s", ((self->filepath) ? self->filepath : "(null)"));

	return et_doc_signal_update(self);
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

	EtDocHistory *hist_item = et_doc_history_get_from_relative(self->history_hive, 0);
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

static int _et_doc_get_num_slot_change_infos(
		const EtDocSlotChangeInfo *slot_change_infos)
{
	int i = 0;
	while(0 <= slot_change_infos[i].id){
		i++;
	}

	return i;
}

bool et_doc_signal_update(EtDoc *self)
{
	// @todo check document change.

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

bool et_doc_signal_update_from_id(EtDocId doc_id)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == self){
		et_error("%d", doc_id);
		return false;
	}

	return et_doc_signal_update(self);
}


bool et_doc_set_image_from_file(EtDoc *self, const char *filepath)
{
	PvElement *element = pv_element_basic_shape_new_from_filepath(filepath);
	if(NULL == element){
		et_error("");
		return false;
	}

	EtDocHistory *hist_item = et_doc_history_get_from_relative(self->history_hive, 0);
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

	EtDocHistory *hist_item = et_doc_history_get_from_relative(self->history_hive, 0);
	if(NULL == hist_item){
		et_bug("");
		goto error;
	}

	return hist_item->focus;
error:
	return NULL;
}

void et_doc_set_element_group_edit_draw_from_id(EtDocId doc_id, const PvElement *element_group)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(doc, "%d", doc_id);

	if(doc->element_group_edit_draw){
		pv_element_remove_free_recursive(doc->element_group_edit_draw);
	}

	if(!element_group){
		doc->element_group_edit_draw = NULL;
	}else{
		doc->element_group_edit_draw = pv_element_copy_recursive(element_group);
	}
}

PvElement *et_doc_get_element_group_edit_draw_from_id(EtDocId doc_id)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(doc, "%d", doc_id);

	return doc->element_group_edit_draw;
}

bool et_doc_save_from_id(EtDocId doc_id)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_bug("%d", doc_id);
		return false;
	}

	// ** check exist difference
	if(0 < et_doc_history_hive_get_num_undo(doc->history_hive)){
		EtDocHistory *hist_now = et_doc_history_get_from_relative(doc->history_hive, 0);
		if(NULL == hist_now || NULL == hist_now->vg){
			et_bug("%p", hist_now);
			return false;
		}
		EtDocHistory *hist_prev = et_doc_history_get_from_relative(doc->history_hive, -1);
		if(NULL == hist_prev || NULL == hist_prev->vg){
			et_bug("%p", hist_prev);
			return false;
		}
		if(!pv_vg_is_diff(hist_now->vg, hist_prev->vg)){
			return true;
		}
	}

	// ** save
	if(!et_doc_history_hive_save_with_focus(doc->history_hive)){
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

	if(!et_doc_history_hive_undo(doc->history_hive)){
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

	if(!et_doc_history_hive_redo(doc->history_hive)){
		et_error("");
		return false;
	}

	return true;
}

bool et_doc_is_saved_from_id(EtDocId doc_id)
{
	EtDoc *self = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(self, "%d", doc_id);
	
	if(! self->latest_saved_vg){
		return false;
	}

	// check saved is matching to document content.
	const EtDocHistory *work_hist = et_doc_history_get_from_relative(self->history_hive, 0);
	return (! pv_vg_is_diff(work_hist->vg, self->latest_saved_vg));
}

