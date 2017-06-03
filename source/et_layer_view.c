#include "et_layer_view.h"

#include <stdlib.h>
#include <string.h>
#include "et_error.h"
#include "et_doc_manager.h"
#include "pv_element.h"
#include "et_mouse_util.h"
#include "et_etaion.h"


typedef struct EtLayerViewLayerCtrl{
	const char * const name;
	const char * const filename_image;
}EtLayerViewLayerCtrl;

const EtLayerViewLayerCtrl et_layer_view_layer_ctrls_buttons_[] = {
	{"New", "layer_new_64x64.svg",},
	{"Child", "layer_new_child_64x64.svg",},
	{"Clone", "layer_clone_64x64.svg",},
	{"Delete", "layer_del_64x64.svg",},
};

typedef struct EtLayerViewElementData{
	int level;
	const char *name;
	PvElementKind kind;
	PvElement *element;
}EtLayerViewElementData;

struct EtLayerView{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *text;
	GtkWidget *box_button_layer_ctrl;
	GtkWidget *button_layer_ctrls[4];

	EtDocId doc_id;
	// buffer
	EtLayerViewElementData **elementDatas;
};

typedef struct EtLayerViewRltDataPack{
	EtLayerViewElementData **datas;
}EtLayerViewRltDataPack;


static EtLayerView *layer_view_ = NULL;

static gboolean cb_button_press_layer_view_content_(
		GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean cb_button_release_layer_view_content_(
		GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean cb_motion_notify_layer_view_content_(
		GtkWidget *widget, GdkEventMotion *event, gpointer data);

static gboolean cb_clicked_layer_ctrl_button_(GtkWidget *widget, gpointer data);

static bool init_layer_ctrl_button_(EtLayerView *self, int index)
{
	GError *error = NULL;
	GdkPixbuf *pixbuf = NULL;
	char tmp[1024];
	snprintf(tmp, sizeof(tmp),
			"%s/resource/%s",
			et_etaion_get_application_base_dir(),
			et_layer_view_layer_ctrls_buttons_[index].filename_image);
	pixbuf = gdk_pixbuf_new_from_file(tmp, &error);
	{
		GdkPixbuf *t = pixbuf;
		pixbuf = gdk_pixbuf_scale_simple(
				t,
				32, 32,
				GDK_INTERP_HYPER);
		et_assert(pixbuf);
		g_object_unref(t);
	}
	if(NULL == pixbuf){
		et_error("%d, '%s'\n", index, tmp);
		return false;
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
	if(NULL == image){
		et_error("");
		return false;
	}

	self->button_layer_ctrls[index] = gtk_button_new();
	if(NULL == self->button_layer_ctrls[index]){
		et_error("");
		return false;
	}
	// フォーカス状態でEnterKey押下した際に、誤作動する問題への対処
#if GTK_CHECK_VERSION (3,20,0)
	gtk_widget_set_focus_on_click(self->button_layer_ctrls[index], false);
#else
	gtk_button_set_focus_on_click(GTK_BUTTON(self->button_layer_ctrls[index]), false);
#endif
	// gtk_widget_set_can_focus(self->button_layer_ctrls[index], false);
	gtk_container_add(GTK_CONTAINER(self->button_layer_ctrls[index]),
			image);

	gtk_container_add(GTK_CONTAINER(self->box_button_layer_ctrl),
			self->button_layer_ctrls[index]);

	g_signal_connect(self->button_layer_ctrls[index], "clicked",
			G_CALLBACK(cb_clicked_layer_ctrl_button_), (gpointer)self);

	// デフォルト無効からスタート
	gtk_widget_set_sensitive(self->button_layer_ctrls[index], false);

	return true;
}

EtLayerView *et_layer_view_init()
{
	et_assert(!layer_view_);

	EtLayerView *self = (EtLayerView *)malloc(sizeof(EtLayerView));
	et_assert(self);

	self->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

	self->scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(GTK_WIDGET(self->scroll), TRUE);  
	gtk_widget_set_vexpand(GTK_WIDGET(self->scroll), TRUE);
	gtk_container_add(GTK_CONTAINER(self->box), self->scroll);

	self->event_box = gtk_event_box_new();
	et_assert(self->event_box);
	gtk_widget_set_events(self->event_box,
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK
			);
	g_signal_connect(self->event_box, "button-press-event",
			G_CALLBACK(cb_button_press_layer_view_content_), (gpointer)self);
	g_signal_connect(self->event_box, "button-release-event",
			G_CALLBACK(cb_button_release_layer_view_content_), (gpointer)self);
	g_signal_connect(self->event_box, "motion-notify-event",
			G_CALLBACK(cb_motion_notify_layer_view_content_), (gpointer)self);

	gtk_container_add(GTK_CONTAINER(self->scroll), self->event_box);

	self->text = gtk_text_view_new();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->text));
	gtk_text_buffer_set_text (buffer, "<Nothing document>", -1);
	gtk_text_view_set_editable (GTK_TEXT_VIEW(self->text), false);
#if GTK_CHECK_VERSION (3,16,0)
	gtk_text_view_set_monospace (GTK_TEXT_VIEW(self->text), TRUE);
#endif
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(self->text), false);
	gtk_container_add(GTK_CONTAINER(self->event_box), self->text);

	self->box_button_layer_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(self->box), self->box_button_layer_ctrl);

	et_assert(init_layer_ctrl_button_(self, 0));
	et_assert(init_layer_ctrl_button_(self, 1));
	et_assert(init_layer_ctrl_button_(self, 2));
	et_assert(init_layer_ctrl_button_(self, 3));

	self->widget = self->box;

	self->doc_id = -1;
	self->elementDatas = NULL;

	layer_view_ = self;

	return self;
}

GtkWidget *et_layer_view_get_widget_frame()
{
	EtLayerView *self = layer_view_;
	et_assert(self);

	return self->widget;
}

static bool read_layer_tree_(PvElement *element, gpointer data, int level)
{
	EtLayerViewRltDataPack *func_rlt_data_pack = data;
	EtLayerViewElementData ***datas = &(func_rlt_data_pack->datas);

	EtLayerViewElementData *rlt = malloc(sizeof(EtLayerViewElementData));
	et_assert(rlt);

	rlt->level = level;
	rlt->name = "";
	rlt->kind = element->kind;
	rlt->element = element;

	int num = pv_general_get_parray_num((void **)*datas);
	*datas = (EtLayerViewElementData **)
		realloc(*datas, sizeof(EtLayerViewElementData *) * (num + 2));
	et_assert(*datas);

	(*datas)[num] = rlt;
	(*datas)[num + 1] = NULL;

	return true;
}

static bool draw_element_tree_(EtLayerView *self)
{
	et_assert(self);

	const int LEN_ELEMENT = 128;

	EtLayerViewElementData **elementDatas = self->elementDatas;
	size_t num = pv_general_get_parray_num((void **)elementDatas);
	size_t len_elements = LEN_ELEMENT * num;
	char *buf = malloc((sizeof(char) * len_elements) + 1);
	et_assert(buf);

	const PvElement *focus_element = NULL;
	if(self->doc_id < 0){
		strcpy(buf, "<None>");
	}else{

		const PvFocus *focus = et_doc_get_focus_ref_from_id(self->doc_id);
		if(NULL == focus){
			et_error("");
			return false;
		}

		buf[0] = '\0';
		focus_element = pv_focus_get_first_element(focus);
		for(int i = 0; i < (int)num; i++){
			EtLayerViewElementData *data = elementDatas[i];

			if(0 == i){
				// skip root element.
				if(PvElementKind_Root != data->element->kind){
					et_bug("%d\n", data->element->kind);
				}
				continue;
			}else{
				if(PvElementKind_Root == data->element->kind){
					et_bug("%d\n", i);
					continue;
				}
			}

			unsigned long debug_pointer = 0;
			memcpy(&debug_pointer, &data->element, sizeof(unsigned long));

			char str_element_head[128];
			str_element_head[0] = '\0';
			for(int t = 0; t < data->level; t++){
				str_element_head[t] = '_';
				str_element_head[t + 1] = '\0';
				if(!(t < ((int)sizeof(str_element_head) - 2))){
					break;
				}
			}
			const char *kind_name = pv_element_get_name_from_kind(data->kind);
			if(NULL == kind_name){
				kind_name = "";
			}
			char str_element[128];
			snprintf(str_element, sizeof(str_element),
					"%s%c:%s\t:%08lx '%s'\n",
					str_element_head,
					((focus_element == data->element)? '>':'_'),
					//data->level,
					kind_name,
					debug_pointer,
					((data->name)?"":data->name));

			strncat(buf, str_element, (len_elements - 1) - strlen(buf));
			buf[(len_elements - 1)] = '\0';
			if(!(strlen(buf) < (len_elements - 2))){
				et_warning("%zu/%zu\n", strlen(buf), len_elements);
			}
		}
	}

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (self->text));
	gtk_text_buffer_set_text (buffer, buf, -1);
	free(buf);

	return true;
}

static bool update_ui_(EtLayerView *self)
{
	et_assert(self);

	if(!draw_element_tree_(self)){
		et_warning("");
	}

	// ターゲット状態でlayer_ctrlsのbutton状態を変更する
	{
		PvElement *focus_element = NULL;
		const PvFocus *focus = et_doc_get_focus_ref_from_id(self->doc_id);
		if(NULL != focus){
			focus_element = pv_focus_get_first_element(focus);
		}

		gtk_widget_set_sensitive(self->button_layer_ctrls[0], (0 <= self->doc_id));
		gtk_widget_set_sensitive(self->button_layer_ctrls[1], (NULL != focus_element));
		gtk_widget_set_sensitive(self->button_layer_ctrls[2], (NULL != focus_element));
		gtk_widget_set_sensitive(self->button_layer_ctrls[3], (NULL != focus_element));
	}

	return true;
}

static bool update_doc_tree_()
{
	EtLayerView *self = layer_view_;
	et_assert(self);

	EtLayerViewRltDataPack func_rlt_data_pack = {
		.datas = NULL
	};

	if(self->doc_id < 0){
		// nop
	}else{

		EtDoc *doc = et_doc_manager_get_doc_from_id(self->doc_id);
		if(NULL == doc){
			et_error("");
			return false;
		}

		PvVg *vg = et_doc_get_vg_ref(doc);
		if(NULL == vg){
			et_error("");
			return false;
		}

		PvElementRecursiveError error;
		if(!pv_element_recursive_desc_before(vg->element_root,
					read_layer_tree_, &func_rlt_data_pack,
					&error)){
			et_error("level:%d", error.level);
			return false;
		}
	}

	EtLayerViewElementData **before = self->elementDatas;
	self->elementDatas = func_rlt_data_pack.datas;
	if(NULL != before){
		int num = pv_general_get_parray_num((void **)before);
		for(int i = 0; i < num; i++){
			EtLayerViewElementData *data = before[i];
			free(data);
		}
	}

	return update_ui_(self);
}

bool et_layer_view_set_doc_id(EtDocId doc_id)
{
	EtLayerView *self = layer_view_;
	et_assert(self);

	self->doc_id = doc_id;

	if(!update_doc_tree_()){
		et_error("");
		return false;
	}

	return true;
}

void et_layer_view_slot_from_doc_change(EtDoc *doc, gpointer data)
{
	if(!update_doc_tree_()){
		et_error("");
	}
}

void slot_et_layer_view_from_etaion_change_state(EtState state, gpointer data)
{
	EtLayerView *self = layer_view_;
	et_assert(self);

	self->doc_id = state.doc_id;

	if(!et_layer_view_set_doc_id(self->doc_id)){
		et_error("");
		return;
	}
}

int index_data_from_position_(EtLayerView *self, int x, int y)
{
	et_assert(self);

	int index = (y/16) + 1; // +1 = hidden root layer

	int num = pv_general_get_parray_num((void **)self->elementDatas);
	if(!(index < num)){
		return -1;
	}

	return index;
}

static gboolean cb_button_press_layer_view_content_(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	EtLayerView *self = (EtLayerView *)data;
	et_debug("BUTTON PRESS: (%4d, %4d)", (int)event->x, (int)event->y);
	et_mouse_util_button_kind(event->button);
	et_mouse_util_modifier_kind(event->state);

	int index = index_data_from_position_(self, event->x, event->y);
	et_debug("%d", index);
	if(0 <= index){
		PvFocus *focus = et_doc_get_focus_ref_from_id(self->doc_id);
		if(NULL == focus){
			et_error("");
			return false;
		}

		pv_focus_clear_set_element(focus, self->elementDatas[index]->element);

		if(!update_ui_(self)){
			et_error("");
			return false;
		}

		if(!et_doc_signal_update_from_id(self->doc_id)){
			et_error("");
			return false;
		}
	}

	return false;
}

static gboolean cb_button_release_layer_view_content_(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	return false;
}

static gboolean cb_motion_notify_layer_view_content_(
		GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	// et_debug("(%3d, %3d)", (int)event->x, (int)event->y);

	return false;
}

static gboolean cb_clicked_layer_ctrl_button_(
		GtkWidget *widget, gpointer data)
{
	EtLayerView *self = (EtLayerView *)data;
	et_assert(self);

	int index = -1;
	for(int i = 0; i < 4; i++){
		if(widget == self->button_layer_ctrls[i]){
			index = i;
			break;
		}
	}

	switch(index){
		case 0:
			if(!et_etaion_append_new_layer(self->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 1:
			if(!et_etaion_append_new_layer_child(self->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 2:
			if(!et_etaion_copy_layer(self->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 3:
			if(!et_etaion_remove_delete_layer(self->doc_id)){
				et_error("");
				return false;
			}
			break;
		default:
			et_error("");
			return false;
	}

	return false;
}

