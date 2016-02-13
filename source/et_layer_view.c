#include "et_layer_view.h"

#include <stdlib.h>
#include <string.h>
#include "et_error.h"
#include "et_doc_manager.h"
#include "pv_element.h"


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
	GtkWidget *text;

	EtDocId doc_id;
	// buffer
	EtLayerViewElementData **elementDatas;
};

typedef struct _EtLayerViewRltDataPack{
	EtLayerViewElementData **datas;
}EtLayerViewRltDataPack;


static EtLayerView *layer_view = NULL;

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

	this->text = gtk_text_view_new();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (this->text));
	gtk_text_buffer_set_text (buffer, "default scale", -1);
	gtk_text_view_set_editable (GTK_TEXT_VIEW(this->text), false);
	gtk_container_add(GTK_CONTAINER(this->scroll), this->text);

	this->widget = this->box;

	this->doc_id = -1;
	this->elementDatas = NULL;

	layer_view = this;

	return this;
}

GtkWidget *et_layer_view_get_widget_frame(EtLayerView *this)
{
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
		unsigned long debug_pointer = 0;
		memcpy(&debug_pointer, &data->element, sizeof(unsigned long));

		char str_tmp[128];
		snprintf(str_tmp, sizeof(str_tmp),
				"%c%03d:%03d:%08lx '%s'\n",
				((focus.element == data->element)? '>':' '),
				data->level, data->kind,
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

	return true;
}

bool _et_layer_view_slot_update_doc()
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
	if(!pv_element_recursive(vg->element_root,
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

bool et_layer_view_set_doc_id(EtLayerView *this, EtDocId doc_id)
{
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	this->doc_id = doc_id;

	if(!_et_layer_view_slot_update_doc()){
		et_error("");
		return false;
	}

	return true;
}

void et_layer_view_slot_from_doc_change(EtDoc *doc, gpointer data)
{
	if(!_et_layer_view_slot_update_doc()){
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
