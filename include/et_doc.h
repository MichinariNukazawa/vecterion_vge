#ifndef __ET_DOC_H__
#define __ET_DOC_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

struct _EtDoc;
typedef struct _EtDoc EtDoc;

struct _EtPoint;
typedef struct _EtPoint EtPoint;
struct _EtPoint{
	double x;
	double y;
};

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
	GdkPixbuf *pixbuf;

	EtDocCallback *callback_draws;
	EtPoint points[10];
};

EtDoc *et_doc_new();
bool et_doc_set_image_from_file(EtDoc *this, const char *filepath);
EtCallbackId et_doc_add_draw_callback(EtDoc *this, EtDocDrawCallback func, gpointer data);
GdkPixbuf *et_doc_get_pixbuf(EtDoc *this);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_DOC_H__
