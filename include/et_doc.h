#ifndef include_ET_DOC_H
#define include_ET_DOC_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_point.h"
#include "pv_vg.h"
#include "pv_focus.h"

struct EtDoc;
typedef struct EtDoc EtDoc;

typedef int EtCallbackId;
typedef void (*EtDocSlotChange)(EtDoc *doc, gpointer data);



EtDoc *et_doc_new();
EtDocId et_doc_get_id(EtDoc *this);
bool et_doc_get_filepath(char **filepath, EtDocId doc_id);
bool et_doc_set_filepath(EtDocId doc_id, const char *filepath);
PvVg *et_doc_get_vg_ref(EtDoc *this);
PvVg *et_doc_get_vg_ref_from_id(EtDocId doc_id);
bool et_doc_set_image_from_file(EtDoc *this, const char *filepath);
EtCallbackId et_doc_add_slot_change(EtDocId doc_id, EtDocSlotChange slot, gpointer data);
// Notice: they methods change signale not notify.
PvFocus et_doc_get_focus_from_id(EtDocId id, bool *is_error);
bool et_doc_set_focus_to_id(EtDocId id, PvFocus focus);
// PvFocus et_doc_get_focus(EtDoc *this);
// void et_doc_set_focus(EtDoc *this, PvFocus focus);
// _element: 
bool et_doc_add_point(EtDoc *this, PvElement **_element, double x, double y);
bool et_doc_signal_update(EtDoc *this);
bool et_doc_signal_update_from_id(EtDocId id);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOC_H
