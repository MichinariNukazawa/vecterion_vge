#ifndef __ET_CURRENT_STATE_H__
#define __ET_CURRENT_STATE_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_mouse_action.h"
#include "et_key_action.h"
#include "pv_element.h"

struct _EtCurrentState;
typedef struct _EtCurrentState EtCurrentState;

struct _EtCurrentState{
	EtDocId doc_id;
	PvElement *elementGroup;
	PvElement *element;
};

EtCurrentState *et_current_state_init();
bool et_current_state_slot_mouse_action(EtDocId id_doc, EtMouseAction mouse_action);
bool et_current_state_slot_key_action(EtKeyAction key_action);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CURRENT_STATE_H__
