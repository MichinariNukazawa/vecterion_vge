#include "et_thumbnail.h"

#include "et_error.h"

EtThumbnail *et_thumbnail_new()
{
	EtCanvas *canvas = et_canvas_new_from_doc_id(-1);
	if(NULL == canvas){
		et_error("");
		return NULL;
	}

	return (EtThumbnail *)canvas;
}

EtCanvas *et_thumbnail_get_canvas(EtThumbnail *self)
{
	return (EtCanvas *)self;
}

GtkWidget *et_thumbnail_get_widget_frame(EtThumbnail *self)
{
	EtCanvas *canvas = et_thumbnail_get_canvas(self);

	GtkWidget *widget = et_canvas_get_widget_frame(canvas);
	if(NULL == widget){
		et_error("");
		return NULL;
	}

	return widget;
}

