#include "et_etaion.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_doc_manager.h"

EtEtaion *current_state = NULL;

void _et_etaion_unfocus(EtEtaion *this)
{
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	this->doc_id = -1;
	this->element = NULL;
}

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

	this->elementGroup = NULL;
	_et_etaion_unfocus(this);

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

	if(id_doc != this->doc_id){
		_et_etaion_unfocus(this);
		this->doc_id = id_doc;
	}

	EtDoc *doc = et_doc_manager_get_doc_from_id(id_doc);
	if(NULL == doc){
		et_error("");
		return false;
	}

	PvElement *_element = this->element; 
	if(!et_doc_add_point(doc, &_element,
			mouse_action.point.x, mouse_action.point.y)){
		et_error("");
		return false;
	}else{
		this->element = _element;
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
			_et_etaion_unfocus(this);
			break;
		default:
			et_debug("no use:%d\n", key_action.key);
	}

	return true;
}
