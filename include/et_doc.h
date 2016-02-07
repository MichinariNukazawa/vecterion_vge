#ifndef __ET_DOC_H__
#define __ET_DOC_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_point.h"
#include "pv_vg.h"

struct _EtDoc;
typedef struct _EtDoc EtDoc;

typedef int EtCallbackId;
typedef void (*EtDocDrawCallback)(EtDoc *doc, gpointer data);

struct _EtDocCallback;
typedef struct _EtDocCallback EtDocCallback;
struct _EtDocCallback{
	int id;
	EtDocDrawCallback func;
	gpointer data;
};

struct _EtDoc{
	EtDocId id;
	PvVg *vg;

	EtDocCallback *callback_draws;
};



EtDoc *et_doc_new();
bool et_doc_set_image_from_file(EtDoc *this, const char *filepath);
EtCallbackId et_doc_add_draw_callback(EtDoc *this, EtDocDrawCallback func, gpointer data);

bool et_doc_add_point(EtDoc *this, double x, double y);
bool et_doc_draw_canvas(EtDoc *this);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_DOC_H__
