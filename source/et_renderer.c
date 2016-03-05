#include "et_renderer.h"

#include <stdlib.h>
#include "et_error.h"
#include "pv_renderer.h"
#include "et_doc_manager.h"

struct _EtRenderer{
	int dummy;
};

static EtRenderer *_renderer = NULL;

EtRenderer *et_renderer_init()
{
	if(NULL != _renderer){
		et_bug("");
		return NULL;
	}

	EtRenderer *this = (EtRenderer *)malloc(sizeof(EtRenderer));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	_renderer = this;

	return this;
}

GdkPixbuf *_et_renderer_draw_pixbuf_from_point(GdkPixbuf *pixbuf, double x, double y)
{
	gint width;
	gint height;
	cairo_format_t format;
	cairo_surface_t *surface;
	cairo_t *cr;


	format = (gdk_pixbuf_get_has_alpha (pixbuf)) ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	surface = cairo_image_surface_create (format, width, height);
	if(surface == NULL){
		et_bug("");
		return NULL;
	}
	
//	surface = gdk_cairo_surface_create_from_pixbuf (pixbuf, 1, NULL);
	cr = cairo_create (surface);
	/* Draw the pixbuf */
	gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
	cairo_paint (cr);
	/* Draw a red rectangle */
	cairo_set_source_rgb (cr, 0.1, 0.1, 0.1);
	cairo_rectangle (cr, x, y, 2, 2);
	cairo_fill (cr);

/*
	if(CAIRO_STATUS_SUCCESS != cairo_surface_write_to_png (surface, "output.png")){
		et_error("");
		return NULL;
	}
*/
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	GdkPixbuf *pb = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
	if(NULL == pb){
		et_error("");
		return NULL;
	}

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}

GdkPixbuf *_et_renderer_rendering_pixbuf_new(EtDoc *doc, PvRenderContext render_context)
{
	PvVg *vg = et_doc_get_vg_ref(doc);
	if(NULL == vg){
		et_bug("");
		return NULL;
	}

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id(et_doc_get_id(doc), &is_error);
	if(is_error){
		et_error("");
		return NULL;
	}

	GdkPixbuf *pb = pv_renderer_pixbuf_from_vg(vg, render_context, focus);
	if(NULL == pb){
		et_error("");
		return NULL;
	}

	return pb;
}

void slot_et_renderer_from_canvas_change(EtCanvas *canvas, gpointer data)
{
	et_debug("\n");

	EtRenderer *this = _renderer;
	if(NULL == this){
		et_bug("");
		return;
	}

	if(NULL == canvas){
		et_bug("");
		return;
	}

	EtDocId doc_id = et_canvas_get_doc_id(canvas);
	if(0 > doc_id){
		et_bug("");
		return;
	}
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_bug("");
		return;
	}
	bool isError = true;
	PvRenderContext render_context
		= et_canvas_get_render_context(canvas, &isError);
	if(isError){
		et_bug("");
		return;
	}
	GdkPixbuf *pixbuf = _et_renderer_rendering_pixbuf_new(doc,
			render_context);
	if(NULL == pixbuf){
		et_error("");
	}else{
		et_canvas_draw_pixbuf(canvas, pixbuf);
		g_object_unref(G_OBJECT(pixbuf));
	}
}

