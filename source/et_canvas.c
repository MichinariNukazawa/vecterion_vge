/**
 * @brief
 *
 * PvPoint huangarian
 * ep:	event point // GtkEvent position
 * cwp:	canvas widget point // Canvas Widget position
 * cp:	canvas point // canvas position (sub canvas margin)
 * dp:	document point document. scale pixel by pixel. // notice: not "display point"
 *
 */

#include "et_canvas.h"

#include <stdlib.h>
#include <math.h>
#include "et_error.h"
#include "et_define.h"
#include "et_mouse_util.h"
#include "et_pointing_util.h"
#include "pv_type.h"
#include "et_doc.h"

struct EtCanvas{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;
	GtkWidget *grid;
	GtkWidget *ruler_v;
	GtkWidget *ruler_h;
	GtkWidget *box_infobar;
	GtkWidget *text_scale;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *cussion_v;
	GtkWidget *cussion_h;
	GtkWidget *canvas;

	PvRenderContext render_context;

	PvPoint pointing_context_previous_mouse_point;
	PvPoint pointing_context_down_mouse_point;

	EtDocId doc_id;
	GdkPixbuf *pixbuf_buffer;

	bool is_thumbnail;

	bool is_first_fitting;
	bool is_fitting_scale;
	int centering_control;

	EtCanvasSlotChange slot_change;
	gpointer slot_change_data;
	EtCanvasSlotMouseAction slot_mouse_action;
	gpointer slot_mouse_action_data;
};

static gboolean _cb_expose_event (GtkWidget *widget, cairo_t *cr, gpointer data);

static gboolean _cb_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean _cb_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean _cb_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data);
static gboolean _cb_button_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data);

static gboolean _cb_size_allocate_scrolled(GtkWidget *widget, GdkRectangle *allocation, gpointer data);

static bool _signal_et_canvas_slot_change(EtCanvas *self)
{
	if(NULL == self){
		return false;
	}

	if(NULL == self->slot_change){
		et_warning("");
	}else{
		self->slot_change(self, self->slot_change_data);
	}

	return true;
}

static int _max(double a, double b)
{
	return (int)((a > b)? a:b);
}

static int _et_canvas_get_margin_from_doc_id(EtDocId doc_id)
{
	const PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		et_bug("%d", doc_id);
		return 0;
	}
	int px_majoraxis = _max(vg->rect.w, vg->rect.h);
	int margin = ((px_majoraxis / 200) + 1) * 20;

	return margin;
}

static gboolean _cb_size_allocate_scrolled(
		GtkWidget    *widget,
		GdkRectangle *allocation,
		gpointer      data)
{
	et_debug("%p: %d,%d", widget, allocation->width, allocation->height);

	EtCanvas *self = (EtCanvas *)data;

	// ** first drawing to cancel and rescaleing.
	//	bool is_fitting_scale_from_widget = true;
	if((self->is_first_fitting || self->is_fitting_scale) && 0 <= self->doc_id){
		self->is_first_fitting = false;

		self->render_context.margin = _et_canvas_get_margin_from_doc_id(self->doc_id);

		const PvVg *vg = et_doc_get_vg_ref_from_id(self->doc_id);
		const double w_doc = vg->rect.w;
		const double h_doc = vg->rect.h;
		if(w_doc < allocation->width && h_doc < allocation->height){
			self->render_context.scale = 1.0;
		}else{
			double w_scale = allocation->width / w_doc;
			double h_scale = allocation->height / h_doc;
			// trunc digit
			w_scale = trunc(w_scale * 100.0) / 100.0;
			h_scale = trunc(h_scale * 100.0) / 100.0;
			self->render_context.scale = fmin(w_scale, h_scale);
		}
		_signal_et_canvas_slot_change(self);
	}

	return false;
}

void cb_v_adj_value_changed_(
		GtkAdjustment *adjustment,
		gpointer       user_data)
{
	EtCanvas *self = (EtCanvas *)user_data;

	double value = gtk_adjustment_get_value(adjustment);
	double size = gtk_adjustment_get_page_size(adjustment);

	double margin = _et_canvas_get_margin_from_doc_id(self->doc_id);
	value -= margin;

	value /= self->render_context.scale;
	size /= self->render_context.scale;

	gchar *str = g_strdup_printf("%6.1f -- %6.1f", value, value + size);
	gtk_label_set_text(GTK_LABEL(self->ruler_v), str);
	g_free(str);
}

void cb_h_adj_value_changed_(
		GtkAdjustment *adjustment,
		gpointer       user_data)
{
	EtCanvas *self = (EtCanvas *)user_data;

	double value = gtk_adjustment_get_value(adjustment);
	double size = gtk_adjustment_get_page_size(adjustment);

	double margin = _et_canvas_get_margin_from_doc_id(self->doc_id);
	value -= margin;

	value /= self->render_context.scale;
	size /= self->render_context.scale;

	gchar *str = g_strdup_printf("%6.1f -- %6.1f", value, value + size);
	gtk_label_set_text(GTK_LABEL(self->ruler_h), str);
	g_free(str);
}

EtCanvas *et_canvas_new_from_doc_id(EtDocId doc_id)
{
	EtCanvas *self = (EtCanvas *)malloc(sizeof(EtCanvas));
	if(NULL == self){
		et_error("");
		return NULL;
	}

	self->doc_id = doc_id;
	self->render_context = PvRenderContext_Default;
	self->render_context.is_frame_line = true;
	self->pixbuf_buffer = NULL;
	self->slot_change = NULL;
	self->slot_change_data = NULL;
	self->slot_mouse_action = NULL;
	self->slot_mouse_action_data = NULL;
	self->is_thumbnail = false;
	self->is_first_fitting = true;
	self->is_fitting_scale = false;
	self->centering_control = 0;

	self->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

	self->grid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(self->box), self->grid);

	self->ruler_v = gtk_label_new_with_mnemonic("--");
	gtk_grid_attach(GTK_GRID(self->grid), self->ruler_v, 0, 1, 1, 2);
	gtk_label_set_angle(GTK_LABEL(self->ruler_v), 90);
	gtk_style_context_add_class(gtk_widget_get_style_context(self->ruler_v), "ruler");

	self->ruler_h = gtk_label_new_with_mnemonic("--");
	gtk_grid_attach(GTK_GRID(self->grid), self->ruler_h, 1, 0, 2, 1);
	gtk_style_context_add_class(gtk_widget_get_style_context(self->ruler_h), "ruler");

	self->scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(GTK_WIDGET(self->scroll), TRUE);
	gtk_widget_set_vexpand(GTK_WIDGET(self->scroll), TRUE);
	gtk_grid_attach(GTK_GRID(self->grid), self->scroll, 1, 1, 2, 2);

	self->box_infobar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_container_add(GTK_CONTAINER(self->box), self->box_infobar);

	self->text_scale = gtk_text_view_new();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->text_scale));
	gtk_text_buffer_set_text (buffer, "default scale", -1);
	gtk_text_view_set_editable (GTK_TEXT_VIEW(self->text_scale), false);
	gtk_container_add(GTK_CONTAINER(self->box_infobar), self->text_scale);

	self->event_box = gtk_event_box_new();
	if(NULL == self->event_box){
		et_error("");
		return NULL;
	}
	gtk_widget_set_events(self->event_box,
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK
			);

	g_signal_connect(self->event_box, "button-press-event",
			G_CALLBACK(_cb_button_press), (gpointer)self);
	g_signal_connect(self->event_box, "button-release-event",
			G_CALLBACK(_cb_button_release), (gpointer)self);
	g_signal_connect(self->event_box, "scroll-event",
			G_CALLBACK(_cb_button_scroll), (gpointer)self);
	g_signal_connect(self->event_box, "motion-notify-event",
			G_CALLBACK(_cb_motion_notify), (gpointer)self);

	g_signal_connect(self->scroll, "size-allocate",
			G_CALLBACK(_cb_size_allocate_scrolled), (gpointer)self);
	GtkAdjustment *h_adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(self->scroll));
	g_signal_connect(h_adj, "value-changed",
			G_CALLBACK(cb_h_adj_value_changed_), (gpointer)self);
	g_signal_connect(h_adj, "changed",
			G_CALLBACK(cb_h_adj_value_changed_), (gpointer)self);
	GtkAdjustment *v_adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(self->scroll));
	g_signal_connect(v_adj, "value-changed",
			G_CALLBACK(cb_v_adj_value_changed_), (gpointer)self);
	g_signal_connect(v_adj, "changed",
			G_CALLBACK(cb_v_adj_value_changed_), (gpointer)self);

	gtk_container_add(GTK_CONTAINER(self->scroll), self->event_box);

	self->cussion_v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	self->cussion_h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	self->canvas = gtk_drawing_area_new();
	if(NULL == self->canvas){
		et_error("");
		return NULL;
	}
	gtk_container_add(GTK_CONTAINER(self->event_box), self->cussion_h);
	gtk_container_add(GTK_CONTAINER(self->cussion_h), self->cussion_v);
	gtk_container_add(GTK_CONTAINER(self->cussion_v), self->canvas);

	g_signal_connect (G_OBJECT (self->canvas), "draw",
			G_CALLBACK (_cb_expose_event), (gpointer)self);

	gtk_style_context_add_class(gtk_widget_get_style_context(self->grid), "canvas_widget");
	gtk_style_context_add_class(gtk_widget_get_style_context(self->scroll), "canvas_widget");
	gtk_style_context_add_class(gtk_widget_get_style_context(self->canvas), "canvas_widget");
	gtk_style_context_add_class(gtk_widget_get_style_context(self->event_box), "canvas_widget");
	gtk_style_context_add_class(gtk_widget_get_style_context(self->cussion_h), "canvas_widget");
	gtk_style_context_add_class(gtk_widget_get_style_context(self->cussion_v), "canvas_widget");

	self->widget = self->box;

	return self;
}

void et_canvas_delete(EtCanvas *self)
{
	et_assert(self);

	if(NULL != self->pixbuf_buffer){
		g_object_unref(self->pixbuf_buffer);
	}

	free(self);
}

GtkWidget *et_canvas_get_widget_frame(EtCanvas *self)
{
	if(NULL == self){
		et_bug("");
		return NULL;
	}

	return self->widget;
}


PvRenderContext et_canvas_get_render_context(EtCanvas *self, bool *isError)
{
	PvRenderContext rc = PvRenderContext_Default;
	if(NULL == self){
		et_bug("");
		*isError = true;
	}else{
		*isError = false;
		rc = self->render_context;
	}

	return rc;
}

EtDocId et_canvas_get_doc_id(EtCanvas *self)
{
	if(NULL == self){
		et_bug("");
		return -1;
	}

	return self->doc_id;
}

void slot_et_canvas_from_doc_change(EtDoc *doc, gpointer data)
{
	EtCanvas *self = (EtCanvas *)data;
	if(NULL == self){
		et_bug("");
		return;
	}

	_signal_et_canvas_slot_change(self);
}

bool et_canvas_set_doc_id(EtCanvas *self, EtDocId doc_id)
{
	if(NULL == self){
		et_bug("");
		return false;
	}

	self->doc_id = doc_id;

	return true;
}

static bool _signal_et_canvas_mouse_action(
		EtCanvas *self,
		EtMouseAction mouse_action)
{
	if(NULL == self->slot_mouse_action){
		et_error("");
		return false;
	}

	if(self->doc_id < 0){
		if(! self->is_thumbnail){
			et_error("");
			return false;
		}else{
			return true;
		}
	}

	if(!self->slot_mouse_action(self->doc_id, mouse_action)){
		et_error("");
		return false;
	}

	return true;
}


static gboolean _cb_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	/*
	   et_debug("BUTTON PRESS: (%4d, %4d)", (int)event->x, (int)event->y);
	   et_mouse_util_button_kind(event->button);
	   et_mouse_util_modifier_kind(event->state);
	 */
	EtCanvas *self = (EtCanvas *)data;

	PvPoint ep = {.x = event->x, .y = event->y};
	EtMouseAction mouse_action = et_pointing_util_get_mouse_action(
			&(self->pointing_context_previous_mouse_point),
			&(self->pointing_context_down_mouse_point),
			ep,
			event->state,
			self->render_context.margin,
			self->render_context.scale,
			EtMouseButton_Right,
			EtMouseAction_Down);

	if(!_signal_et_canvas_mouse_action(self, mouse_action)){
		et_error("");
		goto error;
	}

	return false;
error:
	return true;
}

static gboolean _cb_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// et_debug("BUTTON RELEASE");
	EtCanvas *self = (EtCanvas *)data;

	PvPoint ep = {.x = event->x, .y = event->y};
	EtMouseAction mouse_action = et_pointing_util_get_mouse_action(
			&(self->pointing_context_previous_mouse_point),
			&(self->pointing_context_down_mouse_point),
			ep,
			event->state,
			self->render_context.margin,
			self->render_context.scale,
			EtMouseButton_Right,
			EtMouseAction_Up);

	if(!_signal_et_canvas_mouse_action(self, mouse_action)){
		et_error("");
		goto error;
	}
	return false;
error:
	return true;
}

static gboolean _cb_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	//	et_debug("(%3d, %3d)", (int)event->x, (int)event->y);
	EtCanvas *self = (EtCanvas *)data;

	PvPoint ep = {.x = event->x, .y = event->y};
	EtMouseAction mouse_action = et_pointing_util_get_mouse_action(
			&(self->pointing_context_previous_mouse_point),
			&(self->pointing_context_down_mouse_point),
			ep,
			event->state,
			self->render_context.margin,
			self->render_context.scale,
			EtMouseButton_Right,
			EtMouseAction_Move);


	if(!_signal_et_canvas_mouse_action(self, mouse_action)){
		et_error("");
		goto error;
	}
	return false;
error:
	return true;
}

static double _et_ceil_unit(double value, double unit)
{
	double t = (unit / 10);
	return unit * ((int)((value / unit) + t)); // trick for value 0.6 can't ceiled.
}

#define ET_CANVAS_SCALE_UNIT (0.1)
const double ET_RENDER_CONTEXT_SCALE_MIN_OF_UNIT = ET_CANVAS_SCALE_UNIT;
const double ET_RENDER_CONTEXT_SCALE_MAX_OF_UNIT = 3.0;
void et_canvas_change_scale_of_unit(EtCanvas *self, int wait)
{
	et_assert(self);

	double before = self->render_context.scale;
	self->render_context.scale = _et_ceil_unit(self->render_context.scale, ET_CANVAS_SCALE_UNIT);
	self->render_context.scale += (wait * ET_CANVAS_SCALE_UNIT);

	if(self->render_context.scale < ET_RENDER_CONTEXT_SCALE_MIN_OF_UNIT){
		et_debug("limit scale min:%f", self->render_context.scale);
		self->render_context.scale = ET_RENDER_CONTEXT_SCALE_MIN_OF_UNIT;
	}

	if(ET_RENDER_CONTEXT_SCALE_MAX_OF_UNIT < self->render_context.scale){
		et_debug("limit scale max:%f", self->render_context.scale);
		self->render_context.scale = ET_RENDER_CONTEXT_SCALE_MAX_OF_UNIT;
	}

	if(before != self->render_context.scale){
		_signal_et_canvas_slot_change(self);
	}
}

static gboolean _cb_button_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
	EtCanvas *self = (EtCanvas *)data;

	gboolean is_stop_signal = FALSE; // シグナル続行、デフォルトコールバック呼び出し

	if(0 != (ET_GDK_ALT_MASK & event->state)){
		// ** scale change.
		is_stop_signal = TRUE; // cancel scroll up/down (default scrolled callback).

		if(self->is_fitting_scale){
			return FALSE; // cancel rescale (is not error.)
		}

		switch(event->direction){
			case GDK_SCROLL_UP:
				et_debug("BUTTON SCROLL   UP");
				et_canvas_change_scale_of_unit(self, 1);
				break;
			case GDK_SCROLL_DOWN:
				et_debug("BUTTON SCROLL DOWN");
				et_canvas_change_scale_of_unit(self, -1);
				break;
			default:
				break;
		}
	}else{
		// ** move(vertical,horizontal) has GtkScrolledWindow.
	}

	return is_stop_signal;
}

static void et_scrolled_window_centering_(GtkScrolledWindow *scrolled_window)
{
	{
		GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment (scrolled_window);
		double v = (gtk_adjustment_get_upper(vadj) - gtk_adjustment_get_page_size(vadj)) / 2.0;
		gtk_adjustment_set_value (vadj, v);
	}

	{
		GtkAdjustment *hadj = gtk_scrolled_window_get_hadjustment (scrolled_window);
		double h = (gtk_adjustment_get_upper(hadj) - gtk_adjustment_get_page_size(hadj)) / 2.0;
		gtk_adjustment_set_value (hadj, h);
	}
}

static void debug_print_adjustment_(EtCanvas *self)
{
	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(self->scroll));

	et_debug("lower:%.1f upper:%.1f page-size:%.1f value:%.1f "
			"width:%d height:%d "
			,
			gtk_adjustment_get_lower(adj),
			gtk_adjustment_get_upper(adj),
			gtk_adjustment_get_page_size(adj),
			gtk_adjustment_get_value(adj),
			gdk_pixbuf_get_width(self->pixbuf_buffer),
			gdk_pixbuf_get_height(self->pixbuf_buffer)
		);
}

static gboolean _cb_expose_event (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	EtCanvas *self = (EtCanvas *)data;

	if(self->is_thumbnail){
		//! not good timing...
		gtk_widget_hide(self->ruler_v);
		gtk_widget_hide(self->ruler_h);
	}

	// ** scale
	char buf[128];
	snprintf(buf, sizeof(buf), "%.3f", self->render_context.scale);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->text_scale));
	gtk_text_buffer_set_text (buffer, buf, -1);

	// ** canvas
	GdkPixbuf *pixbuf = self->pixbuf_buffer;
	if(NULL == pixbuf){
		et_warning("");
	}else{

		gtk_widget_set_size_request(
				self->canvas,
				gdk_pixbuf_get_width(pixbuf),
				gdk_pixbuf_get_height(pixbuf));

		// Draw pixbuf
		/*
		   cairo_t *cr;
		   cr = gdk_cairo_create (gtk_widget_get_window(widget));
		 */
		//    cr = gdk_cairo_create (da->window);
		gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
		//				self->render_context.margin, self->render_context.margin);
		cairo_paint(cr);
		//    cairo_fill (cr);
		//		cairo_destroy (cr);

		switch(self->centering_control){
			case 0:
				self->centering_control++;
				// kick to re draw event, because only need centering.
				gtk_widget_queue_draw(self->canvas);
				break;
			case 1:
				self->centering_control++;
				debug_print_adjustment_(self);
				et_scrolled_window_centering_(GTK_SCROLLED_WINDOW(self->scroll));
				break;
			default:
				break;
		}
	}

	return FALSE;
}

bool et_canvas_draw_pixbuf(EtCanvas *self, GdkPixbuf *pixbuf)
{
	if(NULL == self){
		et_bug("");
		return false;
	}
	if(NULL == pixbuf){
		et_bug("");
		return false;
	}

	GdkPixbuf *pb_old = self->pixbuf_buffer;
	self->pixbuf_buffer = gdk_pixbuf_copy(pixbuf);
	gtk_widget_queue_draw(self->canvas);
	if(NULL != pb_old){
		g_object_unref(G_OBJECT(pb_old));
	}


	return true;
}

int et_canvas_set_slot_change(EtCanvas *self,
		EtCanvasSlotChange slot, gpointer data)
{
	if(NULL != self->slot_change){
		et_error("");
		return -1;
	}

	self->slot_change = slot;
	self->slot_change_data = data;

	return 1; // Todo: callback id
}

int et_canvas_set_slot_mouse_action(EtCanvas *self,
		EtCanvasSlotMouseAction slot, gpointer data)
{
	if(NULL != self->slot_mouse_action){
		et_bug("");
		return -1;
	}

	self->slot_mouse_action = slot;
	self->slot_mouse_action_data = data;

	return 1; // Todo: callback id
}

void et_canvas_set_is_thumbnail(EtCanvas *self, bool is_thumbnail)
{
	et_assert(self);

	self->is_thumbnail = is_thumbnail;
	self->is_fitting_scale = is_thumbnail;
	gtk_widget_set_sensitive(self->text_scale, !is_thumbnail);
}

