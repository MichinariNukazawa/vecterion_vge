#include "et_color_panel.h"

#include <stdlib.h>
#include "pv_cairo.h"
#include "et_doc.h"
#include "et_etaion.h"
#include "et_error.h"

const int W_PALLET = 100;
const int H_PALLET = 60;
const int BORDER_PALLET = 2;

struct EtColorPanel{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;
	GtkWidget *frame;
	GtkWidget *box_color_panel;
	GtkWidget *event_box_color_sliders;
	GtkWidget *box_color_sliders;
	GtkWidget *slider_boxs[NUM_COLOR_PARAMETER];
	GtkWidget *slider_labels[NUM_COLOR_PARAMETER];
	GtkWidget *slider_sliders[NUM_COLOR_PARAMETER];
	GtkWidget *slider_spins[NUM_COLOR_PARAMETER];
	GtkWidget *event_box_pallet;
	GtkWidget *pallet;

	//! edit in pallet color.
	PvColorPairGround color_pair_ground;
	//! color with focus elements is not compared.
	bool is_multi_colors[PvColorPairGrounds];
	//! current color_pair
	PvColorPair color_pair;
};

EtColorPanel *color_panel_ = NULL;

static gboolean cb_button_press_pallet_(
		GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean cb_button_release_pallet_(
		GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean cb_expose_event_pallet_(GtkWidget *widget, cairo_t *cr, gpointer data);


static gboolean cb_button_release_event_box_color_sliders_(
		GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean cb_button_release_slider_spins_(
		GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean cb_change_value_color_slider_slider_(
		GtkRange *range, GtkScrollType scroll,
		gdouble value, gpointer user_data);
static void cb_value_changed_color_slider_spin_(
		GtkSpinButton *spin_button,
		GtkScrollType  scroll,
		gpointer       user_data);

static void et_color_panel_update_ui_();
static void et_color_panel_update_focus_elements_();

EtColorPanel *et_color_panel_init()
{
	et_assert(NULL == color_panel_);

	EtColorPanel *self = (EtColorPanel *)malloc(sizeof(EtColorPanel));
	et_assert(self);

	self->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	self->frame = gtk_frame_new (NULL);
	gtk_frame_set_label(GTK_FRAME (self->frame), "Color");
	gtk_frame_set_shadow_type (GTK_FRAME (self->frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(self->box), self->frame, true, true, 0);

	self->box_color_panel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(self->frame), self->box_color_panel);

	// **sliders
	self->event_box_color_sliders = gtk_event_box_new();
	gtk_widget_set_events(self->event_box_color_sliders,
			GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			);
	g_signal_connect(self->event_box_color_sliders, "button-release-event",
			G_CALLBACK(cb_button_release_event_box_color_sliders_), NULL);
	self->box_color_sliders = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_container_add(GTK_CONTAINER(self->event_box_color_sliders),
			self->box_color_sliders);

	for(int i = 0; i < NUM_COLOR_PARAMETER; i++){
		const PvColorParameterProperty *color_parameter_property
			= pv_color_get_parameter_property_from_ix(i);

		self->slider_boxs[i] = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
		et_assertf(self->slider_boxs[i], "%d", i);
		gtk_box_pack_start(GTK_BOX(self->box_color_sliders), self->slider_boxs[i], true, true, 1);

		self->slider_labels[i] = gtk_label_new_with_mnemonic(color_parameter_property->name);
		et_assertf(self->slider_labels[i], "%d", i);
		gtk_box_pack_start(GTK_BOX(self->slider_boxs[i]), self->slider_labels[i], false, false, 1);

		self->slider_sliders[i] = gtk_scale_new_with_range(
				GTK_ORIENTATION_HORIZONTAL,
				color_parameter_property->min,
				color_parameter_property->max,
				1);
		et_assertf(self->slider_sliders[i], "%d", i);
		gtk_scale_set_draw_value(GTK_SCALE(self->slider_sliders[i]), false);
		gtk_scale_set_value_pos(GTK_SCALE(self->slider_sliders[i]), GTK_POS_RIGHT);
		gtk_box_pack_start(GTK_BOX(self->slider_boxs[i]), self->slider_sliders[i], true, true, 1);
		g_signal_connect(G_OBJECT(self->slider_sliders[i]), "change-value",
				G_CALLBACK(cb_change_value_color_slider_slider_),
				(gpointer)color_parameter_property);

		self->slider_spins[i] = gtk_spin_button_new_with_range(
				color_parameter_property->min,
				color_parameter_property->max,
				1);
		et_assertf(self->slider_spins[i], "%d", i);
		gtk_box_pack_start(GTK_BOX(self->slider_boxs[i]), self->slider_spins[i], false, false, 1);
		g_signal_connect(G_OBJECT(self->slider_spins[i]), "value-changed",
				G_CALLBACK(cb_value_changed_color_slider_spin_),
				(gpointer)color_parameter_property);
		// spin button [+/-] is event_box not catch button-release-event.
		g_signal_connect(self->slider_spins[i], "button-release-event",
				G_CALLBACK(cb_button_release_slider_spins_), NULL);
	}

	// ** pallet
	self->event_box_pallet = gtk_event_box_new();
	gtk_widget_set_events(self->event_box_pallet,
			GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_POINTER_MOTION_MASK
			);
	g_signal_connect(self->event_box_pallet, "button-press-event",
			G_CALLBACK(cb_button_press_pallet_), NULL);
	g_signal_connect(self->event_box_pallet, "button-release-event",
			G_CALLBACK(cb_button_release_pallet_), NULL);

	self->pallet = gtk_drawing_area_new();
	g_signal_connect (G_OBJECT (self->pallet), "draw",
			G_CALLBACK (cb_expose_event_pallet_), NULL);
	gtk_container_add(GTK_CONTAINER(self->event_box_pallet), self->pallet);
	gtk_widget_set_size_request(self->pallet, (W_PALLET * 2), (H_PALLET * 1.5));

	self->is_multi_colors[PvColorPairGround_ForGround] = false;
	self->is_multi_colors[PvColorPairGround_BackGround] = false;
	self->color_pair = PvColorPair_Default;
	self->color_pair_ground = PvColorPairGround_ForGround;

	gtk_box_pack_start(GTK_BOX(self->box_color_panel), self->event_box_pallet, false, true, 0);
	gtk_box_pack_start(GTK_BOX(self->box_color_panel), self->event_box_color_sliders, true, true, 0);

	self->widget = self->box;
	color_panel_ = self;

	et_color_panel_update_ui_();

	return self;
}

GtkWidget *et_color_panel_get_widget_frame()
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	return self->box;
}

PvColorPair et_color_panel_get_color_pair()
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	return self->color_pair;
}

void slot_change_doc_or_focus_(EtDocId doc_id)
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	//! update color panel is only current document.
	if(doc_id != et_etaion_get_current_doc_id()){
		//! @fixme one happen call this line, because unknown.
		et_bug("");
		return;
	}

	if(doc_id < 0){
		return;
	}

	//! read color pair from focus element. update color pair.
	self->is_multi_colors[PvColorPairGround_ForGround] = false;
	self->is_multi_colors[PvColorPairGround_BackGround] = false;
	PvColorPair color_pair = self->color_pair;
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
			// get focusing color from first focus element
			color_pair = focus->elements[i]->color_pair;
		}else{
			// compare color other focus elements
			if(!pv_color_is_equal(color_pair.colors[PvColorPairGround_ForGround],
						focus->elements[i]->color_pair.colors[PvColorPairGround_ForGround])){
				self->is_multi_colors[PvColorPairGround_ForGround] = true;
			}
			if(!pv_color_is_equal(color_pair.colors[PvColorPairGround_BackGround],
						focus->elements[i]->color_pair.colors[PvColorPairGround_BackGround])){
				self->is_multi_colors[PvColorPairGround_BackGround] = true;
			}
		}
	}
	self->color_pair = color_pair;

	et_color_panel_update_ui_();
}

void slot_et_color_panel_from_etaion_change_state(EtState state, gpointer data)
{
	slot_change_doc_or_focus_(state.doc_id);
}

void slot_et_color_panel_from_doc_change(EtDoc *doc, gpointer data)
{
	slot_change_doc_or_focus_(et_doc_get_id(doc));
}

static gboolean cb_button_release_slider_spins_(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	return cb_button_release_event_box_color_sliders_(
			widget, event, data);
}

/*! @brief callback from completed color element slider. */
static gboolean cb_button_release_event_box_color_sliders_(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//! when start app.
		et_debug("doc is nothing. %d", doc_id);
		return FALSE;
	}

	et_doc_save_from_id(doc_id);

	et_color_panel_update_ui_();

	return FALSE;
}

/*! @brief callback from move the color element slider. */
static gboolean cb_change_value_color_slider_slider_(
		GtkRange *range, GtkScrollType scroll,
		gdouble value, gpointer user_data)
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	const PvColorParameterProperty *color_parameter_property = user_data;
	et_assert(color_parameter_property);

	//! limitation to GtkRange spec is over range max
	if(color_parameter_property->max < value){
		et_bug("%f", value);
		value = color_parameter_property->max;
	}

	pv_color_set_parameter(
			&(self->color_pair.colors[self->color_pair_ground]),
			color_parameter_property->ix,
			value);

	et_color_panel_update_focus_elements_();

	et_color_panel_update_ui_();

	return FALSE;
}

static void cb_value_changed_color_slider_spin_(
		GtkSpinButton *spin_button,
		GtkScrollType  scroll,
		gpointer       user_data)
{
	double value = gtk_spin_button_get_value(spin_button);
	et_debug("%d", (int)value);

	EtColorPanel *self = color_panel_;
	et_assert(self);

	const PvColorParameterProperty *color_parameter_property = user_data;
	et_assert(color_parameter_property);

	pv_color_set_parameter(
			&(self->color_pair.colors[self->color_pair_ground]),
			color_parameter_property->ix,
			value);

	et_color_panel_update_focus_elements_();

	et_color_panel_update_ui_();
}

static PvRect _get_pv_rect_of_pallet_from_ground(
		PvColorPairGround ground, int width, int height)
{
	int wofs = width / 10;
	int hofs = height / 10;
	int border = BORDER_PALLET;

	if(PvColorPairGround_ForGround == ground){
		PvRect r = {
			0 + border, 0 + border,
			(width/2)+wofs - border, (height/2)+hofs - border
		};
		return r;
	}else{
		PvRect r = {
			(width/2)-wofs + border, (height/2)-hofs + border,
			(width/2)+wofs - border, (height/2)+hofs - border
		};
		return r;
	}
}

static gboolean cb_button_press_pallet_(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	PvColorPairGround ground_check = !(self->color_pair_ground);
	PvRect rect = _get_pv_rect_of_pallet_from_ground(ground_check, W_PALLET, H_PALLET);
	if(pv_rect_is_inside(rect, (PvPoint){.x = event->x, .y = event->y})){
		self->color_pair_ground = ground_check;
		et_color_panel_update_ui_();
	}

	return FALSE;
}

static gboolean cb_button_release_pallet_(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	return FALSE;
}

static void et_color_panel_set_slider_sliders_from_color_(PvColor color)
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	for(int i = 0; i < pv_color_parameter_property_get_num(); i++){
		const PvColorParameterProperty *color_parameter_property
			= pv_color_get_parameter_property_from_ix(i);
		et_assert(color_parameter_property);

		gtk_range_set_value(GTK_RANGE(self->slider_sliders[i]),
				color.values[color_parameter_property->ix]);
	}
}

static void _et_color_panel_set_slider_spins_from_color(PvColor color)
{
	EtColorPanel *self = color_panel_;
	assert(self);

	for(int i = 0; i < pv_color_parameter_property_get_num(); i++){
		const PvColorParameterProperty *color_parameter_property
			= pv_color_get_parameter_property_from_ix(i);
		assert(color_parameter_property);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(self->slider_spins[i]),
				color.values[color_parameter_property->ix]);
	}
}

static void et_color_panel_update_ui_()
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	et_color_panel_set_slider_sliders_from_color_(
			self->color_pair.colors[self->color_pair_ground]);
	_et_color_panel_set_slider_spins_from_color(
			self->color_pair.colors[self->color_pair_ground]);

	gtk_widget_queue_draw(self->pallet);
}

static void et_color_panel_update_focus_elements_()
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	EtDocId doc_id = et_etaion_get_current_doc_id();
	if(doc_id < 0){
		//! when start app.
		et_debug("doc is nothing. %d", doc_id);
		return;
	}
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assert(focus);

	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
		focus->elements[i]->color_pair.colors[self->color_pair_ground]
			= self->color_pair.colors[self->color_pair_ground];
	}

	et_doc_signal_update_from_id(doc_id);
}

static void draw_multicolor_(cairo_t *cr, PvRect rect)
{
	PvCairoRgbaColor cc = {0.5, 0.5, 0.5, 0.8};
	cairo_set_source_rgba (cr, cc.r, cc.g, cc.b, cc.a);

	int unit = 6;
	for(int y = 0; y < rect.h; y += unit){
		for(int x = 0 + (((y/unit) % 2) * unit); x < rect.w; x += (unit * 2)){
			cairo_rectangle (cr, x + rect.x, y + rect.y, 4, 4);
		}
	}
	cairo_fill (cr);

	PvCairoRgbaColor cc1 = {0.3, 0.3, 0.3, 0.8};
	cairo_set_source_rgba (cr, cc1.r, cc1.g, cc1.b, cc1.a);

	for(int y = 0; y < rect.h; y += unit){
		for(int x = 0 + (((y/unit) % 2) * unit); x < rect.w; x += (unit * 2)){
			cairo_rectangle (cr, 3 + x + rect.x, 3 + y + rect.y, 4, 4);
		}
	}
	cairo_fill (cr);
}

static void draw_pallet_(cairo_t *cr, PvRect rect, PvCairoRgbaColor cc, bool is_multi)
{
	cairo_save (cr);
	cairo_rectangle (cr, rect.x, rect.y, rect.w, rect.h);
	cairo_clip_preserve (cr);

	// erase background
	pv_cairo_fill_checkboard(cr, rect);

	// fill color
	cairo_set_source_rgba (cr, cc.r, cc.g, cc.b, cc.a);
	cairo_rectangle (cr, rect.x, rect.y, rect.w, rect.h);
	cairo_fill(cr);

	// multi color
	if(is_multi){
		draw_multicolor_(cr, rect);
	}else{
		cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1.0);
	}

	// stroke rectangle
	cairo_set_line_width(cr, BORDER_PALLET * 2);
	cairo_rectangle (cr, rect.x, rect.y, rect.w, rect.h);
	cairo_stroke (cr);

	cairo_restore(cr); // clear clipping
}

static gboolean cb_expose_event_pallet_ (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	EtColorPanel *self = color_panel_;
	et_assert(self);

	gtk_widget_set_size_request(self->pallet, W_PALLET + 10, H_PALLET + 10);

	// ** update pallet
	PvColorPairGround ground;
	PvCairoRgbaColor cc;
	PvRect rect;
	// * Ground of Disable(under)
	ground = ! self->color_pair_ground;
	cc = pv_color_get_cairo_rgba(self->color_pair.colors[ground]);
	rect = _get_pv_rect_of_pallet_from_ground(ground, W_PALLET, H_PALLET);

	draw_pallet_(cr, rect, cc, self->is_multi_colors[ground]);

	// * Ground of Enable(front)
	ground = self->color_pair_ground;
	cc = pv_color_get_cairo_rgba(self->color_pair.colors[ground]);
	rect = _get_pv_rect_of_pallet_from_ground(ground, W_PALLET, H_PALLET);

	draw_pallet_(cr, rect, cc, self->is_multi_colors[ground]);

	return FALSE;
}

