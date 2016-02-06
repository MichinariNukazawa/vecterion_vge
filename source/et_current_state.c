#include "et_current_state.h"

#include <stdlib.h>
#include "et_error.h"

EtCurrentState *current_state = NULL;

EtCurrentState *et_current_state_init()
{
	EtCurrentState *this;
	this = (EtCurrentState *)malloc(sizeof(EtCurrentState));
	if(NULL == this){
		et_critical("");
		return NULL;
	}

	this->doc_nodes[0].doc = NULL;

	current_state = this;

	return this;
}

int _et_current_state_get_num_doc_node(EtDocNode *doc_nodes)
{
	int i = 0;
	while(NULL != doc_nodes[i].doc){
		i++;
	}

	// Todo: fix freescale
	if(14 < i){
		et_fixme("");
		i = 14;
	}

	return i;
}

bool et_current_state_add_doc(EtDoc *doc)
{
	EtCurrentState *this = current_state;
	if(NULL == this){
		et_bug("");
		return false;
	}

	if(NULL == this->doc_nodes){
		et_bug("");
		return false;
	}

	int num = _et_current_state_get_num_doc_node(this->doc_nodes);
	this->doc_nodes[num].doc = doc;

	return true;
}

EtDoc *_et_current_state_get_doc_from_id(const EtDocId doc_id)
{
	EtCurrentState *this = current_state;
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	int i = 0;
	while(NULL != this->doc_nodes[i].doc){
		if(doc_id == this->doc_nodes[i].doc->id){
			return this->doc_nodes[i].doc;
		}
		i++;
	}

	return NULL;
}


bool et_current_state_signal_mouse_action(EtDocId id_doc, EtMouseAction mouse_action)
{
	et_debug(" x:%d, y:%d,\n", (int)mouse_action.point.x, (int)mouse_action.point.y);
	EtDoc *doc = _et_current_state_get_doc_from_id(id_doc);
	if(NULL == doc){
		et_error("");
		return false;
	}

	et_doc_add_point(doc, mouse_action.point.x, mouse_action.point.y);
	et_doc_draw_canvas(doc);

	return true;
}

