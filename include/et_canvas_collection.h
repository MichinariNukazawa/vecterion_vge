#ifndef __ET_CANVAS_COLLECTION_H__
#define __ET_CANVAS_COLLECTION_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_canvas.h"
#include "et_doc_id.h"
#include "et_thumbnail.h"


struct _EtCanvasCollection;
typedef struct _EtCanvasCollection EtCanvasCollection;


EtCanvasCollection *et_canvas_collection_init();
GtkWidget *et_canvas_collection_get_widget_frame();
EtThumbnail *et_canvas_collection_get_thumbnail();
// GtkWidget *et_canvas_collection_get_thumbnail_frame();
EtCanvas *et_canvas_collection_new_canvas(EtDocId doc_id);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_CANVAS_COLLECTION_H__
