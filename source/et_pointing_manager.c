#include "et_pointing_manager.h"

#include <stdlib.h>
#include "et_error.h"

static EtPointingManager *pointing_manager = NULL;

EtPointingManager *et_pointing_manager_init()
{
	if(NULL != pointing_manager){
		et_bug("");
		exit(-1);
	}

	EtPointingManager *self = (EtPointingManager *)malloc(sizeof(EtPointingManager));
	if(NULL == self){
		et_critical("");
		return NULL;
	}

	self->slot_mouse_action = NULL;

	pointing_manager = self;

	return self;
}

bool et_pointing_manager_set_slot_mouse_action(EtPointingManagerSlotMouseAction slot)
{
	EtPointingManager *self = pointing_manager;
	if(NULL == self){
		et_bug("");
		return false;
	}

	if(NULL != self->slot_mouse_action){
		et_bug("");
		return false;
	}

	self->slot_mouse_action = slot;

	return true;
}

bool et_pointing_manager_slot_mouse_action(EtDocId doc_id, EtMouseAction mouse_action)
{
	EtPointingManager *self = pointing_manager;
	if(NULL == self){
		et_bug("");
		return false;
	}
	if(NULL == self->slot_mouse_action){
		et_bug("");
		return false;
	}

	if(!self->slot_mouse_action(doc_id, mouse_action)){
		et_error("");
		return false;
	}

	return true;
}

