#include "et_etaion.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_doc_manager.h"

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

	this->slot_change_state = NULL;
	this->slot_change_state_data = NULL;
	et_state_unfocus(&(this->state));

	current_state = this;

	return this;
}

void _et_etaion_signal_change_state(EtEtaion *this)
{
	if(NULL == this->slot_change_state){
		return;
	}

	this->slot_change_state(this->state, this->slot_change_state_data);
}


bool et_etaion_slot_mouse_action(EtDocId id_doc, EtMouseAction mouse_action)
{
	et_debug(" x:%d, y:%d,\n", (int)mouse_action.point.x, (int)mouse_action.point.y);

	EtEtaion *this = current_state;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	if(id_doc != (this->state).doc_id){
		(this->state).doc_id = id_doc;
		if(!et_doc_set_focus_to_id(id_doc, pv_focus_get_nofocus())){
			et_error("");
			return false;
		}
		_et_etaion_signal_change_state(this);
	}

	EtDoc *doc = et_doc_manager_get_doc_from_id(id_doc);
	if(NULL == doc){
		et_error("");
		return false;
	}

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id(id_doc, &is_error);
	if(is_error){
		et_error("");
		return false;
	}
	PvElement *_element = focus.element;
	if(!et_doc_add_point(doc, &_element,
				mouse_action.point.x, mouse_action.point.y)){
		et_error("");
		return false;
	}else{
		focus.element = _element;
		if(!et_doc_set_focus_to_id(id_doc, focus)){
			et_error("");
			return false;
		}
	}

	et_doc_signal_update(doc);

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
			_et_etaion_signal_change_state(this);
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
