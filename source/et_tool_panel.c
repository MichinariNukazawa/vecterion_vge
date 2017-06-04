#include "et_tool_panel.h"

#include <stdlib.h>
#include "et_error.h"
#include "pv_element_general.h"
#include "et_etaion.h"
#include "et_tool_info.h"
#include "et_mouse_cursor.h"

static EtToolPanel *_et_tool_panel = NULL;

typedef struct EtToolPanelItem{
	EtToolId tool_id;
	GtkWidget *button;
}EtToolPanelItem;


struct EtToolPanel{
	GtkWidget *widget;
	GtkWidget *box;
	GtkWidget *toolpanel;

	EtToolPanelItem **tools;

	EtToolPanelSlotChange slot_change;
	gpointer slot_change_data;
};

static void _cb_et_tool_panel_clicked_button(GtkWidget *toggle_button, gpointer data);
static bool _et_tool_panel_set_current_tool_id(EtToolPanel *self, EtToolId tool_id);
static bool _change_cursor_icon_from_tool_id(EtToolPanel *self, EtToolId tool_id);

static gboolean _cb_widget_draw(
		GtkWidget *widget,
		cairo_t *cr,
		gpointer user_data)
{
	static bool is_first = true;
	if(is_first){
		EtToolPanel *self = _et_tool_panel;
		assert(self);

		bool ret = _change_cursor_icon_from_tool_id(self, et_etaion_get_tool_id());
		if(ret){
			is_first = false;
		}
	}

	return FALSE;
}

static bool _signal_et_tool_panel_change(EtToolPanel *self, EtToolId tool_id)
{
	if(NULL == self){
		et_bug("");
		return false;
	}

	if(NULL == self->slot_change){
		// ignore warning when in initialized.
		if(NULL != _et_tool_panel){
			et_warning("");
		}
		return true;
	}

	return self->slot_change(tool_id, self->slot_change_data);
}

static bool _et_tool_panel_add_tool(EtToolPanel *self, const EtToolInfo *info)
{
	if(NULL == self){
		et_bug("");
		return false;
	}
	if(NULL == info){
		et_bug("");
		return false;
	}
	if(NULL == info->icon){
		et_bug("");
		return false;
	}

	EtToolPanelItem *item = malloc(sizeof(EtToolPanelItem));
	if(NULL == item){
		et_critical("");
		return false;
	}
	item->tool_id = info->tool_id;

	GtkWidget *toggle_button = gtk_button_new();
	GtkWidget *icon = gtk_image_new_from_pixbuf(info->icon);
	gtk_button_set_image(GTK_BUTTON(toggle_button), icon);
	item->button = toggle_button;

	const int column = 1;
	size_t num_tool = pv_general_get_parray_num((void **)self->tools);
	int x = num_tool % column;
	int y = num_tool / column;
	printf("item:%s:%d, %d\n", info->name, x, y);
	gtk_grid_attach(GTK_GRID(self->toolpanel), toggle_button, x, y, 1, 1);

	g_signal_connect(toggle_button, "clicked",
			G_CALLBACK(_cb_et_tool_panel_clicked_button), NULL);

	EtToolPanelItem **items = realloc(self->tools, sizeof(EtToolPanelItem *) * (num_tool + 2));
	if(NULL == items){
		et_critical("");
		return false;
	}
	items[num_tool + 1] = NULL;
	items[num_tool] = item;
	self->tools = items;

	gtk_widget_show_all(self->toolpanel);

	return true;
}

EtToolPanel *et_tool_panel_init()
{
	EtToolPanel *self = (EtToolPanel *)malloc(sizeof(EtToolPanel));
	if(NULL == self){
		et_critical("");
		return NULL;
	}

	self->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	self->widget = self->box;
	self->toolpanel = gtk_grid_new();
	if(NULL == self->toolpanel){
		et_critical("");
		return NULL;
	}
	gtk_widget_set_name (GTK_WIDGET(self->toolpanel), "toolpanel");
	gtk_widget_set_size_request(self->toolpanel, 20, 20);

	g_signal_connect(GTK_WIDGET(self->widget), "draw",
			G_CALLBACK(_cb_widget_draw), NULL);

	gtk_box_pack_start(GTK_BOX(self->box), self->toolpanel,
			false, false, 3);

	self->tools = NULL;

	size_t num_tool = et_tool_get_num();
	if(num_tool <= 0){
		et_bug("");
		return NULL;
	}
	for(int i = 0; i < (int)num_tool; i++){
		const EtToolInfo *info = et_tool_get_info_from_id(i);
		if(NULL == info){
			et_error("");
			return NULL;
		}
		if(!_et_tool_panel_add_tool(self, info)){
			et_error("");
			return NULL;
		}
	}

	self->slot_change = NULL;
	self->slot_change_data = NULL;

	if(!_et_tool_panel_set_current_tool_id(self, et_etaion_get_tool_id())){
		et_error("");
		return NULL;
	}

	_et_tool_panel = self;

	et_assert(et_mouse_cursor_info_init());

	return _et_tool_panel;
}

GtkWidget *et_tool_panel_get_widget_frame()
{
	EtToolPanel *self = _et_tool_panel;
	if(NULL == self){
		et_bug("");
		return false;
	}

	return self->widget;
}

static EtToolId _et_tool_panel_tool_id_from_button(
		const EtToolPanel *self, const GtkWidget *button)
{
	if(NULL == self){
		et_bug("");
		return -1;
	}

	size_t num_tool = pv_general_get_parray_num((void **)self->tools);
	for(int i = 0; i < (int)num_tool; i++){
		if((self->tools[i])->button == button){
			et_debug("%d", (self->tools[i])->tool_id);
			return (self->tools[i])->tool_id;
		}
	}

	et_bug("");
	return -1;
}

static GtkWidget *_et_tool_panel_get_tool_button_from_id(
		const EtToolPanel *self, EtToolId tool_id)
{
	if(NULL == self){
		et_bug("");
		return NULL;
	}

	size_t num_tool = pv_general_get_parray_num((void **)self->tools);
	for(int i = 0; i < (int)num_tool; i++){
		if((self->tools[i])->tool_id == tool_id){
			return (self->tools[i])->button;
		}
	}

	et_bug("");
	return NULL;
}

static bool _change_cursor_icon_from_tool_id(EtToolPanel *self, EtToolId tool_id)
{
	et_assertf(0 <= tool_id, "%d", tool_id);

	GdkWindow *window = gtk_widget_get_parent_window(self->widget);
	if(!window){
		et_debug("GdkWindow not grub");
		return false;
	}else{
		const EtToolInfo *info = et_tool_get_info_from_id(tool_id);
		et_assertf(info, "%d", tool_id);
		gdk_window_set_cursor(window, info->mouse_cursor);
		return true;
	}
}

static bool _et_tool_panel_set_current_tool_from_button(
		EtToolPanel *self, const GtkWidget *button)
{
	if(NULL == self){
		et_bug("");
		return false;
	}

	// ** check exist tool.
	EtToolId tool_id = _et_tool_panel_tool_id_from_button(self, button);
	if(tool_id < 0){
		et_bug("");
		return false;
	}

	size_t num_tool = pv_general_get_parray_num((void **)self->tools);
	for(int i = 0; i < (int)num_tool; i++){
		EtToolPanelItem *item = self->tools[i];
		const EtToolInfo *info = et_tool_get_info_from_id(item->tool_id);
		et_assertf(info, "%d", item->tool_id);

		if(item->button == button){
			GtkWidget *icon = gtk_image_new_from_pixbuf(info->icon_focus);
			gtk_button_set_image(GTK_BUTTON(item->button), icon);
		}else{
			GtkWidget *icon = gtk_image_new_from_pixbuf(info->icon);
			gtk_button_set_image(GTK_BUTTON(item->button), icon);
		}
	}

	_change_cursor_icon_from_tool_id(self, tool_id);

	return true;
}

static bool _et_tool_panel_set_current_tool_id(EtToolPanel *self, EtToolId tool_id)
{
	GtkWidget *button = _et_tool_panel_get_tool_button_from_id(self, tool_id);
	if(NULL == button){
		et_bug("");
		return false;
	}

	if(!_et_tool_panel_set_current_tool_from_button(self, button)){
		et_bug("");
		return false;
	}

	return true;
}

bool et_tool_panel_set_current_tool_id(EtToolId tool_id)
{
	EtToolPanel *self = _et_tool_panel;
	if(NULL == self){
		et_bug("");
		return false;
	}

	return _et_tool_panel_set_current_tool_id(self, tool_id);
}

bool et_tool_panel_set_slot_change(EtToolPanelSlotChange slot, gpointer data)
{
	EtToolPanel *self = _et_tool_panel;
	if(NULL == self){
		et_bug("");
		return false;
	}

	if(NULL != self->slot_change){
		et_bug("");
		return false;
	}

	self->slot_change = slot;
	self->slot_change_data = data;

	return true;
}

static void _cb_et_tool_panel_clicked_button(GtkWidget *button, gpointer data)
{
	EtToolPanel *self = _et_tool_panel;
	if(NULL == self){
		et_bug("");
		return;
	}

	EtToolId tool_id = _et_tool_panel_tool_id_from_button(self, button);
	if(!_signal_et_tool_panel_change(self, tool_id)){
		et_warning("");
	}
}

