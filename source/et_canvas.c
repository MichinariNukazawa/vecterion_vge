#include "et_canvas.h"

#include <stdlib.h>
#include "et_error.h"


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

	this->scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(GTK_WIDGET(this->scroll), TRUE);  
	gtk_widget_set_vexpand(GTK_WIDGET(this->scroll), TRUE);

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

	g_signal_connect(this->event_box, "scroll-event",
			G_CALLBACK(_cb_button_scroll), (gpointer)this);
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

	this->widget = this->scroll;
	this->pixbuf_buffer = NULL;
	this->cb_update = NULL;
	this->cb_update_data = NULL;

	return this;
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
	et_debug("BUTTON PRESS: (%3d, %3d)\n", (int)event->x, (int)event->y);
	et_debug("%ld\n", (long)widget);
	_mouse_button_kind(event->button);
	_modifier_kind(event->state);

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

	if(NULL != this->cb_update){
		et_debug("CALL canvas:%ld\n", this);
		this->cb_update(this, this->cb_update_data);
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
	if(NULL == pixbuf){
		et_error("\n");
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

int et_canvas_set_update_render_context(EtCanvas *this, EtCanvasUpdateCallback func, gpointer data)
{
	if(NULL != this->cb_update){
		et_error("");
		return -1;
	}
	this->cb_update = func;
	this->cb_update_data = data;

	return 1; // Todo: callback id
}
