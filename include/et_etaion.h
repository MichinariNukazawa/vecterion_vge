#ifndef __ET_ETAION_H__
#define __ET_ETAION_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_mouse_action.h"
#include "et_key_action.h"
#include "pv_element.h"
#include "et_state.h"

struct _EtEtaion;
typedef struct _EtEtaion EtEtaion;

typedef void (*EtEtaionSlotChangeState)(EtState state, gpointer data);

struct _EtEtaion{
	EtState state;

	EtEtaionSlotChangeState slot_change_state;
	gpointer slot_change_state_data;
};

EtEtaion *et_etaion_init();
bool et_etaion_slot_mouse_action(EtDocId id_doc, EtMouseAction mouse_action);
bool et_etaion_slot_key_action(EtKeyAction key_action);
int et_etaion_set_slot_change_state(EtEtaionSlotChangeState slot, gpointer data);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_ETAION_H__
