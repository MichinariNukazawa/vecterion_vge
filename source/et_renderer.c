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

	this->canvas_and_docs = new;

	return true;
}
