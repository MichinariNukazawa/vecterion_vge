#ifndef include_ET_STATE_H
#define include_ET_STATE_H

#include <stdbool.h>
#include "et_doc_id.h"
#include "pv_element.h"

struct _EtState;
typedef struct _EtState EtState;
struct _EtState{
	EtDocId doc_id;
};

void et_state_unfocus(EtState *this);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_STATE_H
