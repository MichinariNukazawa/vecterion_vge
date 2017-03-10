#include "et_canvas_collection.h"

#include <glib/gi18n.h>
#include <stdlib.h>
#include "et_error.h"
#include "et_etaion.h"
#include "et_doc.h"
#include "et_renderer.h"
#include "et_pointing_manager.h"
#include "et_doc_relation.h"

#ifdef TARGET_ARCH_WIN
static void gtk_notebook_detach_tab (GtkNotebook *notebook, GtkWidget *child)
{
	gtk_container_remove (GTK_CONTAINER (notebook), child);
}
#endif

static EtCanvasCollection *_canvas_collection = NULL;

struct EtCanvasCollectionCollect;
typedef struct EtCanvasCollectionCollect EtCanvasCollectionCollect;
struct EtCanvasCollectionCollect{
	EtDocId doc_id;
	EtCanvas **canvases;
	GtkWidget *tab_label;
};

struct EtCanvasCollection{
	GtkWidget *widget;
	GtkWidget *widget_tab;
	EtThumbnail *thumbnail;

	EtCanvasCollectionCollect **collects;
};


static EtDocId _et_canvas_collection_get_doc_id_from_canvas_frame(GtkWidget *canvas_frame);
static int _get_collect_index_from_doc_id(EtDocId doc_id);
static void _slot_et_canvas_collection_from_doc_change(EtDoc *doc, gpointer data);

static gboolean _cb_notebook_change_current_page(
		GtkNotebook *notebook,
		GtkWidget    *page,
		gint         page_num,
		gpointer     user_data);

static void _cb_notebook_page_added(
		GtkNotebook *notebook,
		GtkWidget   *child,
		guint        page_num,
		gpointer     user_data);

static void _cb_close_canvas_clicked_button(GtkWidget *button, gpointer data);

EtCanvasCollection *et_canvas_collection_init()
{
	if(NULL != _canvas_collection){
		et_bug("");
		return NULL;
	}

	EtCanvasCollection *self = (EtCanvasCollection *)malloc(sizeof(EtCanvasCollection));
	if(NULL == self){
		et_error("");
		return NULL;
	}

	self->widget_tab = gtk_notebook_new();
	if(NULL == self->widget_tab){
		et_error("");
		return NULL;
	}

	g_signal_connect(GTK_NOTEBOOK(self->widget_tab), "page-added",
			G_CALLBACK(_cb_notebook_page_added), NULL);
	g_signal_connect(GTK_NOTEBOOK(self->widget_tab), "switch-page",
			G_CALLBACK(_cb_notebook_change_current_page), NULL);

	self->thumbnail = et_thumbnail_new();
	if(NULL == self->thumbnail){
		et_error("");
		return NULL;
	}

	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(self->thumbnail);
	int id = et_canvas_set_slot_change(canvas_thumbnail,
			slot_et_renderer_from_canvas_change, NULL);
	if(id < 0){
		et_error("");
		return NULL;
	}

	self->widget = self->widget_tab;

	self->collects = NULL;

	_canvas_collection = self;

	return self;
}

GtkWidget *et_canvas_collection_get_widget_frame()
{
	EtCanvasCollection *self = _canvas_collection;
	if(NULL == self){
		et_bug("");
		return NULL;
	}

	return self->widget;
}

EtThumbnail *et_canvas_collection_get_thumbnail()
{
	EtCanvasCollection *self = _canvas_collection;
	if(NULL == self){
		et_bug("");
		return NULL;
	}

	return self->thumbnail;
}

EtCanvas *et_canvas_collection_new_canvas(EtDocId doc_id)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	// ** new canvas, and setup.
	EtCanvas *canvas = et_canvas_new_from_doc_id(doc_id);
	et_assertf(canvas, "%d", doc_id);

	int id1 = et_doc_add_slot_change(doc_id, slot_et_canvas_from_doc_change, canvas);
	et_assertf(0 <= id1, "%d %d", doc_id, id1);

	int id = et_canvas_set_slot_change(canvas, slot_et_renderer_from_canvas_change, NULL);
	et_assertf(0 <= id, "%d %d", doc_id, id);

	GtkWidget *canvas_frame = et_canvas_get_widget_frame(canvas);
	et_assertf(canvas_frame, "%d", doc_id);

	if(0 > et_canvas_set_slot_mouse_action(canvas, slot_et_pointing_manager_from_mouse_action, NULL)){
		et_error("");
		return NULL;
	}

	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(self->thumbnail);
	bool ret0 = et_canvas_set_doc_id(canvas_thumbnail, doc_id);
	et_assertf(ret0, "%d", doc_id);

	int id2 = et_doc_add_slot_change(doc_id, slot_et_canvas_from_doc_change, canvas_thumbnail);
	et_assertf(0 <= id2, "%d %d", doc_id, id2);

	bool ret1 = et_doc_signal_update_from_id(doc_id);
	et_assertf(ret1, "%d", doc_id);

	// ** new collect
	// TODO: search doc_id in collects alreary.
	EtCanvasCollectionCollect *new_collect = malloc(sizeof(EtCanvasCollectionCollect));
	et_assert(new_collect);

	unsigned int num_canvases = 0;
	new_collect->canvases = realloc(NULL, sizeof(EtCanvas *) * (num_canvases + 2));
	et_assert(new_collect->canvases);

	new_collect->canvases[num_canvases + 1] = NULL;
	new_collect->canvases[num_canvases] = canvas;

	new_collect->doc_id = doc_id;

	// ** add collects
	size_t num_collects = pv_general_get_parray_num((void **)self->collects);
	EtCanvasCollectionCollect **new_collects = realloc(self->collects,
			sizeof(EtCanvasCollectionCollect *) * (num_collects + 2));
	et_assert(new_collects);

	new_collects[num_collects + 1] = NULL;
	new_collects[num_collects + 0] = new_collect;

	self->collects = new_collects;

	// ** add on notebook
	GtkWidget *tab_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	char *title = et_doc_get_new_filename_from_id(doc_id);
	new_collect->tab_label = gtk_label_new(((title) ? title : "(untitled)"));
	if(NULL != title){
		g_free(title);
	}
	GtkWidget *tab_button = gtk_button_new_with_label("[x]");
	gtk_box_pack_start(GTK_BOX(tab_hbox), new_collect->tab_label, true, true, 0);
	gtk_box_pack_start(GTK_BOX(tab_hbox), tab_button, true, true, 0);
	gtk_widget_show_all(tab_hbox);

	int ix = gtk_notebook_append_page(GTK_NOTEBOOK(self->widget_tab), canvas_frame, tab_hbox);
	if(ix < 0){
		et_error("");
		return NULL;
	}

	g_signal_connect(tab_button, "clicked",
			G_CALLBACK(_cb_close_canvas_clicked_button), canvas_frame);

	int id0 = et_doc_add_slot_change(doc_id, _slot_et_canvas_collection_from_doc_change, NULL);
	et_assertf(0 <= id0, "%d %d", doc_id, id0)

		return canvas;
}

EtCanvas *_get_canvas_from_canvas_frame(const GtkWidget *canvas_frame)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	size_t num_collects = pv_general_get_parray_num((void **)self->collects);
	for(int i = 0; i < (int)num_collects; i++){
		const EtCanvasCollectionCollect *collect = self->collects[i];
		size_t num_canvases = pv_general_get_parray_num((void **)collect->canvases);
		for(int t = 0; t < (int)num_canvases; t++){
			et_assertf(collect->canvases[t], "%d,%d", i, t);
			const GtkWidget *frame = et_canvas_get_widget_frame(collect->canvases[t]);
			et_assertf(frame, "%d,%d", i, t);
			if(canvas_frame == frame){
				return collect->canvases[t];
			}
		}
	}

	return NULL;
}

EtCanvas *et_canvas_collection_get_current_canvas()
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	gint page_num = gtk_notebook_get_current_page(GTK_NOTEBOOK(self->widget_tab));
	if(-1 == page_num){
		et_debug("");
		return NULL;
	}

	GtkWidget *canvas_frame = gtk_notebook_get_nth_page(GTK_NOTEBOOK(self->widget_tab), page_num);
	et_assertf(canvas_frame, "%d", page_num);
	EtCanvas *canvas = _get_canvas_from_canvas_frame(canvas_frame);
	et_assertf(canvas, "%d", page_num);

	return canvas;
}

EtDocId et_canvas_collection_get_other_doc_id(EtDocId doc_id)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	et_assert(0 <= doc_id);

	size_t num_collects = pv_general_get_parray_num((void **)self->collects);
	if(num_collects <= 1){
		return -1;
	}else{
		for(int index = 0; num_collects; index++){
			if(doc_id == self->collects[index]->doc_id){
				if(index == ((int)num_collects - 1)){
					return self->collects[index - 1]->doc_id;
				}else{
					return self->collects[index + 1]->doc_id;
				}
			}
		}
	}

	et_abortf("");
}

void et_canvas_collection_delete_canvases_from_doc_id(EtDocId doc_id)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	//! @todo remove or stop signal from EtDoc

	// ** thumbnail
	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(self->thumbnail);
	et_assert(et_canvas_set_doc_id(canvas_thumbnail, -1));

	// ** delete collects.
	int index_collect = _get_collect_index_from_doc_id(doc_id);
	et_assertf(0 <= index_collect, "%d", doc_id);

	EtCanvasCollectionCollect *collect = self->collects[index_collect];
	et_assertf(collect, "%d", doc_id);

	int num_canvases = pv_general_get_parray_num((void **)collect->canvases);
	for(int i = 0; i < (int)num_canvases; i++){
		GtkWidget *canvas_frame = et_canvas_get_widget_frame(collect->canvases[i]);
		et_assertf(canvas_frame, "%d %d %d", doc_id, i, num_canvases);
		gtk_notebook_detach_tab(GTK_NOTEBOOK(self->widget_tab), canvas_frame);

		et_canvas_delete(collect->canvases[i]);
	}

	// remove collect from collects
	size_t num_collects = pv_general_get_parray_num((void **)self->collects);
	memmove(&(self->collects[index_collect]), &(self->collects[index_collect + 1]),
			sizeof(EtCanvasCollectionCollect *) * (num_collects - index_collect));
	et_assert(((int)num_collects - 1) == pv_general_get_parray_num((void **)self->collects));
	free(collect);
}


static EtDocId _et_canvas_collection_get_doc_id_from_canvas_frame(GtkWidget *canvas_frame)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	size_t num_collects = pv_general_get_parray_num((void **)self->collects);
	for(int i = 0; i < (int)num_collects; i++){
		const EtCanvasCollectionCollect *collect = self->collects[i];
		size_t num_canvases = pv_general_get_parray_num((void **)collect->canvases);
		for(int t = 0; t < (int)num_canvases; t++){
			// TODO: const
			// const EtCanvas *canvas = collect->canvases[t];
			EtCanvas *canvas = collect->canvases[t];
			if(NULL == canvas){
				et_bug("");
				goto error;
			}
			const GtkWidget *frame = et_canvas_get_widget_frame(canvas);
			if(NULL == frame){
				et_bug("");
				goto error;
			}
			// ** matching
			if(canvas_frame == frame){
				return collect->doc_id;
			}
		}
	}

error:
	return -1;
}

static gboolean _cb_notebook_change_current_page(
		GtkNotebook *notebook,
		GtkWidget    *page,
		gint         page_num,
		gpointer     user_data)
{
	et_debug("%d", page_num);

	// ** get current doc_id
	EtDocId doc_id = _et_canvas_collection_get_doc_id_from_canvas_frame(page);
	if(doc_id < 0){
		et_error("");
		goto error;
	}
	et_debug("%d", doc_id);

	// ** change Thumbnail doc_id
	EtCanvasCollection *self = _canvas_collection;
	if(NULL == self){
		et_bug("");
		return false;
	}
	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(self->thumbnail);
	if(!et_canvas_set_doc_id(canvas_thumbnail, doc_id)){
		et_bug("");
		return false;
	}

	if(!et_etaion_set_current_doc_id(doc_id)){
		et_bug("");
		goto error;
	}

	// ** update thumbnail (キャンバスが再描画されるのは記述時点で副作用)
	et_doc_signal_update_from_id(doc_id);

	et_debug("%d", doc_id);

	return false;
error:
	return true;
}

static void _cb_notebook_page_added(
		GtkNotebook *notebook,
		GtkWidget   *child,
		guint        page_num,
		gpointer     user_data)
{
	et_debug("%u", page_num);

	gtk_widget_show_all(child);
	gtk_notebook_set_current_page(notebook, (gint)page_num);
}


static int _get_collect_index_from_doc_id(EtDocId doc_id)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	size_t num_collects = pv_general_get_parray_num((void **)self->collects);
	for(int i = 0; i < (int)num_collects; i++){
		if(doc_id == self->collects[i]->doc_id){
			return i;
		}
	}

	return -1;
}

static void _slot_et_canvas_collection_from_doc_change(EtDoc *doc, gpointer data)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	// ** document name to tab
	EtDocId doc_id = et_doc_get_id(doc);

	int index_collect = _get_collect_index_from_doc_id(doc_id);
	et_assertf(0 <= index_collect, "%d", doc_id);

	EtCanvasCollectionCollect *collect = self->collects[index_collect];
	et_assertf(collect, "%d", doc_id);

	char *src_str_title = et_doc_get_new_filename_from_id(doc_id);
	const char *str_title = g_strdup_printf("%c%s",
			((et_doc_is_saved_from_id(doc_id)) ? ' ' : '*'),
			((src_str_title) ? src_str_title : "(untitled)"));

	int num = pv_general_get_parray_num((void **)collect->canvases);
	for(int i = 0; i < num; i++){
		et_assertf(collect->tab_label, "%d %d", doc_id, i);
		et_assertf(str_title, "%d %d", doc_id, i);
		gtk_label_set_text(GTK_LABEL(collect->tab_label), str_title);
	}

	if(NULL != src_str_title){
		g_free(src_str_title);
	}
}

static void _cb_close_canvas_clicked_button(GtkWidget *button, gpointer data)
{
	et_assert(data);

	GtkWidget *canvas_frame = (GtkWidget *)data;

	// ** get document id
	EtDocId doc_id = _et_canvas_collection_get_doc_id_from_canvas_frame(canvas_frame);

	// ** close document
	close_doc_from_id(doc_id, canvas_frame);

	et_debug("Close %d", doc_id);
}

