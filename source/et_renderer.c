#include "et_renderer.h"

#include <stdlib.h>
#include "et_error.h"

EtRenderer *et_renderer_new()
{
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

	return this;
}

int _et_renderer_get_num_canvas_and_docs(EtCanvasAndDoc *canvas_and_docs){
	int i = 0;
	while(0 <= canvas_and_docs[i].id){
		i++;
	}

	return i;
}
void _cb_et_renderer_draw(EtDoc *doc, gpointer data)
{
	EtRenderer *this = (EtRenderer *)data;
	int num = _et_renderer_get_num_canvas_and_docs(this->canvas_and_docs);
	for(int i = 0; i < num; i++){
		if(doc == this->canvas_and_docs[i].doc){
			GdkPixbuf *pixbuf = et_doc_get_pixbuf(doc);
			et_canvas_draw_pixbuf(this->canvas_and_docs[i].canvas, pixbuf);
		}
	}
}

void _et_renderer_cb_update_canvas(EtCanvas *canvas, gpointer data)
{
	et_debug("canvas:%ld, rend:%ld\n", canvas, data);

	EtRenderer *this = (EtRenderer *)data;
	int num = _et_renderer_get_num_canvas_and_docs(this->canvas_and_docs);
	for(int i = 0; i < num; i++){
		if(canvas == this->canvas_and_docs[i].canvas){
			et_debug("DEBUGER\n");
			GdkPixbuf *pixbuf = et_doc_get_pixbuf(this->canvas_and_docs[i].doc);
			// et_canvas_draw_pixbuf(canvas, pixbuf);

			// Todo: canvas->render_contextによる倍率変更
			double w = (double)gdk_pixbuf_get_width(pixbuf)
						* canvas->render_context.scale;
			double h = (double)gdk_pixbuf_get_height(pixbuf)
						* canvas->render_context.scale;
			GdkPixbuf *pb = gdk_pixbuf_scale_simple(pixbuf,
				(int)w, (int)h,
				GDK_INTERP_HYPER);
			et_canvas_draw_pixbuf(canvas, pb);
			g_object_unref(G_OBJECT(pb));
		}
	}
}

bool et_renderer_set_connection(EtRenderer *this, EtCanvas *canvas, EtDoc *doc)
{
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
	new[num].id = et_doc_add_draw_callback(doc,
			_cb_et_renderer_draw, (gpointer)this);
	if(new[num].id < 0){
		et_error("");
		return false;
	}

	int id = et_canvas_set_update_render_context(canvas,
		_et_renderer_cb_update_canvas, (gpointer)this);
	if(id < 0){
		et_error("");
		return false;
	}

	this->canvas_and_docs = new;

	return true;
}
