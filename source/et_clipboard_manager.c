#include "et_clipboard_manager.h"

#include "pv_focus.h"
#include "et_doc.h"
#include "et_error.h"

struct EtClipboardManager{
	PvElement *element_buffer;
};

EtClipboardManager *clipboard_manager = NULL;

EtClipboardManager *et_clipboard_manager_init()
{
	et_assert(NULL == clipboard_manager);

	EtClipboardManager *self = (EtClipboardManager *)malloc(sizeof(EtClipboardManager));
	et_assert(self);

	self->element_buffer = NULL;

	clipboard_manager = self;

	return self;
}

bool et_clipboard_cut_from_doc_id(EtDocId doc_id)
{
	EtClipboardManager *self = clipboard_manager;
	et_assert(self);

	if(0 > doc_id){
		return false;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	pv_element_remove_free_recursive(self->element_buffer);
	self->element_buffer = NULL;

	self->element_buffer = pv_element_new(PvElementKind_Group);
	et_assert(self->element_buffer);

	if(! pv_focus_is_focused(focus)){
		return true;
	}

	PvElement *layer = pv_focus_get_first_layer(focus);

	size_t num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num; i++){
		bool ret;
		PvElement *element = focus->elements[i];
		et_assert(element);
		ret = pv_element_remove(element);
		et_assert(ret);
		ret = pv_element_append_child(self->element_buffer, NULL, element);
		et_assert(ret);
	}
	
	bool ret = pv_focus_clear_set_element(focus, layer);
	et_assert(ret);

	return true;
}

bool et_clipboard_copy_from_doc_id(EtDocId doc_id)
{
	EtClipboardManager *self = clipboard_manager;
	et_assert(self);

	if(0 > doc_id){
		return false;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	pv_element_remove_free_recursive(self->element_buffer);
	self->element_buffer = NULL;

	self->element_buffer = pv_element_new(PvElementKind_Group);
	et_assert(self->element_buffer);

	if(! pv_focus_is_focused(focus)){
		return true;
	}

	size_t num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num; i++){
		PvElement *element = pv_element_copy_recursive(focus->elements[i]);
		et_assert(element);
		pv_element_append_child(self->element_buffer, NULL, element);
	}

	return true;
}

bool et_clipboard_paste_from_doc_id(EtDocId doc_id)
{
	EtClipboardManager *self = clipboard_manager;
	et_assert(self);

	if(0 > doc_id){
		return false;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvElement *layer = pv_focus_get_first_layer(focus);

	if(NULL == self->element_buffer){
		return true;
	}

	size_t num = pv_general_get_parray_num((void **)self->element_buffer->childs);

	if(0 < num){
		pv_focus_clear_to_first_layer(focus);
	}

	for(int i = 0; i < (int)num; i++){
		PvElement *element = pv_element_copy_recursive(self->element_buffer->childs[i]);
		pv_element_append_child(layer, NULL, element);
		pv_focus_add_element(focus, element);
	}

	return true;
}

