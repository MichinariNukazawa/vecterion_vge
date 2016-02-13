#include "et_state.h"

#include <stdlib.h>
#include "et_error.h"

void et_state_unfocus(EtState *this)
{
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	this->doc_id = -1;
}
