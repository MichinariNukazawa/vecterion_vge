#include "et_canvas_collection.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_doc.h"
#include "et_renderer.h"
#include "et_pointing_manager.h"

static EtCanvasCollection *__canvas_collection = NULL;

struct _EtCanvasCollectionCollect;
typedef struct _EtCanvasCollectionCollect EtCanvasCollectionCollect;
struct _EtCanvasCollectionCollect{
	EtDocId doc_id;
	EtCanvas **canvases;
};

struct _EtCanvasCollection{
	GtkWidget *widget;
	GtkWidget *widget_tab;
	EtThumbnail *thumbnail;

	int num_collects;
	EtCanvasCollectionCollect *collects;
};



int et_general_get_parray_num(void **pointers)
{
	if(NULL == pointers){
		return 0;
	}

	int i = 0;
	while(NULL != pointers[i]){
		i++;
	}

	if(15 < i){
		et_debug("num:%d\n", i);
	}

	return i;
}

EtCanvasCollection *et_canvas_collection_init()
{
	if(NULL != __canvas_collection){
		et_bug("");
		return NULL;
	}

	EtCanvasCollection *this = (EtCanvasCollection *)malloc(sizeof(EtCanvasCollection));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	this->widget_tab = gtk_notebook_new();
	if(NULL == this->widget_tab){
		et_error("");
		return NULL;
	}

	this->thumbnail = et_thumbnail_new();
	if(NULL == this->thumbnail){
		et_error("");
		return NULL;
	}

	this->widget = this->widget_tab;

	this->num_collects = 0;
	this->collects = NULL;

	__canvas_collection = this;

	return this;
}

GtkWidget *et_canvas_collection_get_widget_frame()
{
	EtCanvasCollection *this = __canvas_collection;
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	return this->widget;
}

EtThumbnail *et_canvas_collection_get_thumbnail()
{
	EtCanvasCollection *this = __canvas_collection;
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	return this->thumbnail;
}

GtkWidget *et_canvas_collection_get_thumbnail_frame()
{
	EtCanvasCollection *this = __canvas_collection;
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	GtkWidget *widget = et_thumbnail_get_widget_frame(this->thumbnail);
	if(NULL == widget){
		et_bug("");
		return NULL;
	}

	return widget;
}

EtCanvas *et_canvas_collection_new_canvas(EtDocId doc_id)
{
	EtCanvasCollection *this = __canvas_collection;
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	// ** canvas new and setup.
	EtCanvas *canvas = et_canvas_new();
	if(!(et_canvas_get_doc_id(canvas) < 0)){
		et_bug("");
		return false;
	}
	if(!et_canvas_set_doc_id(canvas, doc_id)){
		et_bug("");
		return false;
	}

	int id = et_canvas_set_slot_change(canvas,
			slot_et_renderer_from_canvas_change, NULL);
	if(id < 0){
		et_error("");
		return NULL;
	}

	GtkWidget *canvas_frame1 = et_canvas_get_widget_frame(canvas);
	if(NULL == canvas_frame1){
		et_bug("");
		return NULL;
	}
	gtk_notebook_append_page(GTK_NOTEBOOK(this->widget_tab), canvas_frame1, NULL);

	if(0 > et_canvas_set_slot_mouse_action(canvas,
				et_pointing_manager_slot_mouse_action, NULL)){
		et_error("");
		return NULL;
	}
	if(!et_doc_signal_update_from_id(doc_id)){
		et_error("");
		return NULL;
	}

	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(this->thumbnail);
	if(!et_canvas_set_doc_id(canvas_thumbnail, doc_id)){
		et_bug("");
		return false;
	}
	int id2 = et_canvas_set_slot_change(canvas_thumbnail,
			slot_et_renderer_from_canvas_change, NULL);
	if(id2 < 0){
		et_error("");
		return NULL;
	}

	// ** add collects.
	// TODO: search doc_id in collects alreary.
	EtCanvasCollectionCollect *new = realloc(this->collects,
			sizeof(EtCanvasCollectionCollect) * (this->num_collects + 1));
	if(NULL == new){
		et_error("");
		return NULL;
	}

	int num_canvases = 0;
	EtCanvas **canvases = realloc(NULL, sizeof(EtCanvas *) * (num_canvases + 2));
	if(NULL == new){
		et_error("");
		return NULL;
	}

	canvases[num_canvases + 1] = NULL;
	canvases[num_canvases] = canvas;

	new[this->num_collects].canvases = canvases;
	new[this->num_collects].doc_id = doc_id;
	(this->num_collects)++;

	this->collects = new;

	return canvas;
}

