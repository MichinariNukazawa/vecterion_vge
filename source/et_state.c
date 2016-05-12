#include "et_state.h"

#include <stdlib.h>
#include "et_error.h"

void et_state_unfocus(EtState *self)
{
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	self->doc_id = -1;
}

