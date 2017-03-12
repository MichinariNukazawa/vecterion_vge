/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_THUMBNAIL_H
#define include_ET_THUMBNAIL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_canvas.h"


typedef EtCanvas EtThumbnail;


EtThumbnail *et_thumbnail_new();
EtCanvas *et_thumbnail_get_canvas(EtThumbnail *self);
GtkWidget *et_thumbnail_get_widget_frame(EtThumbnail *self);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_THUMBNAIL_H
