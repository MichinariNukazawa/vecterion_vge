#ifndef __ET_CANVAS_H__
#define __ET_CANVAS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_doc.h"

struct _EtCanvas;
typedef struct _EtCanvas EtCanvas;

struct _EtCanvas{
	EtDoc *doc;

	GtkWidget *widget;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *canvas;
};

EtCanvas *et_canvas_new();
void et_canvas_call_draw(EtCanvas *this);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CANVAS_H__
