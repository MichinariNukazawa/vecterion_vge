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
#include "pv_type.h"
#include "et_doc.h"

struct EtCanvas{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;
	GtkWidget *box_infobar;
	GtkWidget *text_scale;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *cussion_v;
	GtkWidget *cussion_h;
	GtkWidget *canvas;

	PvRenderContext render_context;

	EtDocId doc_id;
	GdkPixbuf *pixbuf_buffer;

	bool is_thumbnail;

	bool is_first_fitting;
	bool is_fitting_scale;

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

EtCanvas *et_canvas_new_from_doc_id(EtDocId doc_id)
{
	EtCanvas *self = (EtCanvas *)malloc(sizeof(EtCanvas));
	if(NULL == self){
		et_error("");
		return NULL;
	}

	self->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

	self->scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(GTK_WIDGET(self->scroll), TRUE);  
	gtk_widget_set_vexpand(GTK_WIDGET(self->scroll), TRUE);
	gtk_container_add(GTK_CONTAINER(self->box), self->scroll);

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
	//gtk_container_add(GTK_CONTAINER(self->event_box), self->canvas);

	g_signal_connect (G_OBJECT (self->canvas), "draw",
			G_CALLBACK (_cb_expose_event), (gpointer)self);

	self->render_context = PvRenderContext_Default;

	self->widget = self->box;
	self->pixbuf_buffer = NULL;
	self->slot_change = NULL;
	self->slot_change_data = NULL;
	self->slot_mouse_action = NULL;
	self->slot_mouse_action_data = NULL;
	self->doc_id = doc_id;

	self->is_thumbnail = false;

	self->is_first_fitting = true;
	self->is_fitting_scale = false;

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
		PvPoint dp,
		PvPoint dp_move,
		PvPoint cwp_diff_down,
		GdkModifierType state,
		EtMouseButtonType mouse_button, EtMouseActionType mouse_action)
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

	EtMouseAction _mouse_action = {
		.button = mouse_button,
		.action = mouse_action,
		.raw = dp,
		.point = dp,
		.move = dp_move,
		.diff_down = cwp_diff_down,
		.state = state,
	};
	if(!self->slot_mouse_action(self->doc_id, _mouse_action)){
		et_error("");
		return false;
	}

	return true;
}


PvPoint _et_canvas_dp_from_cwp(PvPoint cwp, PvRenderContext render_context)
{
	PvPoint cp = pv_point_add_value(cwp, -1 * render_context.margin);
	PvPoint dp = pv_point_div_value(cp, render_context.scale);

	return dp;
}

PvPoint _et_canvas_previous_mouse_point = {0,0};
PvPoint _et_canvas_down_mouse_point = {0,0};
static gboolean _cb_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	/*
	   et_debug("BUTTON PRESS: (%4d, %4d)", (int)event->x, (int)event->y);
	   et_mouse_util_button_kind(event->button);
	   et_mouse_util_modifier_kind(event->state);
	 */
	EtCanvas *self = (EtCanvas *)data;

	PvPoint ep = {.x = event->x, .y = event->y};
	_et_canvas_previous_mouse_point = ep;
	_et_canvas_down_mouse_point = ep;
	PvPoint cwp = ep;
	PvPoint dp = _et_canvas_dp_from_cwp(cwp, self->render_context);

	PvPoint dp_move = {.x = 0, .y = 0};
	PvPoint cwp_diff_down = {.x = 0, .y = 0};
	if(!_signal_et_canvas_mouse_action(
				self,
				dp,
				dp_move,
				cwp_diff_down,
				event->state,
				EtMouseButton_Right, EtMouseAction_Down))
	{
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
	PvPoint cwp = ep;
	PvPoint dp = _et_canvas_dp_from_cwp(cwp, self->render_context);

	PvPoint cwp_prev =_et_canvas_previous_mouse_point;
	PvPoint dp_prev = _et_canvas_dp_from_cwp(cwp_prev, self->render_context); 
	PvPoint dp_move = pv_point_sub(dp, dp_prev);

	_et_canvas_previous_mouse_point = ep;

	PvPoint ep_diff_down = pv_point_sub(ep, _et_canvas_down_mouse_point);
	PvPoint cwp_diff_down = ep_diff_down;

	if(!_signal_et_canvas_mouse_action(
				self,
				dp,
				dp_move,
				cwp_diff_down,
				event->state,
				EtMouseButton_Right, EtMouseAction_Up))
	{
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
	PvPoint cwp = ep;
	PvPoint dp = _et_canvas_dp_from_cwp(cwp, self->render_context);

	PvPoint cwp_prev =_et_canvas_previous_mouse_point;
	PvPoint dp_prev = _et_canvas_dp_from_cwp(cwp_prev, self->render_context); 
	PvPoint dp_move = pv_point_sub(dp, dp_prev);

	_et_canvas_previous_mouse_point = ep;

	PvPoint ep_diff_down = pv_point_sub(ep, _et_canvas_down_mouse_point);
	PvPoint cwp_diff_down = ep_diff_down;

	if(!_signal_et_canvas_mouse_action(
				self,
				dp,
				dp_move,
				cwp_diff_down,
				event->state,
				EtMouseButton_Right, EtMouseAction_Move))
	{
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

const double ET_RENDER_CONTEXT_SCALE_MIN = 0.01;
const double ET_CANVAS_SCALE_UNIT = (0.1);
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
				self->render_context.scale = _et_ceil_unit(self->render_context.scale, ET_CANVAS_SCALE_UNIT);
				self->render_context.scale += ET_CANVAS_SCALE_UNIT; 
				break;
			case GDK_SCROLL_DOWN:
				et_debug("BUTTON SCROLL DOWN");
				self->render_context.scale = _et_ceil_unit(self->render_context.scale, ET_CANVAS_SCALE_UNIT);
				self->render_context.scale -= ET_CANVAS_SCALE_UNIT; 
				break;
			default:
				break;
		}

		if(self->render_context.scale < ET_RENDER_CONTEXT_SCALE_MIN){
			et_debug("under limit scale:%f", self->render_context.scale);
			self->render_context.scale = ET_RENDER_CONTEXT_SCALE_MIN;
		}
	}else{
		// ** move(vertical,horizontal) has GtkScrolledWindow.
	}

	_signal_et_canvas_slot_change(self);

	return is_stop_signal;
}


static gboolean _cb_expose_event (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	EtCanvas *self = (EtCanvas *)data;

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
	}

	return FALSE;
}

bool et_canvas_draw_pixbuf(EtCanvas *self, GdkPixbuf *pixbuf)
{
	if(NULL == self){
		et_bug("\n");
		return false;
	}
	if(NULL == pixbuf){
		et_bug("\n");
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

