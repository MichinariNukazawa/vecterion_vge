#include "et_current_state.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_doc_manager.h"

EtCurrentState *current_state = NULL;
EtCurrentState *et_current_state_init()
{
	if(NULL != current_state){
		et_bug("");
		exit(-1);
	}

	EtCurrentState *this = (EtCurrentState *)malloc(sizeof(EtCurrentState));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	current_state = this;

	return this;
}

bool et_current_state_slot_mouse_action(EtDocId id_doc, EtMouseAction mouse_action)
{
	et_debug(" x:%d, y:%d,\n", (int)mouse_action.point.x, (int)mouse_action.point.y);
	EtDoc *doc = et_doc_manager_get_doc_from_id(id_doc);
	if(NULL == doc){
		et_error("");
		return false;
	}

	et_doc_add_point(doc, mouse_action.point.x, mouse_action.point.y);
	et_doc_draw_canvas(doc);

	return true;
}

