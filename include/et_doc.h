#ifndef __ET_DOC_H__
#define __ET_DOC_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_point.h"
#include "pv_vg.h"
#include "pv_focus.h"

struct _EtDoc;
typedef struct _EtDoc EtDoc;

typedef int EtCallbackId;
typedef void (*EtDocSlotChange)(EtDoc *doc, gpointer data);

struct _EtDocSlotChangeInfo;
typedef struct _EtDocSlotChangeInfo EtDocSlotChangeInfo;
struct _EtDocSlotChangeInfo{
	int id;
	EtDocSlotChange slot;
	gpointer data;
};


EtDoc *et_doc_new();
EtDocId et_doc_get_id(EtDoc *this);
PvVg *et_doc_get_vg_ref(EtDoc *this);
bool et_doc_set_image_from_file(EtDoc *this, const char *filepath);
EtCallbackId et_doc_add_slot_change(EtDoc *this, EtDocSlotChange slot, gpointer data);
PvFocus et_doc_get_focus_from_id(EtDocId id, bool *is_error);
bool et_doc_set_focus_to_id(EtDocId id, PvFocus focus);
// PvFocus et_doc_get_focus(EtDoc *this);
// void et_doc_set_focus(EtDoc *this, PvFocus focus);
// _element: 
bool et_doc_add_point(EtDoc *this, PvElement **_element, double x, double y);
bool et_doc_draw_canvas(EtDoc *this);
bool et_doc_draw_canvas_from_id(EtDocId id);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_DOC_H__
