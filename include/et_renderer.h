/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_RENDERER_H
#define include_ET_RENDERER_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_canvas.h"
#include "et_doc_id.h"


struct EtRenderer;
typedef struct EtRenderer EtRenderer;


EtRenderer *et_renderer_init();
void slot_et_renderer_from_canvas_change(EtCanvas *canvas, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_RENDERER_H
