/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_DOC_H
#define include_ET_DOC_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "pv_vg.h"
#include "pv_focus.h"
#include "pv_document_preference.h"

struct EtDoc;
typedef struct EtDoc EtDoc;

typedef int EtCallbackId;
typedef void (*EtDocSlotChange)(EtDoc *doc, gpointer data);



EtDoc *et_doc_new();
EtDoc *et_doc_new_from_vg(const PvVg *vg);
void et_doc_delete(EtDoc *);
EtDocId et_doc_get_id(EtDoc *self);
char *et_doc_get_new_filename_from_id(EtDocId doc_id);
bool et_doc_get_saved_filepath(char **filepath, EtDocId doc_id);
bool et_doc_set_saved_filepath(EtDocId doc_id, const char *filepath);
PvVg *et_doc_get_vg_ref(EtDoc *self);
PvVg *et_doc_get_vg_ref_from_id(EtDocId doc_id);
bool et_doc_set_image_from_file(EtDoc *self, const char *filepath);
EtCallbackId et_doc_add_slot_change(EtDocId doc_id, EtDocSlotChange slot, gpointer data);
// Notice: they methods change signale not notify.
PvFocus *et_doc_get_focus_ref_from_id(EtDocId id);
// PvFocus et_doc_get_focus(EtDoc *self);
// void et_doc_set_focus(EtDoc *self, PvFocus focus);
PvDocumentPreference et_doc_get_document_preference_from_id(EtDocId);
void et_doc_set_document_preference_from_id(EtDocId, PvDocumentPreference);

void et_doc_set_element_group_edit_draw_from_id(EtDocId, const PvElement *);
PvElement *et_doc_get_element_group_edit_draw_from_id(EtDocId);

bool et_doc_signal_update(EtDoc *self);
bool et_doc_signal_update_from_id(EtDocId id);

bool et_doc_save_from_id(EtDocId doc_id);
bool et_doc_undo_from_id(EtDocId doc_id);
bool et_doc_redo_from_id(EtDocId doc_id);

bool et_doc_is_saved_from_id(EtDocId doc_id);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOC_H
