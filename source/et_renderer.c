#include "et_renderer.h"

#include <stdlib.h>
#include "et_error.h"
#include "pv_renderer.h"
#include "et_doc_manager.h"

struct _EtCanvasAndDoc;
typedef struct _EtCanvasAndDoc EtCanvasAndDoc;
struct _EtCanvasAndDoc{
	EtCallbackId	id;
	EtCanvas	*canvas;
	EtDoc		*doc;
};

struct _EtRenderer{
	EtCanvasAndDoc *canvas_and_docs;
};

static EtRenderer *__renderer = NULL;

EtRenderer *et_renderer_init()
{
	if(NULL != __renderer){
		et_bug("");
		return NULL;
	}

	EtRenderer *this = (EtRenderer *)malloc(sizeof(EtRenderer));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	this->canvas_and_docs = (EtCanvasAndDoc *)malloc(sizeof(EtCanvasAndDoc) * 1);
	if(NULL == this->canvas_and_docs){
		et_error("");
		return NULL;
	}
	this->canvas_and_docs[0].id = -1;

	__renderer = this;

	return this;
}

int _et_renderer_get_num_canvas_and_docs(EtCanvasAndDoc *canvas_and_docs){
	int i = 0;
	while(0 <= canvas_and_docs[i].id){
		i++;
	}

	return i;
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

void _slot_et_renderer_from_doc_change(EtDoc *doc, gpointer data)
{
	EtRenderer *this = __renderer;
	if(NULL == this){
		et_bug("");
		return;
	}

	int num = _et_renderer_get_num_canvas_and_docs(this->canvas_and_docs);
	for(int i = 0; i < num; i++){
		if(doc == this->canvas_and_docs[i].doc){
			EtCanvas *canvas = this->canvas_and_docs[i].canvas;
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
				et_canvas_draw_pixbuf(this->canvas_and_docs[i].canvas, pixbuf);
				g_object_unref(G_OBJECT(pixbuf));
			}
		}
	}
}

void _slot_et_renderer_from_canvas_change(EtCanvas *canvas, gpointer data)
{
	EtRenderer *this = __renderer;
	if(NULL == this){
		et_bug("");
		return;
	}

	int num = _et_renderer_get_num_canvas_and_docs(this->canvas_and_docs);
	for(int i = 0; i < num; i++){
		if(canvas == this->canvas_and_docs[i].canvas){
			EtDoc *doc = this->canvas_and_docs[i].doc;
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
				et_canvas_draw_pixbuf(this->canvas_and_docs[i].canvas, pixbuf);
				g_object_unref(G_OBJECT(pixbuf));
			}
		}
	}
}

bool et_renderer_set_connection(EtCanvas *canvas, EtDocId doc_id)
{
	EtRenderer *this = __renderer;
	if(NULL == this){
		et_bug("");
		return false;
	}

	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_bug("");
		return false;
	}

	int num = _et_renderer_get_num_canvas_and_docs(this->canvas_and_docs);
	EtCanvasAndDoc *new = realloc(this->canvas_and_docs,
			sizeof(EtCanvasAndDoc) * (num + 2));
	if(NULL == new){
		et_error("");
		return false;
	}
	new[num + 1].id = -1;
	new[num].canvas = canvas;
	new[num].doc = doc;
	new[num].id = et_doc_add_slot_change(doc_id,
			_slot_et_renderer_from_doc_change, NULL);
	if(new[num].id < 0){
		et_error("");
		return false;
	}

	int id = et_canvas_set_slot_change(canvas,
			_slot_et_renderer_from_canvas_change, NULL);
	if(id < 0){
		et_error("");
		return false;
	}

	if(!(et_canvas_get_doc_id(canvas) < 0)){
		et_bug("");
		return false;
	}
	if(!et_canvas_set_doc_id(canvas, doc_id)){
		et_bug("");
		return false;
	}


	this->canvas_and_docs = new;

	return true;
}
