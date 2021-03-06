#include "et_etaion.h"

#include <stdlib.h>
#include <string.h>
#include "pv_element_info.h"
#include "et_error.h"
#include "et_doc_manager.h"
#include "et_tool_info.h"
#include "et_tool_panel.h"

EtEtaion *current_state = NULL;

EtEtaion *et_etaion_init()
{
	if(NULL != current_state){
		et_bug("");
		exit(-1);
	}

	EtEtaion *self = (EtEtaion *)malloc(sizeof(EtEtaion));
	if(NULL == self){
		et_error("");
		return NULL;
	}

	if(et_tool_get_num() <= 0){
		et_bug("");
		return NULL;
	}
	self->tool_id = 0; // default tool.

	self->application_path = NULL;
	self->widget_on_mouse_cursor = NULL;

	self->is_extent_view = false;
	self->is_transparent_grid = false;

	self->slot_change_states = NULL;
	self->slot_change_state_datas = NULL;
	self->slot_change_tool_ids = NULL;
	self->slot_change_tool_id_datas = NULL;
	et_state_unfocus(&(self->state));

	current_state = self;

	return self;
}

static void _signal_et_etaion_change_state(EtEtaion *self)
{
	size_t num = pv_general_get_parray_num((void **)self->slot_change_states);
	et_debug("%zu", num);
	for(int i = 0; i < (int)num; i++){
		self->slot_change_states[i](self->state, self->slot_change_state_datas[i]);
	}
}

static void _signal_et_etaion_change_tool_id(EtEtaion *self)
{
	size_t num = pv_general_get_parray_num((void **)self->slot_change_tool_ids);
	et_debug("%zu", num);
	for(int i = 0; i < (int)num; i++){
		self->slot_change_tool_ids[i](self->tool_id, self->slot_change_tool_id_datas[i]);
	}
}

bool et_etaion_set_current_doc_id(EtDocId doc_id)
{
	EtEtaion *self = current_state;
	et_assert(self);

	if((self->state).doc_id == doc_id){
		return true;
	}

	if(0 <= (self->state).doc_id){
		if(!et_doc_save_from_id((self->state).doc_id)){
			et_error("%d %d",(self->state).doc_id, doc_id);
			return false;
		}
	}
	if(0 <= doc_id){
		if(!et_doc_save_from_id(doc_id)){
			et_error("%d %d",(self->state).doc_id, doc_id);
			return false;
		}
	}
	(self->state).doc_id = doc_id;

	_signal_et_etaion_change_state(self);

	return true;
}

EtDocId et_etaion_get_current_doc_id()
{
	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	return (self->state).doc_id;
}

static const char *get_new_application_path_from_execute_path_(const char *execute_path)
{
	char *path0 = g_path_get_dirname(execute_path);
	et_assertf(path0, "'%s'", execute_path);
	const char *path1 = "..";
	if(0 != strcmp(".", path0)){
		path1 = g_path_get_dirname(path0);
		et_assertf(path1, "'%s'", execute_path);
	}
	free(path0);
	return path1;
}

void et_etaion_set_application_base_dir_from_execute_path(const char *execute_path)
{
	EtEtaion *self = current_state;
	et_assert(self);
	et_assert(execute_path);
	et_assert(NULL == self->application_path);
	self->application_path = get_new_application_path_from_execute_path_(execute_path);
	et_debug("dir:'%s'", self->application_path);
}

const char *et_etaion_get_application_base_dir()
{
#ifdef TARGET_OS_WIN
	return ".";
#else
	EtEtaion *self = current_state;
	et_assert(self);
	et_assert(self->application_path);

	return self->application_path;
#endif
}

bool et_etaion_set_is_extent_view(bool is_extent_view)
{
	EtEtaion *self = current_state;
	assert(self);

	self->is_extent_view = is_extent_view;

	// TODO: update all doc.
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(0 <= doc_id){
		et_doc_signal_update_from_id(doc_id);
	}
	return true;
}

bool et_etaion_get_is_extent_view()
{
	EtEtaion *self = current_state;
	assert(self);

	return self->is_extent_view;
}

void et_etaion_set_is_transparent_grid(bool is_transparent_grid)
{
	EtEtaion *self = current_state;
	et_assert(self);

	self->is_transparent_grid = is_transparent_grid;

	//! @todo update all doc.
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(0 <= doc_id){
		et_doc_signal_update_from_id(doc_id);
	}
}

bool et_etaion_get_is_transparent_grid()
{
	EtEtaion *self = current_state;
	et_assert(self);

	return self->is_transparent_grid;
}

EtToolId et_etaion_get_tool_id()
{
	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	return self->tool_id;
}

static bool set_mouse_cursor_(GdkCursor *cursor)
{
	EtEtaion *self = current_state;
	et_assert(self);

	et_assert(cursor);

	GdkWindow *window = gtk_widget_get_parent_window(self->widget_on_mouse_cursor);
	if(!window){
		et_debug("GdkWindow not grub");
		return false;
	}

	gdk_window_set_cursor(window, cursor);
	return true;
}

bool slot_et_etaion_from_mouse_action(EtDocId doc_id, EtMouseAction mouse_action)
{
	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	// ** change current focus doc_id
	EtDocId doc_id_prev = et_etaion_get_current_doc_id();
	if(doc_id != doc_id_prev){
		et_debug("%d, %d", doc_id, doc_id_prev);
		if(!et_etaion_set_current_doc_id(doc_id)){
			et_error("");
			return false;
		}
	}

	// ** call tool_info->function
	if(self->tool_id < 0){
		et_bug("");
		return false;
	}
	const EtToolInfo *tool_info = et_tool_get_info_from_id(self->tool_id);
	et_assert(tool_info);
	et_assert(tool_info->func_mouse_action);

	PvDocumentPreference document_preference = et_doc_get_document_preference_from_id(doc_id);
	if(0 != (mouse_action.state & GDK_SHIFT_MASK)){
		document_preference.snap_context.is_snap_for_degree = true;
	}
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	if(!tool_info->func_mouse_action(doc_id, document_preference, mouse_action, &edit_draw_element, &cursor)){
		et_error("");
		return false;
	}

	et_doc_set_element_group_edit_draw_from_id(doc_id, edit_draw_element);
	if(NULL != edit_draw_element){
		pv_element_remove_free_recursive(edit_draw_element);
	}

	if(!cursor){
		cursor = tool_info->mouse_cursor;
	}
	et_assert(cursor);
	et_assert(set_mouse_cursor_(cursor));

	/*
	   if(EtMouseAction_Up == mouse_action.action){
	   et_doc_save_from_id(doc_id);
	   }
	   */

	// ** redraw doc
	et_doc_signal_update_from_id(doc_id);

	return true;
}

bool slot_et_etaion_from_key_action(EtKeyAction key_action)
{
	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	// et_debug("key:%04x", key_action.key);

	if(0 > (self->state).doc_id){
		et_debug("no current document. %d", (self->state).doc_id);
		return true;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id((self->state).doc_id);
	if(NULL == focus){
		et_error("");
		return false;
	}

	//! key list: https://git.gnome.org/browse/gtk+/plain/gdk/gdkkeysyms.h
	switch(key_action.key){
		default:
			et_debug("no use:%d", key_action.key);
			return true;
	}

	et_doc_signal_update_from_id((self->state).doc_id);

	return true;
}

int et_etaion_add_slot_change_state(EtEtaionSlotChangeState slot, gpointer data)
{
	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	size_t num = pv_general_get_parray_num((void **)self->slot_change_states);
	EtEtaionSlotChangeState *new_slots = realloc(self->slot_change_states,
			sizeof(self->slot_change_states[0]) * (num + 2));
	gpointer *new_datas = realloc(self->slot_change_state_datas,
			sizeof(self->slot_change_state_datas[0]) * (num + 2));
	new_slots[num] = slot;
	new_slots[num + 1] = NULL;
	new_datas[num] = data;
	new_datas[num + 1] = NULL;
	self->slot_change_states = new_slots;
	self->slot_change_state_datas = new_datas;

	return 1; //! @todo return callback id
}

int et_etaion_add_slot_change_tool_id(EtEtaionSlotChangeToolId slot, gpointer data)
{
	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	size_t num = pv_general_get_parray_num((void **)self->slot_change_tool_ids);
	EtEtaionSlotChangeToolId *new_slots = realloc(
			self->slot_change_tool_ids,
			sizeof(self->slot_change_tool_ids[0]) * (num + 2));
	gpointer *new_datas = realloc(
			self->slot_change_tool_id_datas,
			sizeof(self->slot_change_tool_id_datas[0]) * (num + 2));
	new_slots[num] = slot;
	new_slots[num + 1] = NULL;
	new_datas[num] = data;
	new_datas[num + 1] = NULL;
	self->slot_change_tool_ids = new_slots;
	self->slot_change_tool_id_datas = new_datas;

	return 1; //! @todo return callback id
}

bool et_etaion_append_new_layer(EtDocId doc_id)
{
	EtEtaion *self = current_state;
	et_assert(self);

	(self->state).doc_id = doc_id;

	PvFocus *focus = et_doc_get_focus_ref_from_id((self->state).doc_id);
	et_assertf(focus, "%d", doc_id);

	PvElement *sister_layer = pv_focus_get_first_layer(focus);
	et_assertf(sister_layer->parent, "%d", doc_id);

	PvElement *layer = pv_element_new(PvElementKind_Layer);
	et_assert(layer);

	bool ret = pv_element_append_child(sister_layer->parent, sister_layer, layer);
	et_assert(ret);

	pv_focus_clear_set_element(focus, layer);

	_signal_et_etaion_change_state(self);

	et_doc_save_from_id((self->state).doc_id);
	et_doc_signal_update_from_id((self->state).doc_id);

	return true;
}

bool et_etaion_append_new_layer_child(EtDocId doc_id)
{
	EtEtaion *self = current_state;
	et_assert(self);

	(self->state).doc_id = doc_id;

	PvFocus *focus = et_doc_get_focus_ref_from_id((self->state).doc_id);
	et_assertf(focus, "%d", doc_id);

	PvElement *parent = pv_focus_get_first_layer(focus);
	et_assertf(parent, "%d", doc_id);

	PvElement *layer = pv_element_new(PvElementKind_Layer);
	et_assert(layer);

	bool ret = pv_element_append_child(parent, NULL, layer);
	et_assert(ret);

	pv_focus_clear_set_element(focus, layer);

	_signal_et_etaion_change_state(self);

	et_doc_save_from_id((self->state).doc_id);
	et_doc_signal_update_from_id((self->state).doc_id);

	return true;
}

#include <string.h>
/** @brief focus(Layer)および子Elementを複製して親レイヤーに追加 */
bool et_etaion_copy_layer(EtDocId doc_id)
{
	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	(self->state).doc_id = doc_id;

	PvFocus *focus = et_doc_get_focus_ref_from_id((self->state).doc_id);
	if(NULL == focus){
		et_error("");
		return false;
	}

	// 対象Layerを取得
	PvElement *element = pv_focus_get_first_element(focus);
	if(NULL == element){
		et_error("%p", element);
		return false;
	}

	// 親がNULLはありえない
	if(NULL == element->parent){
		unsigned long tmp = 0;
		memcpy(&tmp, &element, sizeof(tmp));
		et_bug("%lx", tmp);
		return false;
	}

	// LayerTreeを複製
	PvElement *new_element_tree = pv_element_copy_recursive(element);
	if(NULL == new_element_tree){
		et_error("");
		return false;
	}

	if(!pv_element_append_child(element->parent, 
				element, new_element_tree)){
		et_error("");
		return false;
	}

	pv_focus_clear_set_element(focus, new_element_tree);

	_signal_et_etaion_change_state(self);

	et_doc_save_from_id((self->state).doc_id);
	et_doc_signal_update_from_id((self->state).doc_id);

	return true;
}

//! @return elder sister -> younger sister -> null
static PvElement *pv_element_get_sister_(PvElement *element)
{
	if(NULL == element->parent){
		return NULL;
	}

	size_t num = pv_general_get_parray_num((void **)element->parent->childs);
	if(1 >= num){
		return NULL;
	}
	for(int i = 0; i < (int)num; i++){
		if(element == element->parent->childs[i]){
			if(i == 0){
				return element->parent->childs[1];
			}else{
				return element->parent->childs[i - 1];
			}
		}
	}

	et_bug("");
	return NULL;
}

bool et_etaion_remove_delete_layer(EtDocId doc_id)
{
	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	(self->state).doc_id = doc_id;

	PvFocus *focus = et_doc_get_focus_ref_from_id((self->state).doc_id);
	et_assertf(focus, "%d", doc_id);

	PvElement *layer_element = pv_focus_get_first_layer(focus);
	et_assert(layer_element);
	et_assertf(PvElementKind_Root != layer_element->kind, "%d", doc_id);

	PvElement *sister = pv_element_get_sister_(layer_element);
	if(NULL == sister){
		//! @todo focus to root is unacceptable
		pv_focus_clear_set_element(focus, layer_element->parent);
	}else{
		pv_focus_clear_set_element(focus, sister);
	}

	bool ret = pv_element_remove_free_recursive(layer_element);
	et_assertf(ret, "%d", doc_id);

	//! focus resetting to not root
	PvElement *element_first = pv_focus_get_first_element(focus);
	if(PvElementKind_Root == element_first->kind){
		if(0 == pv_general_get_parray_num((void **)element_first->childs)){
			PvElement *layer = pv_element_new(PvElementKind_Layer);
			et_assert(layer);
			pv_element_append_child(element_first, NULL, layer);
		}
		pv_focus_clear_set_element(focus, element_first->childs[0]);
	}

	_signal_et_etaion_change_state(self);

	et_doc_save_from_id((self->state).doc_id);
	et_doc_signal_update_from_id((self->state).doc_id);

	return true;
}

bool et_etaion_remove_delete_by_focusing(EtDocId doc_id)
{
	EtEtaion *self = current_state;
	et_assert(self);

	(self->state).doc_id = doc_id;

	PvFocus *focus = et_doc_get_focus_ref_from_id((self->state).doc_id);
	et_assertf(focus, "%d", doc_id);

	PvElement *first_element = pv_focus_get_first_element(focus);
	if(!pv_element_kind_is_object(first_element->kind)){
		et_debug("");
		return true;
	}

	const EtToolInfo *tool_info = et_tool_get_info_from_id(current_state->tool_id);
	et_assertf(tool_info, "%u", current_state->tool_id);

	if(tool_info->is_element_tool){
		size_t num = pv_general_get_parray_num((void **)focus->elements);
		for(int i = ((int)num) - 1; 0 <= i; i--){
			if(!pv_element_kind_is_object(focus->elements[i]->kind)){
				et_bug("");
				goto failure;
			}

			bool ret;
			PvElement *element = NULL;
			if(0 == i){
				element = pv_focus_get_first_element(focus);
				ret = pv_focus_clear_to_first_layer(focus);
				et_assertf(ret, "%d", doc_id);
				et_assert(element != pv_focus_get_first_layer(focus));
			}else{
				element = focus->elements[i];
				ret = pv_focus_remove_element(focus, element);
				et_assertf(ret, "%d", doc_id);
			}

			ret = pv_element_remove_free_recursive(element);
			et_assertf(ret, "%d", doc_id);
		}
	}else{
		size_t num = pv_general_get_parray_num((void **)focus->anchor_points);
		PvAnchorPoint **anchor_points = NULL;
		if(0 != num){
			anchor_points = malloc(sizeof(PvAnchorPoint *) * (num + 1));
			et_assert(anchor_points);
			memcpy(anchor_points, focus->anchor_points, sizeof(PvAnchorPoint *) * (num + 1));
		}

		for(int i = ((int)num) - 1; 0 <= i; i--){
			PvAnchorPoint *anchor_point = anchor_points[i];
			PvElement *element = pv_element_get_in_elements_from_member_anchor_point(
					focus->elements,
					anchor_point);
			et_assert(element);
			const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
			et_assert(info);

			pv_focus_remove_anchor_point(focus, element, anchor_point);
			bool is_delete = false;
			PvElement *foot_element = NULL;
			info->func_remove_delete_anchor_point(element, anchor_point, &foot_element, &is_delete);

			if(NULL != foot_element){
				pv_focus_add_element(focus, foot_element);
			}
			if(is_delete){
				pv_focus_remove_element(focus, element);
			}else{
				pv_focus_add_element(focus, element); // to top focus.
			}
		}

		free(anchor_points);
	}

failure:
	_signal_et_etaion_change_state(self);

	et_doc_save_from_id((self->state).doc_id);
	et_doc_signal_update_from_id((self->state).doc_id);

	return true;
}

bool slot_et_etaion_change_tool(EtToolId tool_id, gpointer data)
{
	et_debug("%d", tool_id);

	EtEtaion *self = current_state;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	if(!et_doc_save_from_id(et_etaion_get_current_doc_id())){
		et_error("");
		return false;
	}

	if(self->tool_id != tool_id){
		et_debug("tool:%d->%d", self->tool_id, tool_id);

		self->tool_id = tool_id;

		bool ret = et_tool_panel_set_current_tool_id(tool_id);
		et_assertf(ret, "%d", tool_id);

		_signal_et_etaion_change_tool_id(self);

		et_doc_save_from_id((self->state).doc_id);
		et_doc_signal_update_from_id((self->state).doc_id);
	}

	return true;
}

void et_etaion_set_widget_on_mouse_cursor(GtkWidget *widget)
{
	EtEtaion *self = current_state;
	et_assert(self);

	et_assert(NULL == self->widget_on_mouse_cursor);
	self->widget_on_mouse_cursor = widget;
}

