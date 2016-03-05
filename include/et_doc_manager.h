#ifndef include_ET_DOC_MANAGER_H
#define include_ET_DOC_MANAGER_H

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

struct _EtDocManager;
typedef struct _EtDocManager EtDocManager;

struct _EtDocManager{
	EtDocNode doc_nodes[16];
};

EtDocManager *et_doc_manager_init();
EtDocId et_doc_manager_new_doc();
EtDoc *et_doc_manager_get_doc_from_id(const EtDocId doc_id);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOC_MANAGER_H
