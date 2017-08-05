#include "et_layer_view.h"

#include <stdlib.h>
#include <string.h>
#include "et_error.h"
#include "et_doc_manager.h"
#include "pv_element.h"
#include "et_mouse_util.h"
#include "et_etaion.h"


const int ELEMENT_WIDTH = 300;
const int ELEMENT_HEIGHT = 18;
#define ELEMENT_ICON_PX			(18)
#define ELEMENT_HEAD_OFFSET_X		(ELEMENT_ICON_PX * 2)
const int FONT_SIZE = 14;

typedef struct{
	const char * const name;
	const char * const filename_image;
}EtLayerViewLayerCtrl;

const EtLayerViewLayerCtrl et_layer_view_layer_ctrls_buttons_[] = {
	{"New", "layer_new_64x64.svg",},
	{"Child", "layer_new_child_64x64.svg",},
	{"Clone", "layer_clone_64x64.svg",},
	{"Delete", "layer_del_64x64.svg",},
};

typedef struct{
	int level;
	const char *name;
	PvElementKind kind;
	PvElement *element;
}EtLayerViewElementData;

struct EtLayerView{
	GtkWidget *widget; // Top widget pointer.
	GtkWidget *box;
	GtkWidget *scroll;
	GtkWidget *event_box;
	GtkWidget *layer_tree_canvas;
	GtkWidget *box_button_layer_ctrl;
	GtkWidget *button_layer_ctrls[4];

	EtDocId doc_id;
	// buffer
	EtLayerViewElementData **elementDatas;
	GdkPixbuf *pixbuf_layer_tree;
};

typedef struct{
	EtLayerViewElementData **datas;
}EtLayerViewRltDataPack;


static EtLayerView *layer_view_ = NULL;

static gboolean cb_expose_event_layer_tree_canvas_(GtkWidget *widget, cairo_t *cr, gpointer data);

static gboolean cb_button_press_layer_view_content_(
		GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean cb_button_release_layer_view_content_(
		GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean cb_motion_notify_layer_view_content_(
		GtkWidget *widget, GdkEventMotion *event, gpointer data);

static gboolean cb_clicked_layer_ctrl_button_(GtkWidget *widget, gpointer data);

static bool init_layer_ctrl_button_(EtLayerView *self, int index);

EtLayerView *et_layer_view_init()
{
	et_assert(!layer_view_);

	EtLayerView *self = (EtLayerView *)malloc(sizeof(EtLayerView));
	et_assert(self);

	self->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

	self->scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_hexpand(GTK_WIDGET(self->scroll), TRUE);  
	gtk_widget_set_vexpand(GTK_WIDGET(self->scroll), TRUE);
	gtk_container_add(GTK_CONTAINER(self->box), self->scroll);

	self->event_box = gtk_event_box_new();
	et_assert(self->event_box);
	gtk_widget_set_events(self->event_box,
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK
			);
	g_signal_connect(self->event_box, "button-press-event",
			G_CALLBACK(cb_button_press_layer_view_content_), (gpointer)self);
	g_signal_connect(self->event_box, "button-release-event",
			G_CALLBACK(cb_button_release_layer_view_content_), (gpointer)self);
	g_signal_connect(self->event_box, "motion-notify-event",
			G_CALLBACK(cb_motion_notify_layer_view_content_), (gpointer)self);

	gtk_container_add(GTK_CONTAINER(self->scroll), self->event_box);

	self->layer_tree_canvas = gtk_drawing_area_new();
	g_signal_connect (G_OBJECT (self->layer_tree_canvas), "draw",
			G_CALLBACK (cb_expose_event_layer_tree_canvas_), (gpointer)self);
	gtk_container_add(GTK_CONTAINER(self->event_box), self->layer_tree_canvas);

	self->box_button_layer_ctrl = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER(self->box), self->box_button_layer_ctrl);

	et_assert(init_layer_ctrl_button_(self, 0));
	et_assert(init_layer_ctrl_button_(self, 1));
	et_assert(init_layer_ctrl_button_(self, 2));
	et_assert(init_layer_ctrl_button_(self, 3));

	self->widget = self->box;

	self->doc_id = -1;
	self->elementDatas = NULL;
	self->pixbuf_layer_tree = NULL;

	layer_view_ = self;

	return self;
}

GtkWidget *et_layer_view_get_widget_frame()
{
	EtLayerView *self = layer_view_;
	et_assert(self);

	return self->widget;
}

static bool read_layer_tree_(PvElement *element, gpointer data, int level)
{
	EtLayerViewRltDataPack *func_rlt_data_pack = data;
	EtLayerViewElementData ***datas = &(func_rlt_data_pack->datas);

	EtLayerViewElementData *rlt = malloc(sizeof(EtLayerViewElementData));
	et_assert(rlt);

	rlt->level = level;
	rlt->name = "";
	rlt->kind = element->kind;
	rlt->element = element;

	size_t num = pv_general_get_parray_num((void **)*datas);
	*datas = (EtLayerViewElementData **)
		realloc(*datas, sizeof(EtLayerViewElementData *) * (num + 2));
	et_assert(*datas);

	(*datas)[num] = rlt;
	(*datas)[num + 1] = NULL;

	return true;
}

static void set_font_(cairo_t *cr)
{
	cairo_select_font_face (cr, "Consolas",
			CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

	cairo_set_font_size (cr, FONT_SIZE);
}

static GdkPixbuf *gen_empty_()
{
	const int WIDTH = ELEMENT_WIDTH;
	const int HEIGHT = 24;
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH, HEIGHT);
	et_assert(surface);

	cairo_t *cr = cairo_create (surface);
	et_assert(cr);

	set_font_(cr);

	cairo_move_to(cr, 0, FONT_SIZE);
	cairo_show_text(cr, "<no document>");

	return gdk_pixbuf_get_from_surface(surface, 0, 0, WIDTH, HEIGHT);
}

static bool draw_element_tree_(EtLayerView *self, cairo_t *cr);

static gboolean cb_expose_event_layer_tree_canvas_(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	EtLayerView *self = (EtLayerView *)data;

	EtLayerViewElementData **elementDatas = self->elementDatas;
	size_t num = pv_general_get_parray_num((void **)elementDatas);

	if(0 == num){
		self->pixbuf_layer_tree = gen_empty_();
		et_assert(self->pixbuf_layer_tree);

		gtk_widget_set_size_request(
				self->layer_tree_canvas,
				gdk_pixbuf_get_width(self->pixbuf_layer_tree),
				gdk_pixbuf_get_height(self->pixbuf_layer_tree));

		gdk_cairo_set_source_pixbuf(cr, self->pixbuf_layer_tree, 0, 0);
		cairo_paint(cr);

		return FALSE;
	}

	int canvas_width = ELEMENT_WIDTH;
	int canvas_height = num * ELEMENT_HEIGHT;
	gtk_widget_set_size_request(
			self->layer_tree_canvas,
			canvas_width,
			canvas_height);

	draw_element_tree_(self, cr);

	return FALSE;
}

static void draw_invisible_icon_(cairo_t *cr, bool is_invisible, int x, int y)
{

	if(is_invisible){
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.2);
	}else{
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);

		cairo_move_to(cr, x + 3.5, y + 9.5);
		cairo_rel_line_to(cr,  6, -3);
		cairo_rel_line_to(cr,  6,  3);
		cairo_set_line_width(cr, 1);
		cairo_stroke(cr);

		cairo_move_to(cr, x + 3.5 + 6, y + 9.5 - 3);
		cairo_rel_line_to(cr,  0, 6);
		cairo_stroke(cr);
	}
	// ** eye icon
	cairo_move_to(cr, x + 3.5, y + 10.5);
	cairo_rel_line_to(cr,  6,  3);
	cairo_rel_line_to(cr,  6, -3);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);

	// ** frame
	cairo_rectangle(cr, x + 2.5, y + 2.5, ELEMENT_ICON_PX - 4, ELEMENT_ICON_PX - 4);
	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);
}

static void draw_lock_icon_(cairo_t *cr, bool is_locked, int x, int y)
{

	if(is_locked){
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	}else{
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.2);
	}
	// ** keybox icon
	cairo_move_to(cr, x + 6.5, y + 10.5);
	cairo_rel_line_to(cr,  0, -5);
	cairo_rel_line_to(cr,  6, 0);
	cairo_rel_line_to(cr,  0, 5);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);

	cairo_rectangle(cr, x + 5, y + 10, 9, 5);
	cairo_fill(cr);

	// ** frame
	cairo_rectangle(cr, x + 2.5, y + 2.5, ELEMENT_ICON_PX - 4, ELEMENT_ICON_PX - 4);
	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);
}

static bool draw_element_tree_(EtLayerView *self, cairo_t *cr)
{
	et_assert(self);

	EtLayerViewElementData **elementDatas = self->elementDatas;
	size_t num = pv_general_get_parray_num((void **)elementDatas);

	const PvFocus *focus = et_doc_get_focus_ref_from_id(self->doc_id);
	et_assert(focus);

	set_font_(cr);

	for(int i = 0; i < (int)num; i++){
		EtLayerViewElementData *data = elementDatas[i];
		if(0 == i){
			// skip root element.
			if(PvElementKind_Root != data->element->kind){
				et_bug("%d", data->element->kind);
			}
			continue;
		}else{
			if(PvElementKind_Root == data->element->kind){
				et_bug("%d", i);
				continue;
			}
		}
		int offset = 1;
		int index = i - offset;

		// ** clipping
		cairo_save(cr);
		cairo_rectangle(cr, 0, (ELEMENT_HEIGHT * index), ELEMENT_WIDTH, ELEMENT_HEIGHT);
		cairo_clip(cr);

		// ** background
		bool is_fill_background = false;
		if(PvElementKind_Layer != data->element->kind){
			if(pv_focus_is_exist_element(focus, data->element)){
				is_fill_background = true;
				cairo_set_source_rgba (cr,  0.85, 0.95, 1.0, 1.0);
				if(pv_focus_get_first_element(focus) == data->element){
					cairo_set_source_rgba (cr,  0.3, 0.9, 1.0, 1.0);
				}
			}
		}else{
			if(pv_focus_get_first_layer(focus) == data->element){
				is_fill_background = true;
				cairo_set_source_rgba (cr, 0.5, 0.7, 1.0, 1.0);
			}
		}
		if(is_fill_background){
			cairo_rectangle(cr, 0, (ELEMENT_HEIGHT * index), ELEMENT_WIDTH, ELEMENT_HEIGHT);
			cairo_fill(cr);
		}

		// ** element content
		unsigned long debug_pointer = 0;
		memcpy(&debug_pointer, &data->element, sizeof(unsigned long));

		char str_element_head[128];
		str_element_head[0] = '\0';
		for(int t = 0; t < data->level; t++){
			str_element_head[t] = '_';
			str_element_head[t + 1] = '\0';
			if(!(t < ((int)sizeof(str_element_head) - 2))){
				break;
			}
		}

		const char *kind_name = pv_element_get_kind_name(data->element);
		if(PvElementKind_Group == data->element->kind){
			kind_name = pv_element_get_group_name_from_element(data->element);
		}
		if(NULL == kind_name){
			kind_name = "";
		}

		// ** invisible icon
		draw_invisible_icon_(cr, data->element->is_invisible, 0, (ELEMENT_HEIGHT * index));

		// ** lock icon
		draw_lock_icon_(cr, data->element->is_locked, ELEMENT_ICON_PX, (ELEMENT_HEIGHT * index));

		// ** tree, name
		char str_element[128];
		snprintf(str_element, sizeof(str_element),
				"%s%c:%s    :%08lx '%s'",
				str_element_head,
				((pv_focus_get_first_element(focus) == data->element)? '>':'_'),
				//data->level,
				kind_name,
				debug_pointer,
				data->name);

		cairo_text_extents_t te;
		cairo_text_extents (cr, str_element, &te);
		/*if(i < 10){
		  et_debug("w:%.2f, %.2f, %.2f, h:%.2f, %.2f, %.2f, ",
		  te.width, te.x_bearing, te.x_advance,
		  te.height, te.y_bearing, te.y_advance);
		  }*/
		cairo_move_to(cr, ELEMENT_HEAD_OFFSET_X, (ELEMENT_HEIGHT * index));
		cairo_rel_move_to (cr, 0, te.height);
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
		cairo_show_text(cr, str_element);

		// ** border
		cairo_rectangle(cr, 0, (ELEMENT_HEIGHT * index), ELEMENT_WIDTH, ELEMENT_HEIGHT);
		cairo_set_source_rgba (cr, 0.9, 0.9, 0.9, 1.0);
		cairo_set_line_width(cr, 1 * 2);
		cairo_stroke(cr);

		// ** clear clipping
		cairo_restore(cr);
	}

	return true;
}

static bool update_ui_(EtLayerView *self)
{
	et_assert(self);

	gtk_widget_queue_draw(self->layer_tree_canvas);

	// ターゲット状態でlayer_ctrlsのbutton状態を変更する
	{
		PvElement *focus_element = NULL;
		const PvFocus *focus = et_doc_get_focus_ref_from_id(self->doc_id);
		if(NULL != focus){
			focus_element = pv_focus_get_first_element(focus);
		}

		gtk_widget_set_sensitive(self->button_layer_ctrls[0], (0 <= self->doc_id));
		gtk_widget_set_sensitive(self->button_layer_ctrls[1], (NULL != focus_element));
		gtk_widget_set_sensitive(self->button_layer_ctrls[2], (NULL != focus_element));
		gtk_widget_set_sensitive(self->button_layer_ctrls[3], (NULL != focus_element));
	}

	return true;
}

static bool update_doc_tree_()
{
	EtLayerView *self = layer_view_;
	et_assert(self);

	EtLayerViewRltDataPack func_rlt_data_pack = {
		.datas = NULL
	};

	if(self->doc_id < 0){
		// nop
	}else{

		EtDoc *doc = et_doc_manager_get_doc_from_id(self->doc_id);
		if(NULL == doc){
			et_error("");
			return false;
		}

		PvVg *vg = et_doc_get_vg_ref(doc);
		if(NULL == vg){
			et_error("");
			return false;
		}

		PvElementRecursiveError error;
		if(!pv_element_recursive_desc_before(vg->element_root,
					read_layer_tree_, &func_rlt_data_pack,
					&error)){
			et_error("level:%d", error.level);
			return false;
		}
	}

	EtLayerViewElementData **before = self->elementDatas;
	self->elementDatas = func_rlt_data_pack.datas;
	if(NULL != before){
		size_t num = pv_general_get_parray_num((void **)before);
		for(int i = 0; i < (int)num; i++){
			EtLayerViewElementData *data = before[i];
			free(data);
		}
	}

	return update_ui_(self);
}

bool et_layer_view_set_doc_id(EtDocId doc_id)
{
	EtLayerView *self = layer_view_;
	et_assert(self);

	self->doc_id = doc_id;

	if(!update_doc_tree_()){
		et_error("");
		return false;
	}

	return true;
}

void et_layer_view_slot_from_doc_change(EtDoc *doc, gpointer data)
{
	if(!update_doc_tree_()){
		et_error("");
	}
}

void slot_et_layer_view_from_etaion_change_state(EtState state, gpointer data)
{
	EtLayerView *self = layer_view_;
	et_assert(self);

	self->doc_id = state.doc_id;

	if(!et_layer_view_set_doc_id(self->doc_id)){
		et_error("");
		return;
	}
}

static bool init_layer_ctrl_button_(EtLayerView *self, int index)
{
	GError *error = NULL;
	GdkPixbuf *pixbuf = NULL;
	char tmp[1024];
	snprintf(tmp, sizeof(tmp),
			"%s/resource/%s",
			et_etaion_get_application_base_dir(),
			et_layer_view_layer_ctrls_buttons_[index].filename_image);
	pixbuf = gdk_pixbuf_new_from_file(tmp, &error);
	{
		GdkPixbuf *t = pixbuf;
		pixbuf = gdk_pixbuf_scale_simple(
				t,
				32, 32,
				GDK_INTERP_HYPER);
		et_assert(pixbuf);
		g_object_unref(t);
	}
	if(NULL == pixbuf){
		et_error("%d, '%s'", index, tmp);
		return false;
	}
	GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
	if(NULL == image){
		et_error("");
		return false;
	}

	self->button_layer_ctrls[index] = gtk_button_new();
	if(NULL == self->button_layer_ctrls[index]){
		et_error("");
		return false;
	}
	// フォーカス状態でEnterKey押下した際に、誤作動する問題への対処
#if GTK_CHECK_VERSION (3,20,0)
	gtk_widget_set_focus_on_click(self->button_layer_ctrls[index], false);
#else
	gtk_button_set_focus_on_click(GTK_BUTTON(self->button_layer_ctrls[index]), false);
#endif
	// gtk_widget_set_can_focus(self->button_layer_ctrls[index], false);
	gtk_container_add(GTK_CONTAINER(self->button_layer_ctrls[index]),
			image);

	gtk_container_add(GTK_CONTAINER(self->box_button_layer_ctrl),
			self->button_layer_ctrls[index]);

	g_signal_connect(self->button_layer_ctrls[index], "clicked",
			G_CALLBACK(cb_clicked_layer_ctrl_button_), (gpointer)self);

	// デフォルト無効からスタート
	gtk_widget_set_sensitive(self->button_layer_ctrls[index], false);

	return true;
}

int index_data_from_position_(EtLayerView *self, int x, int y)
{
	et_assert(self);

	if(y < 0){
		et_bug("%d", y);
		return -1;
	}

	unsigned int index = (y / ELEMENT_HEIGHT) + 1; // +1 = hidden root layer

	size_t num = pv_general_get_parray_num((void **)self->elementDatas);
	if(!(index < num)){
		return -1;
	}

	return index;
}

static gboolean cb_button_press_layer_view_content_(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	EtLayerView *self = (EtLayerView *)data;
	et_debug("BUTTON PRESS: (%4d, %4d)", (int)event->x, (int)event->y);
	et_mouse_util_button_kind(event->button);
	et_mouse_util_modifier_kind(event->state);

	int index = index_data_from_position_(self, event->x, event->y);
	et_debug("%d", index);
	if(0 <= index){
		PvFocus *focus = et_doc_get_focus_ref_from_id(self->doc_id);
		if(NULL == focus){
			et_error("");
			return false;
		}

		pv_focus_clear_set_element(focus, self->elementDatas[index]->element);

		if((0 < event->x) && (event->x < (ELEMENT_ICON_PX))){
			self->elementDatas[index]->element->is_invisible
				= !self->elementDatas[index]->element->is_invisible;
		}

		if((ELEMENT_ICON_PX < event->x) && (event->x < (ELEMENT_ICON_PX * 2))){
			self->elementDatas[index]->element->is_locked
				= !self->elementDatas[index]->element->is_locked;
		}

		if(self->elementDatas[index]->element->is_locked
				|| self->elementDatas[index]->element->is_invisible ){
			pv_focus_clear_to_first_layer(focus);
		}

		if(!et_doc_signal_update_from_id(self->doc_id)){
			et_error("");
			return false;
		}
	}

	return false;
}

static gboolean cb_button_release_layer_view_content_(
		GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	return false;
}

static gboolean cb_motion_notify_layer_view_content_(
		GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	// et_debug("(%3d, %3d)", (int)event->x, (int)event->y);

	return false;
}

static gboolean cb_clicked_layer_ctrl_button_(
		GtkWidget *widget, gpointer data)
{
	EtLayerView *self = (EtLayerView *)data;
	et_assert(self);

	int index = -1;
	for(int i = 0; i < 4; i++){
		if(widget == self->button_layer_ctrls[i]){
			index = i;
			break;
		}
	}

	switch(index){
		case 0:
			if(!et_etaion_append_new_layer(self->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 1:
			if(!et_etaion_append_new_layer_child(self->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 2:
			if(!et_etaion_copy_layer(self->doc_id)){
				et_error("");
				return false;
			}
			break;
		case 3:
			if(!et_etaion_remove_delete_layer(self->doc_id)){
				et_error("");
				return false;
			}
			break;
		default:
			et_error("");
			return false;
	}

	return false;
}

