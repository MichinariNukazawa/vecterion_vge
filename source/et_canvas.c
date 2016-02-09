#include "et_canvas.h"

#include <stdlib.h>
#include "et_error.h"

struct _EtCanvas{
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

EtCanvas *et_canvas_new()
{
	EtCanvas *this = (EtCanvas *)malloc(sizeof(EtCanvas));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	this->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

	this->box_infobar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(this->box), this->box_infobar);

	this->scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(GTK_WIDGET(this->scroll), TRUE);  
	gtk_widget_set_vexpand(GTK_WIDGET(this->scroll), TRUE);
	gtk_container_add(GTK_CONTAINER(this->box), this->scroll);

	this->text_scale = gtk_text_view_new();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (this->text_scale));
	gtk_text_buffer_set_text (buffer, "default scale", -1);
	gtk_text_view_set_editable (GTK_TEXT_VIEW(this->text_scale), false);
	gtk_container_add(GTK_CONTAINER(this->box_infobar), this->text_scale);

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
			G_CALLBACK(_cb_button_press), (gpointer)this);
	g_signal_connect(this->event_box, "button-release-event",
			G_CALLBACK(_cb_button_release), (gpointer)this);
	g_signal_connect(this->event_box, "scroll-event",
			G_CALLBACK(_cb_button_scroll), (gpointer)this);
	g_signal_connect(this->event_box, "motion-notify-event",
			G_CALLBACK(_cb_motion_notify), (gpointer)this);

	gtk_container_add(GTK_CONTAINER(this->scroll), this->event_box);

	this->canvas = gtk_drawing_area_new();
	if(NULL == this->canvas){
		et_error("");
		return NULL;
	}
	gtk_container_add(GTK_CONTAINER(this->event_box), this->canvas);

	g_signal_connect (G_OBJECT (this->canvas), "draw",
			G_CALLBACK (cb_expose_event), (gpointer)this);

	this->render_context.scale = 1.0;

	this->widget = this->box;
	this->pixbuf_buffer = NULL;
	this->slot_change = NULL;
	this->slot_change_data = NULL;
	this->slot_mouse_action = NULL;
	this->slot_mouse_action_data = NULL;
	this->doc_id = -1;

	return this;
}

GtkWidget *et_canvas_get_widget_frame(EtCanvas *this)
{
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	return this->widget;
}


PvRenderContext et_canvas_get_render_context(EtCanvas *this, bool *isError)
{
	PvRenderContext rc;
	if(NULL == this){
		et_bug("");
		*isError = true;
	}else{
		*isError = false;
		rc = this->render_context;
	}

	return rc;
}

EtDocId et_canvas_get_doc_id(EtCanvas *this)
{
	if(NULL == this){
		et_bug("");
		return -1;
	}

	return this->doc_id;
}
	
bool et_canvas_set_doc_id(EtCanvas *this, EtDocId doc_id)
{
	if(NULL == this){
		et_bug("");
		return false;
	}

	this->doc_id = doc_id;

	return true;
}

void _modifier_kind(int state)
{
	if(state & GDK_SHIFT_MASK){
		printf("Shift\n");
	}
	if(state & GDK_CONTROL_MASK){
		printf("CONTROL\n");
	}
	if(state & GDK_MOD1_MASK){
		printf("Alt\n");
	}
}

void _mouse_button_kind(int button)
{
	switch(button){
		case 1:
			et_debug("LEFT\n");
			break;
		case 2:
			et_debug("CENTER\n");
			break;
		case 3:
			et_debug("RIGHT\n");
			break;
		default:
			et_debug("UNKNOWN:%d\n", button);
	}
}

gboolean _cb_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	EtCanvas *this = (EtCanvas *)data;
	et_debug("BUTTON PRESS: (%4d, %4d)\n", (int)event->x, (int)event->y);
	_mouse_button_kind(event->button);
	_modifier_kind(event->state);

	if(NULL == this->slot_mouse_action){
		et_error("");
	}else{
		EtPoint point = {
			.x = event->x / this->render_context.scale,
			.y = event->y / this->render_context.scale,
		};
		EtMouseAction mouse_action = {
			.button = EtMouseButton_Right,
			.action = EtMouseAction_Down,
			.point = point,
		};
		if(!this->slot_mouse_action(this->doc_id, mouse_action)){
			et_error("");
		}
	}

	return false;
}

gboolean _cb_button_release(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	et_debug("BUTTON RELEASE\n");
	return false;
}

gboolean _cb_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	//	et_debug("(%3d, %3d)\n", (int)event->x, (int)event->y);

	return false;
}

gboolean _cb_button_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
	EtCanvas *this = (EtCanvas *)data;

	switch(event->direction){
		case GDK_SCROLL_UP:
			et_debug("BUTTON SCROLL   UP\n");
			this->render_context.scale += 0.1;
			break;
		case GDK_SCROLL_DOWN:
			et_debug("BUTTON SCROLL DOWN\n");
			this->render_context.scale -= 0.1;
			break;
		default:
			break;
	}

	const int ET_RENDER_CONTEXT_SCALE_MIN = 0.01;
	if(this->render_context.scale < ET_RENDER_CONTEXT_SCALE_MIN){
		et_debug("under limit scale:%f", this->render_context.scale);
		this->render_context.scale = ET_RENDER_CONTEXT_SCALE_MIN;
	}

	char buf[128];
	snprintf(buf, sizeof(buf), "%.3f", this->render_context.scale);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (this->text_scale));
	gtk_text_buffer_set_text (buffer, buf, -1);

	if(NULL != this->slot_change){
		et_debug("CALL canvas:%ld\n", this);
		this->slot_change(this, this->slot_change_data);
	}

	return false;
}


gboolean cb_expose_event (GtkWidget *widget, cairo_t *cr, gpointer data)
{
	EtCanvas *etCanvas = (EtCanvas *)data;

	GdkPixbuf *pixbuf = etCanvas->pixbuf_buffer;

	if(NULL == pixbuf){
		et_warning("");
	}else{

		gtk_widget_set_size_request(
				etCanvas->canvas,
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

bool et_canvas_draw_pixbuf(EtCanvas *this, GdkPixbuf *pixbuf)
{
	if(NULL == this){
		et_bug("\n");
		return false;
	}
	if(NULL == pixbuf){
		et_bug("\n");
		return false;
	}

	GdkPixbuf *pb_old = this->pixbuf_buffer;
	this->pixbuf_buffer = gdk_pixbuf_copy(pixbuf);
	gtk_widget_queue_draw(this->canvas);
	if(NULL != pb_old){
		g_object_unref(G_OBJECT(pb_old));
	}

	return true;
}

int et_canvas_set_slot_change(EtCanvas *this,
		EtCanvasSlotChange slot, gpointer data)
{
	if(NULL != this->slot_change){
		et_error("");
		return -1;
	}

	this->slot_change = slot;
	this->slot_change_data = data;

	return 1; // Todo: callback id
}

int et_canvas_set_slot_mouse_action(EtCanvas *this,
		EtCanvasSlotMouseAction slot, gpointer data)
{
	if(NULL != this->slot_mouse_action){
		et_bug("");
		return -1;
	}

	this->slot_mouse_action = slot;
	this->slot_mouse_action_data = data;

	return 1; // Todo: callback id
}
