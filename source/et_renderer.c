#include "et_renderer.h"

#include <stdlib.h>
#include "et_error.h"
#include "pv_renderer.h"
#include "et_doc_manager.h"
#include "et_etaion.h"

struct EtRenderer{
	int dummy;
};

static EtRenderer *_renderer = NULL;

EtRenderer *et_renderer_init()
{
	if(NULL != _renderer){
		et_bug("");
		return NULL;
	}

	EtRenderer *self = (EtRenderer *)malloc(sizeof(EtRenderer));
	if(NULL == self){
		et_error("");
		return NULL;
	}

	_renderer = self;

	return self;
}

static GdkPixbuf *_et_renderer_rendering_pixbuf_new(EtDoc *doc, PvRenderContext render_context)
{
	PvVg *vg = et_doc_get_vg_ref(doc);
	if(NULL == vg){
		et_bug("");
		return NULL;
	}

	const PvFocus *focus = et_doc_get_focus_ref_from_id(et_doc_get_id(doc));
	if(NULL == focus){
		et_error("");
		return NULL;
	}

	PvElement *element_group_edit_draw
		= et_doc_get_element_group_edit_draw_from_id(et_doc_get_id(doc));

	GdkPixbuf *pb = pv_renderer_pixbuf_from_vg(vg, render_context, focus, element_group_edit_draw);
	if(NULL == pb){
		et_error("");
		return NULL;
	}

	return pb;
}

void slot_et_renderer_from_canvas_change(EtCanvas *canvas, gpointer data)
{
	EtRenderer *self = _renderer;
	if(NULL == self){
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
	PvRenderContext render_context = et_canvas_get_render_context(canvas, &isError);
	if(isError){
		et_bug("");
		return;
	}

	render_context.is_extent_view = et_etaion_get_is_extent_view();
	render_context.is_transparent_grid = et_etaion_get_is_transparent_grid();

	GdkPixbuf *pixbuf = _et_renderer_rendering_pixbuf_new(doc, render_context);
	if(NULL == pixbuf){
		et_error("");
	}else{
		et_canvas_draw_pixbuf(canvas, pixbuf);
		g_object_unref(G_OBJECT(pixbuf));
	}
}

