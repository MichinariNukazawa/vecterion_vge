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

	et_state_unfocus(&(this->state));

	current_state = this;

	return this;
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
		et_state_unfocus(&(this->state));
		(this->state).doc_id = id_doc;
	}

	EtDoc *doc = et_doc_manager_get_doc_from_id(id_doc);
	if(NULL == doc){
		et_error("");
		return false;
	}

	PvElement *_element = (this->state).element; 
	if(!et_doc_add_point(doc, &_element,
				mouse_action.point.x, mouse_action.point.y)){
		et_error("");
		return false;
	}else{
		(this->state).element = _element;
	}

	et_doc_draw_canvas(doc);

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

	switch(key_action.key){
		case EtKey_Enter:
			et_state_unfocus(&(this->state));
			break;
		default:
			et_debug("no use:%d\n", key_action.key);
	}

	return true;
}
