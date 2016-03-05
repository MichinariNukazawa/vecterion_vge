#ifndef include_ET_CANVAS_COLLECTION_H
#define include_ET_CANVAS_COLLECTION_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_canvas.h"
#include "et_doc_id.h"
#include "et_thumbnail.h"


struct EtCanvasCollection;
typedef struct EtCanvasCollection EtCanvasCollection;


EtCanvasCollection *et_canvas_collection_init();
GtkWidget *et_canvas_collection_get_widget_frame();
EtThumbnail *et_canvas_collection_get_thumbnail();
// GtkWidget *et_canvas_collection_get_thumbnail_frame();
EtCanvas *et_canvas_collection_new_canvas(EtDocId doc_id);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_CANVAS_COLLECTION_H
