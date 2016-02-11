#ifndef __ET_STATE_H__
#define __ET_STATE_H__

#include <stdbool.h>
#include "et_doc_id.h"
#include "pv_element.h"

struct _EtState;
typedef struct _EtState EtState;
struct _EtState{
	EtDocId doc_id;
	PvElement *element;
};

void et_state_unfocus(EtState *this);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_STATE_H__
