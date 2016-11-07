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
EtDoc *et_doc_manager_get_doc_from_id(const EtDocId doc_id);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOC_MANAGER_H
