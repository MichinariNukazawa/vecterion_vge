#include "et_snap_panel.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_doc.h"
#include "et_etaion.h"

struct EtSnapPanel{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;
	GtkWidget *frame_snap;
	GtkWidget *box_snap;
	GtkWidget *toggle_snap_for_pixel;
};

EtSnapPanel *snap_panel = NULL;

static void _cb_toggled_from_check_snap_for_pixel(GtkToggleButton *button, gpointer user_data);

/*
static void _slot_change_doc_or_focus(EtDocId);

static void _update_ui_from_local();
static void _update_focus_elements_from_local();
*/

EtSnapPanel *et_snap_panel_init()
{
	assert(NULL == snap_panel);

	EtSnapPanel *self = (EtSnapPanel *)malloc(sizeof(EtSnapPanel));
	if(NULL == self){
		et_critical("");
		return NULL;
	}


	self->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

	self->frame_snap = gtk_frame_new (NULL);
	gtk_frame_set_label(GTK_FRAME (self->frame_snap), "Snap");
	gtk_frame_set_shadow_type (GTK_FRAME (self->frame_snap), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(self->box), self->frame_snap, false, true, 1);

	self->box_snap = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(self->frame_snap), self->box_snap);

	self->toggle_snap_for_pixel = gtk_toggle_button_new_with_mnemonic("SnapForPixel");
	assert(self->toggle_snap_for_pixel);
	gtk_box_pack_start(GTK_BOX(self->box_snap), self->toggle_snap_for_pixel, false, true, 1);

	g_signal_connect(self->toggle_snap_for_pixel, "toggled",
			G_CALLBACK(_cb_toggled_from_check_snap_for_pixel), NULL);

	self->widget = self->box;
	snap_panel = self;

	// _update_ui_from_local();

	return self;
}

GtkWidget *et_snap_panel_get_widget_frame()
{
	EtSnapPanel *self = snap_panel;
	if(NULL == self){
		et_bug("");
		exit(-1);
	}

	return self->box;
}

EtSnap et_snap_panel_get_snap()
{
	EtSnapPanel *self = snap_panel;
	assert(self);

	EtSnap snap = EtSnap_Default;
	snap.is_snap_for_pixel = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(self->toggle_snap_for_pixel));

	return snap;
}

/*
void slot_et_snap_panel_from_etaion_change_state(EtState state, gpointer data)
{
	_slot_change_doc_or_focus(state.doc_id);
}

void slot_et_snap_panel_from_doc_change(EtDoc *doc, gpointer data)
{
	_slot_change_doc_or_focus(et_doc_get_id(doc));
}
*/

static void _cb_toggled_from_check_snap_for_pixel(GtkToggleButton *button, gpointer user_data)
{
	EtSnapPanel *self = snap_panel;
	assert(self);

	bool is_a = gtk_toggle_button_get_active(button);
	et_debug("XXXX %d", is_a);

	// _update_focus_elements_from_local();
}

/*
static void _slot_change_doc_or_focus(EtDocId doc_id)
{
	EtSnapPanel *self = snap_panel;
	assert(self);

	//! update snap panel is only current document.
	if(doc_id != et_etaion_get_current_doc_id()){
		//! @fixme one happen call this line, because unknown.
		et_bug("");
		return;
	}

	if(doc_id < 0){
		return;
	}

	//! read snap pair from focus element. update snap pair.
	self->is_multi = false;
	PvSnap snap = self->snap;
	const PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	bool is_first = true;
	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
		if(!pv_element_kind_is_viewable_object(focus->elements[i]->kind)){
			continue;
			et_debug("CON %d %d", i, num);
		}
		if(is_first){
			is_first = false;
			// get focusing snap from first focus element
			snap = focus->elements[i]->snap;
		}else{
			// compare snap other focus elements
			if(!pv_snap_is_equal(snap, focus->elements[i]->snap)){
				self->is_multi = true;
			}
		}
	}
	self->snap = snap;

	_update_ui_from_local();
}

static void _update_ui_from_local()
{
	EtSnapPanel *self = snap_panel;
	assert(self);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(self->spin_width), self->snap.width);
	gtk_combo_box_set_active(GTK_COMBO_BOX(self->combo_linecap), self->snap.linecap);
	gtk_combo_box_set_active(GTK_COMBO_BOX(self->combo_linejoin), self->snap.linejoin);

	//! @todo is_multi (ui design)
	gtk_label_set_text(GTK_LABEL(self->label_width), self->is_multi? "width *":"width  " );
	gtk_label_set_use_underline (GTK_LABEL(self->label_width), self->is_multi);
}

static void _update_focus_elements_from_local()
{
	EtSnapPanel *self = snap_panel;
	assert(self);

	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//! when start app.
		et_debug("doc is noting. %d", doc_id);
		return;
	}
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	assert(focus);

	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
		focus->elements[i]->snap = self->snap;
	}

	et_doc_signal_update_from_id(doc_id);
	et_doc_save_from_id(doc_id);
}
*/


void slot_et_snap_panel_from_etaion_change_state(EtState state, gpointer data)
{
}

void slot_et_snap_panel_from_doc_change(EtDoc *doc, gpointer data)
{
}

