#ifndef include_ET_DOC_H
#define include_ET_DOC_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "pv_vg.h"
#include "pv_focus.h"

struct EtDoc;
typedef struct EtDoc EtDoc;

typedef int EtCallbackId;
typedef void (*EtDocSlotChange)(EtDoc *doc, gpointer data);



EtDoc *et_doc_new();
EtDoc *et_doc_new_from_vg(const PvVg *vg);
EtDocId et_doc_get_id(EtDoc *self);
bool et_doc_get_filepath(char **filepath, EtDocId doc_id);
bool et_doc_set_filepath(EtDocId doc_id, const char *filepath);
PvVg *et_doc_get_vg_ref(EtDoc *self);
PvVg *et_doc_get_vg_ref_from_id(EtDocId doc_id);
bool et_doc_set_image_from_file(EtDoc *self, const char *filepath);
EtCallbackId et_doc_add_slot_change(EtDocId doc_id, EtDocSlotChange slot, gpointer data);
// Notice: they methods change signale not notify.
PvFocus *et_doc_get_focus_ref_from_id(EtDocId id);
// PvFocus et_doc_get_focus(EtDoc *self);
// void et_doc_set_focus(EtDoc *self, PvFocus focus);
bool et_doc_signal_update(EtDoc *self);
bool et_doc_signal_update_from_id(EtDocId id);

bool et_doc_save_from_id(EtDocId doc_id);
bool et_doc_undo_from_id(EtDocId doc_id);
bool et_doc_redo_from_id(EtDocId doc_id);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOC_H
