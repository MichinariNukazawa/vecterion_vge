#ifndef __ET_RENDERER_H__
#define __ET_RENDERER_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_canvas.h"
#include "et_doc.h"

struct _EtCanvasAndDoc;
typedef struct _EtCanvasAndDoc EtCanvasAndDoc;
struct _EtCanvasAndDoc{
	EtCallbackId	id;
	EtCanvas	*canvas;
	EtDoc		*doc;
};


struct _EtRenderer;
typedef struct _EtRenderer EtRenderer;

struct _EtRenderer{
	EtCanvasAndDoc *canvas_and_docs;
};

EtRenderer *et_renderer_new();
bool et_renderer_set_connection(EtRenderer *this, EtCanvas *canvas, EtDoc *doc);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_RENDERER_H__
