/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
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
EtCanvas *et_canvas_collection_new_canvas(EtDocId doc_id);
EtCanvas *et_canvas_collection_get_current_canvas();
EtDocId et_canvas_collection_get_other_doc_id(EtDocId);
void et_canvas_collection_delete_canvases_from_doc_id(EtDocId);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_CANVAS_COLLECTION_H
