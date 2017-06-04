#include "et_stroke_panel.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_doc.h"
#include "et_etaion.h"

typedef enum{
	STROKE_PROPERTY_WIDTH,
	STROKE_PROPERTY_LINECAP,
	STROKE_PROPERTY_LINEJOIN,
	STROKE_PROPERTY_NUM,		// number
}STROKE_PROPERTY;


void init_stroke_diffs_(bool *is_stroke_diffs)
{
	for(int i = 0; i < STROKE_PROPERTY_NUM; i++){
		is_stroke_diffs[i] = false;
	}
}

bool is_stroke_diffs_(bool *is_stroke_diffs)
{
	for(int i = 0; i < STROKE_PROPERTY_NUM; i++){
		if(is_stroke_diffs[i]){
			return true;
		}
	}

	return false;
}


struct EtStrokePanel{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;
	GtkWidget *frame_stroke;
	GtkWidget *box_stroke;
	GtkWidget *box_width;
	GtkWidget *label_width;
	GtkWidget *spin_width;
	GtkWidget *box_linecap;
	GtkWidget *label_linecap;
	GtkWidget *combo_linecap;
	GtkWidget *box_linejoin;
	GtkWidget *label_linejoin;
	GtkWidget *combo_linejoin;

	gulong stroke_property_signal_handler_id[STROKE_PROPERTY_NUM];
	//! PvStroke with focus elements is not compared.
	bool is_stroke_diffs[STROKE_PROPERTY_NUM];
	//! current PvStroke
	PvStroke stroke;
};

EtStrokePanel *stroke_panel = NULL;

static void _cb_value_changed_stroke_width_spin(GtkSpinButton *spin_button, gpointer user_data);
static void _cb_changed_linecap_with_combo(GtkComboBox *widget, gpointer user_data);
static void _cb_changed_linejoin_with_combo(GtkComboBox *widget, gpointer user_data);

static void _slot_change_doc_or_focus(EtDocId);

static void _update_ui_from_local();
static void _update_focus_elements_from_local();

EtStrokePanel *et_stroke_panel_init()
{
	assert(NULL == stroke_panel);

	EtStrokePanel *self = (EtStrokePanel *)malloc(sizeof(EtStrokePanel));
	if(NULL == self){
		et_critical("");
		return NULL;
	}


	self->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	self->frame_stroke = gtk_frame_new (NULL);
	gtk_frame_set_label(GTK_FRAME (self->frame_stroke), "Stroke");
	gtk_frame_set_shadow_type (GTK_FRAME (self->frame_stroke), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(self->box), self->frame_stroke, false, true, 0);

	self->box_stroke = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(self->frame_stroke), self->box_stroke);

	self->box_width = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(self->box_stroke), self->box_width, false, true, 0);
	self->label_width = gtk_label_new_with_mnemonic("width  ");
	gtk_box_pack_start(GTK_BOX(self->box_width), self->label_width, false, true, 0);
	self->spin_width = gtk_spin_button_new_with_range(0, 20000, 1);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON(self->spin_width), 3);
	gtk_box_pack_start(GTK_BOX(self->box_width), self->spin_width, false, true, 0);

	// ** linecap
	{
		self->box_linecap = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
		gtk_box_pack_start(GTK_BOX(self->box_stroke), self->box_linecap, false, true, 0);
		self->label_linecap = gtk_label_new_with_mnemonic("linecap  ");
		gtk_box_pack_start(GTK_BOX(self->box_linecap), self->label_linecap, false, true, 0);
		// self->combo_linecap = gtk_combo_box_new();
		//	GtkListStore *liststore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
		GtkListStore *liststore = gtk_list_store_new(1, G_TYPE_STRING);
		size_t num = get_num_stroke_linecap_infos();
		for(int i = 0; i < (int)num; i++){
			const PvStrokeLinecapInfo *info = get_stroke_linecap_info_from_id(i);
			gtk_list_store_insert_with_values(liststore, NULL, -1,
					0, info->name,
					//						1, info->linecap,
					-1);
		}
		self->combo_linecap = gtk_combo_box_new_with_model(GTK_TREE_MODEL(liststore));
		g_object_unref(liststore);
		GtkCellRenderer *column = gtk_cell_renderer_text_new();
		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(self->combo_linecap), column, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(self->combo_linecap), column,
				"text", 0, NULL);
		gtk_combo_box_set_active(GTK_COMBO_BOX(self->combo_linecap), 0);
		gtk_box_pack_start(GTK_BOX(self->box_linecap), self->combo_linecap, false, true, 0);
	}

	// ** linejoin
	{
		self->box_linejoin = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_pack_start(GTK_BOX(self->box_stroke), self->box_linejoin, false, true, 0);
		self->label_linejoin = gtk_label_new_with_mnemonic("linejoin  ");
		gtk_box_pack_start(GTK_BOX(self->box_linejoin), self->label_linejoin, false, true, 0);
		// self->combo_linejoin = gtk_combo_box_new();
		//	GtkListStore *liststore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
		GtkListStore *liststore = gtk_list_store_new(1, G_TYPE_STRING);
		size_t num = get_num_stroke_linejoin_infos();
		for(int i = 0; i < (int)num; i++){
			const PvStrokeLinejoinInfo *info = get_stroke_linejoin_info_from_id(i);
			gtk_list_store_insert_with_values(liststore, NULL, -1,
					0, info->name,
					//						1, info->linejoin,
					-1);
		}
		self->combo_linejoin = gtk_combo_box_new_with_model(GTK_TREE_MODEL(liststore));
		g_object_unref(liststore);
		GtkCellRenderer *column = gtk_cell_renderer_text_new();
		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(self->combo_linejoin), column, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(self->combo_linejoin), column,
				"text", 0, NULL);
		gtk_combo_box_set_active(GTK_COMBO_BOX(self->combo_linejoin), 0);
		gtk_box_pack_start(GTK_BOX(self->box_linejoin), self->combo_linejoin, false, true, 0);
	}

	self->stroke_property_signal_handler_id[STROKE_PROPERTY_WIDTH] = 
		g_signal_connect(self->spin_width, "value-changed",
				G_CALLBACK(_cb_value_changed_stroke_width_spin), NULL);
	self->stroke_property_signal_handler_id[STROKE_PROPERTY_LINECAP] = 
		g_signal_connect(self->combo_linecap, "changed",
				G_CALLBACK(_cb_changed_linecap_with_combo), NULL);
	self->stroke_property_signal_handler_id[STROKE_PROPERTY_LINEJOIN] = 
		g_signal_connect(self->combo_linejoin, "changed",
				G_CALLBACK(_cb_changed_linejoin_with_combo), NULL);

	self->widget = self->box;
	stroke_panel = self;

	self->stroke = PvStroke_Default;

	_update_ui_from_local();

	return self;
}

GtkWidget *et_stroke_panel_get_widget_frame()
{
	EtStrokePanel *self = stroke_panel;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	return self->box;
}

PvStroke et_stroke_panel_get_stroke()
{
	EtStrokePanel *self = stroke_panel;
	assert(self);

	return self->stroke;
}

void slot_et_stroke_panel_from_etaion_change_state(EtState state, gpointer data)
{
	_slot_change_doc_or_focus(state.doc_id);
}

void slot_et_stroke_panel_from_doc_change(EtDoc *doc, gpointer data)
{
	_slot_change_doc_or_focus(et_doc_get_id(doc));
}

static void _cb_value_changed_stroke_width_spin(GtkSpinButton *spin_button, gpointer user_data)
{
	EtStrokePanel *self = stroke_panel;
	assert(self);

	self->stroke.width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(self->spin_width));

	_update_focus_elements_from_local();
}

static void _cb_changed_linecap_with_combo(GtkComboBox *widget, gpointer user_data)
{
	EtStrokePanel *self = stroke_panel;
	assert(self);

	self->stroke.linecap = gtk_combo_box_get_active(GTK_COMBO_BOX(self->combo_linecap));

	_update_focus_elements_from_local();
}

static void _cb_changed_linejoin_with_combo(GtkComboBox *widget, gpointer user_data)
{
	EtStrokePanel *self = stroke_panel;
	assert(self);

	self->stroke.linejoin = gtk_combo_box_get_active(GTK_COMBO_BOX(self->combo_linejoin));

	_update_focus_elements_from_local();
}

static void _slot_change_doc_or_focus(EtDocId doc_id)
{
	EtStrokePanel *self = stroke_panel;
	assert(self);

	//! update stroke panel is only current document.
	if(doc_id != et_etaion_get_current_doc_id()){
		//! @fixme one happen call this line, because unknown.
		et_bug("");
		return;
	}

	if(doc_id < 0){
		return;
	}

	//! read stroke pair from focus element. update stroke pair.
	init_stroke_diffs_(self->is_stroke_diffs);
	PvStroke stroke = self->stroke;
	const PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	bool is_first = true;
	size_t num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num; i++){
		if(!pv_element_kind_is_viewable_object(focus->elements[i]->kind)){
			continue;
			et_debug("CON %d %zu", i, num);
		}
		if(is_first){
			is_first = false;
			// get focusing stroke from first focus element
			stroke = focus->elements[i]->stroke;
		}else{
			// compare stroke other focus elements
			PvStroke stroke0 = stroke;
			PvStroke stroke1 = focus->elements[i]->stroke;

			if(stroke0.width != stroke1.width){
				self->is_stroke_diffs[STROKE_PROPERTY_WIDTH] = true;
			}
			if(stroke0.linecap != stroke1.linecap){
				self->is_stroke_diffs[STROKE_PROPERTY_LINECAP] = true;
			}
			if(stroke0.linejoin != stroke1.linejoin){
				self->is_stroke_diffs[STROKE_PROPERTY_LINEJOIN] = true;
			}

		}
	}
	self->stroke = stroke;

	_update_ui_from_local();
}

static void _update_ui_from_local()
{
	EtStrokePanel *self = stroke_panel;
	assert(self);

	g_signal_handler_block(self->spin_width, self->stroke_property_signal_handler_id[STROKE_PROPERTY_WIDTH]);
	g_signal_handler_block(self->combo_linecap, self->stroke_property_signal_handler_id[STROKE_PROPERTY_LINECAP]);
	g_signal_handler_block(self->combo_linejoin, self->stroke_property_signal_handler_id[STROKE_PROPERTY_LINEJOIN]);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(self->spin_width), self->stroke.width);
	gtk_combo_box_set_active(GTK_COMBO_BOX(self->combo_linecap), self->stroke.linecap);
	gtk_combo_box_set_active(GTK_COMBO_BOX(self->combo_linejoin), self->stroke.linejoin);

	g_signal_handler_unblock(self->spin_width, self->stroke_property_signal_handler_id[STROKE_PROPERTY_WIDTH]);
	g_signal_handler_unblock(self->combo_linecap, self->stroke_property_signal_handler_id[STROKE_PROPERTY_LINECAP]);
	g_signal_handler_unblock(self->combo_linejoin, self->stroke_property_signal_handler_id[STROKE_PROPERTY_LINEJOIN]);

	//! @todo is_multi (ui design)
	gtk_label_set_text(GTK_LABEL(self->label_width),
			self->is_stroke_diffs[STROKE_PROPERTY_WIDTH]? "width *":"width  " );
	gtk_label_set_text(GTK_LABEL(self->label_linecap),
			self->is_stroke_diffs[STROKE_PROPERTY_LINECAP]? "linecap *":"linecap  " );
	gtk_label_set_text(GTK_LABEL(self->label_linejoin),
			self->is_stroke_diffs[STROKE_PROPERTY_LINEJOIN]? "linejoin *":"linejoin  " );
}

static void _update_focus_elements_from_local()
{
	EtStrokePanel *self = stroke_panel;
	assert(self);

	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//! when start app.
		et_debug("doc is nothing. %d", doc_id);
		return;
	}
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	assert(focus);

	size_t num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num; i++){
		focus->elements[i]->stroke = self->stroke;
	}

	et_doc_signal_update_from_id(doc_id);
	et_doc_save_from_id(doc_id);
}

