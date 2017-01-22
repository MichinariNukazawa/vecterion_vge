#include "et_pointing_manager.h"

#include <stdlib.h>
#include <math.h>
#include "et_error.h"
#include "pv_general.h"

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

	self->slot_mouse_actions = NULL;

	pointing_manager = self;

	return self;
}

bool et_pointing_manager_add_slot_mouse_action(EtPointingManagerSlotMouseAction slot)
{
	EtPointingManager *self = pointing_manager;
	assert(self);

	int num = pv_general_get_parray_num((void *)self->slot_mouse_actions);
	EtPointingManagerSlotMouseAction *new_array = realloc(
			self->slot_mouse_actions,
			sizeof(EtPointingManagerSlotMouseAction) * (num + 2));
	assert(new_array);
	new_array[num + 1] = NULL;
	new_array[num + 0] = slot;

	self->slot_mouse_actions = new_array;

	return true;
}

bool slot_et_pointing_manager_from_mouse_action(EtDocId doc_id, EtMouseAction mouse_action)
{
	EtPointingManager *self = pointing_manager;
	assert(self);
	assert(self->slot_mouse_actions);

	int num = pv_general_get_parray_num((void *)self->slot_mouse_actions);
	for(int i = 0; i < num; i++){
		assert(self->slot_mouse_actions[i]);
		if(!self->slot_mouse_actions[i](doc_id, mouse_action)){
			et_error("");
		}
	}

	return true;
}

