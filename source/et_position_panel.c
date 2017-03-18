#include "et_position_panel.h"

#include <stdlib.h>
#include "et_error.h"
#include "pv_element_info.h"
#include "et_doc.h"
#include "et_etaion.h"
#include "et_tool_info.h"

typedef enum{
	PvPositionIndex_X,
	PvPositionIndex_Y,
	PvPositionIndex_W,
	PvPositionIndex_H,
}PvPositionIndex;
#define NUM_PV_POSITION (4)

typedef struct{
	double p[NUM_PV_POSITION];
}PvPosition;

PvPosition PvPosition_Default = {{0, 0, 0, 0}};

static PvPosition _pv_position_sub(PvPosition p0, PvPosition p1);

typedef struct{
	PvPositionIndex index;
	const char *name;
}PvPositionInfo;

const PvPositionInfo position_infos[] = {
	{PvPositionIndex_X, "x",},
	{PvPositionIndex_Y, "y",},
	{PvPositionIndex_W, "w",},
	{PvPositionIndex_H, "h",},
};

PvPosition _get_position_from_rect(PvRect);
PvRect _get_rect_from_position(PvPosition);

PvPosition _get_position_from_rect(PvRect rect)
{
	PvPosition position = {
		.p = {
			rect.x,
			rect.y,
			rect.w,
			rect.h,
		},
	};

	return position;
}

PvRect _get_rect_from_position(PvPosition position)
{
	PvRect rect = {
		.x = position.p[PvPositionIndex_X],
		.y = position.p[PvPositionIndex_Y],
		.w = position.p[PvPositionIndex_W],
		.h = position.p[PvPositionIndex_H],
	};

	return rect;
}
struct EtPositionPanel{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;

	GtkWidget *label_positions[NUM_PV_POSITION];
	GtkWidget *spin_positions[NUM_PV_POSITION];

	PvPosition position;
	//! difference of previouse(change to focus element from spin)
	PvPosition position_diff;
};

EtPositionPanel *position_panel = NULL;

static void _cb_value_changed_positions_spin(GtkSpinButton *spin_button, gpointer user_data);

static gboolean _cb_button_release_event_spin_positions(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//! when start app.
		et_debug("doc is nothing. %d", doc_id);
		return false;
	}

	et_doc_save_from_id(doc_id);
	return false;
}

static void _slot_change_doc_or_focus(EtDocId);

static void _update_ui_from_local();
static void _update_focus_element_or_anchor_point_from_local();

EtPositionPanel *et_position_panel_init()
{
	assert(NULL == position_panel);

	EtPositionPanel *self = (EtPositionPanel *)malloc(sizeof(EtPositionPanel));
	assert(self);

	self->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);

	for(int i = 0; i < NUM_PV_POSITION; i++){
		const PvPositionInfo *info = &(position_infos[i]);
		self->label_positions[i] = gtk_label_new_with_mnemonic(info->name);
		assert(self->label_positions[i]);
		gtk_box_pack_start(GTK_BOX(self->box), self->label_positions[i], false, true, 3);

		//! @fixme if element range over is not show for user and debug message.
		self->spin_positions[i] = gtk_spin_button_new_with_range(-20000, 20000, 1);
		assert(self->spin_positions[i]);
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON(self->spin_positions[i]), 3);
		gtk_box_pack_start(GTK_BOX(self->box), self->spin_positions[i], false, true, 3);

		//! @todo implement resize
		if(PvPositionIndex_W == i || PvPositionIndex_H == i){
			gtk_widget_set_sensitive(self->spin_positions[i], false);
		}

		g_signal_connect(self->spin_positions[i], "value-changed",
				G_CALLBACK(_cb_value_changed_positions_spin), (void *)info);

		g_signal_connect(self->spin_positions[i], "button-release-event",
				G_CALLBACK(_cb_button_release_event_spin_positions), (void *)info);
	}

	self->widget = self->box;
	position_panel = self;

	self->position = PvPosition_Default;
	self->position_diff = PvPosition_Default;

	_update_ui_from_local();

	return self;
}

GtkWidget *et_position_panel_get_widget_frame()
{
	EtPositionPanel *self = position_panel;
	assert(self);

	return self->box;
}

PvPosition et_position_panel_get_position()
{
	EtPositionPanel *self = position_panel;
	assert(self);

	return self->position;
}

void slot_et_position_panel_from_etaion_change_state(EtState state, gpointer data)
{
	_slot_change_doc_or_focus(state.doc_id);
}

void slot_et_position_panel_from_etaion_change_tool_id(EtToolId tool_id, gpointer data)
{
	_slot_change_doc_or_focus(et_etaion_get_current_doc_id());
}

void slot_et_position_panel_from_doc_change(EtDoc *doc, gpointer data)
{
	_slot_change_doc_or_focus(et_doc_get_id(doc));
}

static void _cb_value_changed_positions_spin(GtkSpinButton *spin_button, gpointer user_data)
{
	EtPositionPanel *self = position_panel;
	assert(self);

	const PvPositionInfo *info = (const PvPositionInfo *)user_data;
	assert(info);

	PvPosition prev = self->position;
	self->position.p[info->index] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(self->spin_positions[info->index]));
	self->position_diff = _pv_position_sub(self->position, prev);

	_update_focus_element_or_anchor_point_from_local();
}

static void _slot_change_doc_or_focus(EtDocId doc_id)
{
	EtPositionPanel *self = position_panel;
	assert(self);

	//! update position panel is only current document.
	if(doc_id != et_etaion_get_current_doc_id()){
		//! @fixme one happen call this line, because unknown.
		et_bug("");
		return;
	}

	if(doc_id < 0){
		return;
	}

	EtToolId tool_id = et_etaion_get_tool_id();
	const EtToolInfo *tool_info = et_tool_get_info_from_id(tool_id);
	et_assertf(tool_info, "%d", tool_id);

	PvRect rect = _get_rect_from_position(self->position);
	const PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	bool is_first = true;
	if(tool_info->is_element_tool){
		int num = pv_general_get_parray_num((void **)focus->elements);
		for(int i = 0; i < num; i++){
			if(!pv_element_kind_is_viewable_object(focus->elements[i]->kind)){
				continue;
			}

			const PvElementInfo *info = pv_element_get_info_from_kind(focus->elements[i]->kind);
			PvRect rect_ = info->func_get_rect_by_anchor_points(focus->elements[i]);

			if(is_first){
				is_first = false;
				// get focusing position from first focus element
				rect = rect_;
			}else{
				// expand position other focus elements
				rect = pv_rect_expand(rect, rect_);
			}
		}
	}else{
		int num = pv_general_get_parray_num((void **)focus->anchor_points);
		for(int i = 0; i < num; i++){
			PvPoint p = pv_anchor_point_get_point(focus->anchor_points[i]);
			PvRect rect_ = (PvRect){
				.x = p.x,
					.y = p.y,
					.w = 0,
					.h = 0,
			};
			if(is_first){
				is_first = false;
				rect = rect_;
			}else{
				rect = pv_rect_expand(rect, rect_);
			}
		}
	}
	self->position = _get_position_from_rect(rect);

	_update_ui_from_local();
}

static void _update_ui_from_local()
{
	EtPositionPanel *self = position_panel;
	assert(self);

	for(int i = 0; i < NUM_PV_POSITION; i++){
		const PvPositionInfo *info = &(position_infos[i]);
		gtk_spin_button_set_value(
				GTK_SPIN_BUTTON(self->spin_positions[info->index]),
				self->position.p[info->index]);
	}
}

static void _update_focus_element_or_anchor_point_from_local()
{
	EtPositionPanel *self = position_panel;
	assert(self);

	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//! when start app.
		et_debug("doc is nothing. %d", doc_id);
		return;
	}
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	assert(focus);

	EtToolId tool_id = et_etaion_get_tool_id();
	const EtToolInfo *tool_info = et_tool_get_info_from_id(tool_id);
	et_assertf(tool_info, "%d", tool_id);

	if(tool_info->is_element_tool){
		int num = pv_general_get_parray_num((void **)focus->elements);
		for(int i = 0; i < num; i++){
			const PvElementInfo *info = pv_element_get_info_from_kind(focus->elements[i]->kind);
			bool ret = info->func_move_element(focus->elements[i],
					self->position_diff.p[PvPositionIndex_X],
					self->position_diff.p[PvPositionIndex_Y]);
			assert(ret);
			//! @todo implement resize
		}
	}else{
		int num = pv_general_get_parray_num((void **)focus->anchor_points);
		for(int i = 0; i < num; i++){
			PvElement *element = pv_element_get_in_elements_from_member_anchor_point(
					focus->elements,
					focus->anchor_points[i]);
			et_assert(element);

			const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
			et_assert(info);

			PvPoint p = pv_anchor_point_get_point(focus->anchor_points[i]);
			p.x += self->position_diff.p[PvPositionIndex_X];
			p.y += self->position_diff.p[PvPositionIndex_Y];
			info->func_set_anchor_point_point(element, focus->anchor_points[i], p);
		}
	}

	et_doc_signal_update_from_id(doc_id);
}

static PvPosition _pv_position_sub(PvPosition p0, PvPosition p1)
{
	PvPosition ret = PvPosition_Default;
	for(int i = 0; i < NUM_PV_POSITION; i++){
		ret.p[i] = p0.p[i] - p1.p[i];
	}

	return ret;
}

