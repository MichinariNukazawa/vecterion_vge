#ifndef __ET_RENDERER_H__
#define __ET_RENDERER_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_canvas.h"
#include "et_doc.h"


struct _EtRenderer;
typedef struct _EtRenderer EtRenderer;


EtRenderer *et_renderer_init();
bool et_renderer_set_connection(EtCanvas *canvas, EtDoc *doc);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_RENDERER_H__
