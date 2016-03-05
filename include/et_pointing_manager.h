#ifndef include_ET_POINTING_MANAGER_H
#define include_ET_POINTING_MANAGER_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_mouse_action.h"

struct _EtPointingManager;
typedef struct _EtPointingManager EtPointingManager;

typedef bool (*EtPointingManagerSignalMouseAction)(EtDocId id_doc, EtMouseAction mouse_action);
typedef bool (*EtPointingManagerSlotMouseAction)(EtDocId id_doc, EtMouseAction mouse_action);

struct _EtPointingManager{
	EtPointingManagerSlotMouseAction slot_mouse_action;
};

EtPointingManager *et_pointing_manager_init();
bool et_pointing_manager_set_slot_mouse_action(EtPointingManagerSlotMouseAction slot);
bool et_pointing_manager_slot_mouse_action(EtDocId id_doc, EtMouseAction mouse_action);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_POINTING_MANAGER_H
