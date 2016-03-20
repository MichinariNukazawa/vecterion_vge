#ifndef include_ET_STATE_H
#define include_ET_STATE_H

#include <stdbool.h>
#include "et_doc_id.h"
#include "pv_element.h"

struct EtState;
typedef struct EtState EtState;
struct EtState{
	EtDocId doc_id;
};

void et_state_unfocus(EtState *self);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_STATE_H
