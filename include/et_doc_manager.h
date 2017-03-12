/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_DOC_MANAGER_H
#define include_ET_DOC_MANAGER_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc.h"
#include "pv_vg.h"

struct EtDocManager;
typedef struct EtDocManager EtDocManager;

EtDocManager *et_doc_manager_init();
EtDocId et_doc_manager_new_doc_from_vg(const PvVg *vg);
void et_doc_manager_delete_doc_from_id(EtDocId);
EtDoc *et_doc_manager_get_doc_from_id(const EtDocId doc_id);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOC_MANAGER_H
