#ifndef __ET_THUMBNAIL_H__
#define __ET_THUMBNAIL_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "et_canvas.h"


typedef EtCanvas EtThumbnail;


EtThumbnail *et_thumbnail_new();
EtCanvas *et_thumbnail_get_canvas(EtThumbnail *this);
GtkWidget *et_thumbnail_get_widget_frame(EtThumbnail *this);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_THUMBNAIL_H__
