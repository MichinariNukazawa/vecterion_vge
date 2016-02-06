#ifndef __ET_CURRENT_STATE_H__
#define __ET_CURRENT_STATE_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc.h"
#include "et_mouse_action.h"

struct _EtDocNode;
typedef struct _EtDocNode EtDocNode;

struct _EtDocNode{
	EtDoc *doc;
};

struct _EtCurrentState;
typedef struct _EtCurrentState EtCurrentState;

struct _EtCurrentState{
	EtDocNode doc_nodes[16];
};

EtCurrentState *et_current_state_init();
bool et_current_state_add_doc(EtDoc *doc);
bool et_current_state_signal_mouse_action(EtDocId id_doc, EtMouseAction mouse_action);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CURRENT_STATE_H__
