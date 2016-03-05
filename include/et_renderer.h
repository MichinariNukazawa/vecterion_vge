#ifndef __ET_RENDERER_H__
#define __ET_RENDERER_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_canvas.h"
#include "et_doc_id.h"


struct _EtRenderer;
typedef struct _EtRenderer EtRenderer;


EtRenderer *et_renderer_init();
void slot_et_renderer_from_canvas_change(EtCanvas *canvas, gpointer data);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_RENDERER_H__
