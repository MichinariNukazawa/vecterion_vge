#include "et_etaion.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_doc_manager.h"
#include "et_tool_panel.h"

EtEtaion *current_state = NULL;

EtEtaion *et_etaion_init()
{
	if(NULL != current_state){
		et_bug("");
		exit(-1);
	}

	EtEtaion *this = (EtEtaion *)malloc(sizeof(EtEtaion));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	if(et_tool_get_num() <= 0){
		et_bug("");
		return NULL;
	}
	this->tool_id = 0; // default tool.

	this->slot_change_state = NULL;
	this->slot_change_state_data = NULL;
	et_state_unfocus(&(this->state));

	current_state = this;

	return this;
}

void _signal_et_etaion_change_state(EtEtaion *this)
{
	if(NULL == this->slot_change_state){
		return;
	}

	this->slot_change_state(this->state, this->slot_change_state_data);
}

bool et_etaion_set_current_doc_id(EtDocId doc_id)
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	(this->state).doc_id = doc_id;

	_signal_et_etaion_change_state(this);

	return true;
}

EtDocId et_etaion_get_current_doc_id()
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	return (this->state).doc_id;
}

static bool _et_etaion_is_extent_view = false;
bool et_etaion_set_is_extent_view(bool is_extent_view)
{
	et_debug("%s\n", (is_extent_view? "TRUE":"FALSE"));
	_et_etaion_is_extent_view = is_extent_view;

	// TODO: update all doc.
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(0 <= doc_id){
		et_doc_signal_update_from_id(doc_id);
	}
	return true;
}

bool et_etaion_get_is_extent_view(){
	return _et_etaion_is_extent_view;
}

EtToolId et_etaion_get_tool_id()
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	return this->tool_id;
}

bool et_etaion_slot_mouse_action(EtDocId doc_id, EtMouseAction mouse_action)
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	// ** change current focus doc_id
	EtDocId doc_id_prev = et_etaion_get_current_doc_id();
	if(doc_id != doc_id_prev){
		if(!et_doc_set_focus_to_id(doc_id, pv_focus_get_nofocus())){
			et_error("");
			return false;
		}
		if(!et_etaion_set_current_doc_id(doc_id)){
			et_error("");
			return false;
		}
	}

	// ** call tool_info->function
	if(this->tool_id < 0){
		et_bug("");
		return false;
	}
	const EtToolInfo *info = et_tool_get_info_from_id(this->tool_id);
	if(NULL == info){
		et_bug("");
		return false;
	}
	if(NULL == info->func_mouse_action){
		et_bug("");
		return false;
	}

	if(!info->func_mouse_action(doc_id, mouse_action)){
		et_error("");
		return false;
	}

	// ** redraw docs
	if(doc_id_prev != et_etaion_get_current_doc_id()){
		et_doc_signal_update_from_id(doc_id_prev);
	}
	et_doc_signal_update_from_id(doc_id);

	return true;
}

bool et_etaion_slot_key_action(EtKeyAction key_action)
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	et_debug("key:%d\n", key_action.key);

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id((this->state).doc_id, &is_error);
	if(is_error){
		et_error("");
		return false;
	}

	switch(key_action.key){
		case EtKey_Enter:
			if(NULL != focus.element){
				if(PvElementKind_Layer != focus.element->kind){
					focus.element = (focus.element)->parent;
					if(!et_doc_set_focus_to_id((this->state).doc_id, focus)){
						et_error("");
						return false;
					}
				}
			}
			_signal_et_etaion_change_state(this);
			break;
		default:
			et_debug("no use:%d\n", key_action.key);
			return true;
	}

	et_doc_signal_update_from_id((this->state).doc_id);

	return true;
}

int et_etaion_set_slot_change_state(EtEtaionSlotChangeState slot, gpointer data)
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	if(NULL != this->slot_change_state){
		et_bug("");
		return -1;
	}

	this->slot_change_state = slot;
	this->slot_change_state_data = data;

	return 1; // Todo: return callback id
}

// @brief 自分を含めて、親方向へLayerを探す
PvElement *_et_etaion_get_parent_layer_from_element(PvElement *element)
{
	if(NULL == element){
		return NULL;
	}

	if(PvElementKind_Layer != element->kind){
		// parent方向へLayerを探しに行く
		return _et_etaion_get_parent_layer_from_element(element->parent);
	}else{
		// 自分がLayerであれば自分を返す
		return element;
	}
}

bool et_etaion_add_new_layer(EtDocId doc_id)
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	(this->state).doc_id = doc_id;

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id((this->state).doc_id, &is_error);
	if(is_error){
		et_error("");
		return false;
	}

	PvElement *parent = NULL;
	PvElement *prev = focus.element;

	// **レイヤの追加位置を決める
	// FocusされたElementに近い親Layerを位置指定にする
	prev = _et_etaion_get_parent_layer_from_element(focus.element);
	// 位置指定から親を取る
	if(NULL != prev){
		parent = prev->parent;
	}
	// 親がNULLなら、親をroot直下に指定し、位置指定を解除
	if(NULL == parent){
		EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
		PvVg *vg = et_doc_get_vg_ref(doc);
		parent = vg->element_root;
		prev = NULL;
	}

	PvElement *layer = pv_element_new(PvElementKind_Layer);
	if(NULL == layer){
		et_error("");
		return false;
	}

	if(!pv_element_append_child(parent, prev, layer)){
		et_error("");
		return false;
	}

	focus.element = layer;
	if(!et_doc_set_focus_to_id((this->state).doc_id, focus)){
		et_error("");
		return false;
	}

	_signal_et_etaion_change_state(this);
	et_doc_signal_update_from_id((this->state).doc_id);

	return true;
}

bool et_etaion_add_new_layer_child(EtDocId doc_id)
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	(this->state).doc_id = doc_id;

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id((this->state).doc_id, &is_error);
	if(is_error){
		et_error("");
		return false;
	}

	PvElement *parent = focus.element;

	// **レイヤの追加位置を決める
	// FocusされたElementに近いLayerをparentにする
	parent = _et_etaion_get_parent_layer_from_element(focus.element);
	// 親がNULLはありえない
	if(NULL == parent){
		et_bug("");
		return false;
	}

	PvElement *layer = pv_element_new(PvElementKind_Layer);
	if(NULL == layer){
		et_error("");
		return false;
	}

	if(!pv_element_append_child(parent, NULL, layer)){
		et_error("");
		return false;
	}

	focus.element = layer;
	if(!et_doc_set_focus_to_id((this->state).doc_id, focus)){
		et_error("");
		return false;
	}

	_signal_et_etaion_change_state(this);
	et_doc_signal_update_from_id((this->state).doc_id);

	return true;
}

#include <string.h>
/** @brief focus(Layer)および子Elementを複製して親レイヤーに追加 */
bool et_etaion_copy_layer(EtDocId doc_id)
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	(this->state).doc_id = doc_id;

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id((this->state).doc_id, &is_error);
	if(is_error){
		et_error("");
		return false;
	}

	// 対象Layerを取得
	PvElement *element = _et_etaion_get_parent_layer_from_element(focus.element);
	if(NULL == element){
		unsigned long tmp = 0;
		memcpy(&tmp, &(focus.element), sizeof(tmp));
		et_error("%lx\n", tmp);
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

	focus.element = new_element_tree;
	if(!et_doc_set_focus_to_id((this->state).doc_id, focus)){
		et_error("");
		return false;
	}

	_signal_et_etaion_change_state(this);
	et_doc_signal_update_from_id((this->state).doc_id);

	return true;
}

bool et_etaion_remove_delete_layer(EtDocId doc_id)
{
	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	(this->state).doc_id = doc_id;

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id((this->state).doc_id, &is_error);
	if(is_error){
		et_error("");
		return false;
	}

	PvElement *element = _et_etaion_get_parent_layer_from_element(focus.element);
	if(NULL == element){
		et_error("");
		return false;
	}

	// 削除実行前にfocus用に親を取得しておく
	PvElement *parent = element->parent;

	bool ret = true;
	if(!pv_element_remove_delete_recursive(element)){
		et_error("");
		ret = false;
		goto error;
	}

error:

	// ** focusを変更
	// 親がNULL or rootなら、root直下をfocus指定する
	if(NULL == parent || PvElementKind_Root == parent->kind){
		EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
		PvVg *vg = et_doc_get_vg_ref(doc);
		parent = pv_vg_get_layer_top(vg);
		if(NULL == parent){
			et_error("");
			// TODO: エラー処理へ飛ばす
		}
	}

	focus.element = parent; 
	if(!et_doc_set_focus_to_id((this->state).doc_id, focus)){
		et_error("");
	}

	_signal_et_etaion_change_state(this);
	et_doc_signal_update_from_id((this->state).doc_id);

	return ret;
}

bool slot_et_etaion_change_tool(EtToolId tool_id, gpointer data)
{
	et_debug("%d\n", tool_id);

	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	if(this->tool_id != tool_id){
		et_debug("tool:%d->%d\n", this->tool_id, tool_id);
#include "et_tool_panel.h"
		if(!et_tool_panel_set_current_tool_id(tool_id)){
			et_bug("");
			return false;
		}

		this->tool_id = tool_id;
		// TODO: tool_id change after work.

		et_doc_signal_update_from_id((this->state).doc_id);
	}

	return true;
}

