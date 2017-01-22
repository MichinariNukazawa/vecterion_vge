#ifndef include_ET_POINTING_MANAGER_H
#define include_ET_POINTING_MANAGER_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_mouse_action.h"

struct EtPointingManager;
typedef struct EtPointingManager EtPointingManager;

typedef bool (*EtPointingManagerSignalMouseAction)(EtDocId doc_id, EtMouseAction mouse_action);
typedef bool (*EtPointingManagerSlotMouseAction)(EtDocId doc_id, EtMouseAction mouse_action);

struct EtPointingManager{
	EtPointingManagerSlotMouseAction *slot_mouse_actions;
};

EtPointingManager *et_pointing_manager_init();
bool et_pointing_manager_add_slot_mouse_action(EtPointingManagerSlotMouseAction slot);
bool slot_et_pointing_manager_from_mouse_action(EtDocId doc_id, EtMouseAction mouse_action);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_POINTING_MANAGER_H
