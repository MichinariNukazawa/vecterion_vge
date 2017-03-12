/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_DOC_RELATION_H
#define include_ET_DOC_RELATION_H

#include "et_doc_id.h"
#include "pv_vg.h"

EtDocId open_doc_new(PvVg *vg_src);
bool close_doc_from_id(EtDocId doc_id, GtkWidget *widget);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOC_RELATION_H

