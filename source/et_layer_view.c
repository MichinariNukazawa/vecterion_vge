#include "et_layer_view.h"

#include <stdlib.h>
#include <string.h>
#include "et_error.h"
#include "et_doc_manager.h"
#include "pv_element.h"
#include "et_mouse_util.h"
#include "et_etaion.h"

static char *DIRECTORY_RESOURCE = "./resource";

typedef struct _EtLayerViewLayerCtrl{
	const char * const name;
	const char * const filename_image;
}EtLayerViewLayerCtrl;

const EtLayerViewLayerCtrl _et_layer_view_layer_ctrls[] = {
	{"New", "layer_new_64x64.svg",},
	{"Child", "layer_new_child_64x64.svg",},
	{"Clone", "layer_clone_64x64.svg",},
	{"Delete", "layer_del_64x64.svg",},
};

typedef struct _EtLayerViewElementData{
	int level;
	char *name;
	PvElementKind kind;
	PvElement *element;
}EtLayerViewElementData;

struct _EtLayerView{
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

typedef struct _EtLayerViewRltDataPack{
	EtLayerViewElementData **datas;
}EtLayerViewRltDataPack;


static EtLayerView *layer_view = NULL;

gboolean _et_layer_view_cb_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean _et_layer_view_cb_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean _et_layer_view_cb_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data);

gboolean _et_layer_view_cb_layer_ctrl(GtkWidget *widget, gpointer data);

bool _et_layer_view_set_layer_ctrl(EtLayerView *this, int index)
{
	GError *error = NULL;
	GdkPixbuf *pixbuf = NULL;
	char tmp[1024];
	snprintf(tmp, sizeof(tmp), "%s/%s",
			DIRECTORY_RESOURCE, _et_layer_view_layer_ctrls[index].filename_image);
	pixbuf = gdk_pixbuf_new_from_file(tmp, &error);
	if(NULL == pixbuf){
		et_error("%d, '%s'\n", index, tmp);
		return false;
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
	if(NULL == image){
		et_error("");
		return false;
	}

	this->button_layer_ctrls[index] = gtk_button_new();
	if(NULL == this->button_layer_ctrls[index]){
		et_error("");
		return false;
	}
	// フォーカス状態でEnterKey押下した際に、誤作動する問題への対処
	gtk_button_set_focus_on_click(GTK_BUTTON(this->button_layer_ctrls[index]), false);
	// gtk_widget_set_can_focus(this->button_layer_ctrls[index], false);
	gtk_container_add(GTK_CONTAINER(this->button_layer_ctrls[index]),
			image);

	gtk_container_add(GTK_CONTAINER(this->box_button_layer_ctrl),
			this->button_layer_ctrls[index]);

	g_signal_connect(this->button_layer_ctrls[index], "clicked",
			G_CALLBACK(_et_layer_view_cb_layer_ctrl), (gpointer)this);

	// デフォルト無効からスタート
	gtk_widget_set_sensitive(this->button_layer_ctrls[index], false);

	return true;
}

EtLayerView *et_layer_view_init()
{
	if(NULL != layer_view){
		et_bug("");
		exit(-1);
	}

	EtLayerView *this = (EtLayerView *)malloc(sizeof(EtLayerView));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	this->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

	this->scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(GTK_WIDGET(this->scroll), TRUE);  
	gtk_widget_set_vexpand(GTK_WIDGET(this->scroll), TRUE);
	gtk_container_add(GTK_CONTAINER(this->box), this->scroll);

	this->event_box = gtk_event_box_new();
	if(NULL == this->event_box){
		et_error("");
		return NULL;
	}
	gtk_widget_set_events(this->event_box,
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK
			);
	g_signal_connect(this->event_box, "button-press-event",
			G_CALLBACK(_et_layer_view_cb_button_press), (gpointer)this);
	g_signal_connect(this->event_box, "button-release-event",
			G_CALLBACK(_et_layer_view_cb_button_release), (gpointer)this);
	g_signal_connect(this->event_box, "motion-notify-event",
			G_CALLBACK(_et_layer_view_cb_motion_notify), (gpointer)this);

	gtk_container_add(GTK_CONTAINER(this->scroll), this->event_box);

	this->text = gtk_text_view_new();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (this->text));
	gtk_text_buffer_set_text (buffer, "default scale", -1);
	gtk_text_view_set_editable (GTK_TEXT_VIEW(this->text), false);
	gtk_text_view_set_monospace (GTK_TEXT_VIEW(this->text), TRUE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(this->text), false);
	gtk_container_add(GTK_CONTAINER(this->event_box), this->text);

	this->box_button_layer_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(this->box), this->box_button_layer_ctrl);

	if(!_et_layer_view_set_layer_ctrl(this, 0)){
		et_error("");
		return NULL;
	}
	if(!_et_layer_view_set_layer_ctrl(this, 1)){
		et_error("");
		return NULL;
	}
	if(!_et_layer_view_set_layer_ctrl(this, 2)){
		et_error("");
		return NULL;
	}
	if(!_et_layer_view_set_layer_ctrl(this, 3)){
		et_error("");
		return NULL;
	}

	this->widget = this->box;

	this->doc_id = -1;
	this->elementDatas = NULL;

	layer_view = this;

	return this;
}

GtkWidget *et_layer_view_get_widget_frame()
{
	EtLayerView *this = layer_view;
	if(NULL == this){
		et_error("");
		return NULL;
	}

	return this->widget;
}

bool _et_layer_view_read_layer_tree(PvElement *element, gpointer data, int level)
{
	EtLayerViewRltDataPack *func_rlt_data_pack = data;
	EtLayerViewElementData ***datas = &(func_rlt_data_pack->datas);

	EtLayerViewElementData *rlt = malloc(sizeof(EtLayerViewElementData));
	if(NULL == rlt){
		et_error("");
		exit(-1);
	}
	rlt->level = level;
	rlt->name = "";
	rlt->kind = element->kind;
	rlt->element = element;

	int num = pv_general_get_parray_num((void **)*datas);
	*datas = (EtLayerViewElementData **)
			realloc(*datas, sizeof(EtLayerViewElementData *) * (num + 2));
	if(NULL == *datas){
		et_critical("");
		exit(-1);
	}

	(*datas)[num] = rlt;
	(*datas)[num + 1] = NULL;
	
	return true;
}

bool _et_layer_view_draw(EtLayerView *this)
{
	if(NULL == this){
		et_bug("");
		return false;
	}

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id(this->doc_id, &is_error);
	if(is_error){
		et_error("");
		return false;
	}

	EtLayerViewElementData **elementDatas = this->elementDatas;

	char buf[10240];
	buf[0] = '\0';
	int num = pv_general_get_parray_num((void **)elementDatas);
	for(int i = 0; i < num; i++){
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

		char str_head[128];
		str_head[0] = '\0';
		for(int i = 0; i < data->level; i++){
			str_head[i] = '_';
			str_head[i + 1] = '\0';
			if(!(i < ((int)sizeof(str_head) - 2))){
				break;
			}
		}
		const char *kind_name = pv_element_get_name_from_kind(data->kind);
		if(NULL == kind_name){
			kind_name = "";
		}
		char str_tmp[128];
		snprintf(str_tmp, sizeof(str_tmp),
				"%s%c:%s\t:%08lx '%s'\n",
				str_head,
				((focus.element == data->element)? '>':'_'),
				//data->level,
				kind_name,
				debug_pointer,
				((data->name)?"":data->name));

		strncat(buf, str_tmp, (sizeof(buf) - 1));
		buf[sizeof(buf)-1] = '\0';
		if(!(strlen(buf) < (sizeof(buf) - 2))){
			et_warning("%lu/%lu\n", strlen(buf), sizeof(buf));
		}
	}

	GtkTextBuffer *buffer = gtk_text_view_get_buffer(
					GTK_TEXT_VIEW (this->text));
	gtk_text_buffer_set_text (buffer, buf, -1);

	// ターゲット状態でlayer_ctrlsのbutton状態を変更する
	gtk_widget_set_sensitive(this->button_layer_ctrls[0], (0 <= this->doc_id));
	gtk_widget_set_sensitive(this->button_layer_ctrls[1], (NULL != focus.element));
	gtk_widget_set_sensitive(this->button_layer_ctrls[2], (NULL != focus.element));
	gtk_widget_set_sensitive(this->button_layer_ctrls[3], (NULL != focus.element));

	return true;
}

bool _et_layer_view_update_doc_tree()
{
	EtLayerView *this = layer_view;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	EtDoc *doc = et_doc_manager_get_doc_from_id(this->doc_id);
	if(NULL == doc){
		et_error("");
		return false;
	}

	PvVg *vg = et_doc_get_vg_ref(doc);
	if(NULL == vg){
		et_error("");
		return false;
	}

	EtLayerViewRltDataPack func_rlt_data_pack = {
		.datas = NULL
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_before(vg->element_root,
				_et_layer_view_read_layer_tree, &func_rlt_data_pack,
				&error)){
		et_error("level:%d", error.level);
		return false;
	}

	EtLayerViewElementData **before = this->elementDatas;
	this->elementDatas = func_rlt_data_pack.datas;
	if(NULL != before){
		int num = pv_general_get_parray_num((void **)before);
		for(int i = 0; i < num; i++){
			EtLayerViewElementData *data = before[i];
			free(data);
		}
	}

	return _et_layer_view_draw(this);
}

bool et_layer_view_set_doc_id(EtDocId doc_id)
{
	EtLayerView *this = layer_view;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	this->doc_id = doc_id;

	if(!_et_layer_view_update_doc_tree()){
		et_error("");
		return false;
	}

	return true;
}

void et_layer_view_slot_from_doc_change(EtDoc *doc, gpointer data)
{
	if(!_et_layer_view_update_doc_tree()){
		et_error("");
	}
}

void et_layer_view_slot_from_etaion_change_state(EtState state, gpointer data)
{
	EtLayerView *this = layer_view;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	if(!_et_layer_view_draw(this)){
		et_error("");
		return;
	}
}

int _et_layer_view_index_data_from_position(EtLayerView *this, int x, int y)
{
	if(NULL == this){
		et_bug("");
		return -1;
	}
	int index = (y/16) + 1; // +1 = hidden root layer

	int num = pv_general_get_parray_num((void **)this->elementDatas);
	if(!(index < num)){
		return -1;
	}

	return index;
}

gboolean _et_layer_view_cb_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	EtLayerView *this = (EtLayerView *)data;
	et_debug("BUTTON PRESS: (%4d, %4d)\n", (int)event->x, (int)event->y);
	et_mouse_util_button_kind(event->button);
	et_mouse_util_modifier_kind(event->state);

	int index = _et_layer_view_index_data_from_position(this, event->x, event->y);
	et_debug("%d\n", index);
	if(0 <= index){
		bool is_error = true;
		PvFocus focus = et_doc_get_focus_from_id(this->doc_id, &is_error);
		if(is_error){
			et_error("");
			return false;
		}

		focus.element = this->elementDatas[index]->element;

		if(!et_doc_set_focus_to_id(this->doc_id, focus)){
			et_error("");
			return false;
		}

		if(!_et_layer_view_draw(this)){
			et_error("");
			return false;
		}

		if(!et_doc_signal_update_from_id(this->doc_id)){
			et_error("");
			return false;
		}
	}

	return false;
}

gboolean _et_layer_view_cb_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	et_debug("BUTTON RELEASE\n");
	return false;
}

gboolean _et_layer_view_cb_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	// et_debug("(%3d, %3d)\n", (int)event->x, (int)event->y);

	return false;
}

gboolean _et_layer_view_cb_layer_ctrl(GtkWidget *widget, gpointer data)
{
	EtLayerView *this = (EtLayerView *)data;
	if(NULL == this){
		et_bug("");
		return false;
	}

	int index = -1;
	for(int i = 0; i < 4; i++){
		if(widget == this->button_layer_ctrls[i]){
			index = i;
			break;
		}
	}

	switch(index){
		case 0:
			if(!et_etaion_add_new_layer(this->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 1:
			if(!et_etaion_add_new_layer_child(this->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 2:
			if(!et_etaion_copy_layer(this->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 3:
			if(!et_etaion_remove_delete_layer(this->doc_id)){
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
