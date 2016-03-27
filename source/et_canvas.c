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
	GtkWidget *canvas;

	PvRenderContext render_context;

	EtDocId doc_id;
	GdkPixbuf *pixbuf_buffer;

	bool is_first_fitting;

	EtCanvasSlotChange slot_change;
	gpointer slot_change_data;
	EtCanvasSlotMouseAction slot_mouse_action;
	gpointer slot_mouse_action_data;
};

gboolean cb_expose_event (GtkWidget *widget, cairo_t *cr, gpointer data);

gboolean _cb_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean _cb_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean _cb_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data);
gboolean _cb_button_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data);

bool _signal_et_canvas_slot_change(EtCanvas *self)
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

gboolean _cb_size_allocate_event_box(
		GtkWidget    *widget,
		GdkRectangle *allocation,
		gpointer      data)
{
	et_debug("%p: %d,%d", widget, allocation->width, allocation->height);

	EtCanvas *self = (EtCanvas *)data;

	// ** first drawing to cancel and rescaleing.
	//	bool is_fitting_scale_from_widget = true;
	if(self->is_first_fitting && 0 <= self->doc_id){
		self->is_first_fitting = false;

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

	g_signal_connect(self->event_box, "size-allocate",
			G_CALLBACK(_cb_size_allocate_event_box), (gpointer)self);

	gtk_container_add(GTK_CONTAINER(self->scroll), self->event_box);

	self->canvas = gtk_drawing_area_new();
	if(NULL == self->canvas){
		et_error("");
		return NULL;
	}
	gtk_container_add(GTK_CONTAINER(self->event_box), self->canvas);

	g_signal_connect (G_OBJECT (self->canvas), "draw",
			G_CALLBACK (cb_expose_event), (gpointer)self);

	self->render_context = PvRenderContext_default;

	self->widget = self->box;
	self->pixbuf_buffer = NULL;
	self->slot_change = NULL;
	self->slot_change_data = NULL;
	self->slot_mouse_action = NULL;
	self->slot_mouse_action_data = NULL;
	self->doc_id = doc_id;

	self->is_first_fitting = true;

	return self;
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
	PvRenderContext rc = PvRenderContext_default;
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

bool _signal_et_canvas_mouse_action(
		EtCanvas *self,
		double x, double y,
		double x_move, double y_move,
		GdkModifierType state,
		EtMouseButtonType mouse_button, EtMouseActionType mouse_action)
{
	if(NULL == self->slot_mouse_action){
		et_error("");
		return false;
	}

	PvPoint point = {
		.x = x,
		.y = y,
	};
	PvPoint move = {
		.x = x_move,
		.y = y_move,
	};
	EtMouseAction _mouse_action = {
		.button = mouse_button,
		.action = mouse_action,
		.point = point,
		.move = move,
		.state = state,
	};
	if(!self->slot_mouse_action(self->doc_id, _mouse_action)){
		et_error("");
		return false;
	}

	return true;
}

PvPoint _et_canvas_previous_mouse_point = {0,0};
gboolean _cb_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	/*
	   et_debug("BUTTON PRESS: (%4d, %4d)", (int)event->x, (int)event->y);
	   et_mouse_util_button_kind(event->button);
	   et_mouse_util_modifier_kind(event->state);
	 */
	EtCanvas *self = (EtCanvas *)data;

	PvPoint move = {.x = 0, .y = 0};
	_et_canvas_previous_mouse_point.x = event->x;
	_et_canvas_previous_mouse_point.y = event->y;

	if(!_signal_et_canvas_mouse_action(
				self,
				event->x / self->render_context.scale,
				event->y / self->render_context.scale,
				move.x / self->render_context.scale,
				move.y / self->render_context.scale,
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

gboolean _cb_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// et_debug("BUTTON RELEASE");
	EtCanvas *self = (EtCanvas *)data;

	PvPoint move = {
		.x = event->x - _et_canvas_previous_mouse_point.x,
		.y = event->y - _et_canvas_previous_mouse_point.y,
	};
	_et_canvas_previous_mouse_point.x = event->x;
	_et_canvas_previous_mouse_point.y = event->y;


	if(!_signal_et_canvas_mouse_action(
				self,
				event->x / self->render_context.scale,
				event->y / self->render_context.scale,
				move.x / self->render_context.scale,
				move.y / self->render_context.scale,
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

gboolean _cb_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	//	et_debug("(%3d, %3d)", (int)event->x, (int)event->y);
	EtCanvas *self = (EtCanvas *)data;

	PvPoint move = {
		.x = event->x - _et_canvas_previous_mouse_point.x,
		.y = event->y - _et_canvas_previous_mouse_point.y,
	};
	_et_canvas_previous_mouse_point.x = event->x;
	_et_canvas_previous_mouse_point.y = event->y;

	if(!_signal_et_canvas_mouse_action(
				self,
				event->x / self->render_context.scale,
				event->y / self->render_context.scale,
				move.x / self->render_context.scale,
				move.y / self->render_context.scale,
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

double _et_ceil_unit(double value, double unit)
{
	double t = (unit / 10);
	return unit * ((int)((value / unit) + t)); // trick for value 0.6 can't ceiled.
}

const double ET_RENDER_CONTEXT_SCALE_MIN = 0.01;
const double ET_CANVAS_SCALE_UNIT = (0.1);
gboolean _cb_button_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
	EtCanvas *self = (EtCanvas *)data;

	// ** scale change.
	if(0 != (ET_GDK_ALT_MASK & event->state)){

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

	return false;
}


gboolean cb_expose_event (GtkWidget *widget, cairo_t *cr, gpointer data)
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
