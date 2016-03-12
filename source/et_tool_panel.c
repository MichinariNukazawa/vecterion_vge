#include "et_tool_panel.h"

#include <stdlib.h>
#include "et_error.h"
#include "pv_element_general.h"
#include "et_etaion.h"

static EtToolPanel *_et_tool_panel = NULL;

typedef struct EtToolPanelItem{
	EtToolId tool_id;
	GdkPixbuf *icon_focus;
	GdkPixbuf *icon_unfocus;
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

void _cb_et_tool_panel_clicked_button(GtkWidget *toggle_button, gpointer data);
bool _et_tool_panel_set_current_tool_id(EtToolPanel *this, EtToolId tool_id);

bool _signal_et_tool_panel_change(EtToolPanel *this, EtToolId tool_id)
{
	if(NULL == this){
		et_bug("");
		return false;
	}

	if(NULL == this->slot_change){
		// ignore warning when in initialized.
		if(NULL != _et_tool_panel){
			et_warning("");
		}
		return true;
	}

	return this->slot_change(tool_id, this->slot_change_data);
}

bool _et_tool_panel_add_tool(EtToolPanel *this, const EtToolInfo *info)
{
	if(NULL == this){
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

	item->icon_unfocus = info->icon;
	item->icon_focus = info->icon_focus;
	if(NULL == item->icon_unfocus || NULL == item->icon_unfocus){
		et_critical("");
		return false;
	}

	GtkWidget *toggle_button = gtk_button_new();
	GtkWidget *icon = gtk_image_new_from_pixbuf(info->icon);
	gtk_button_set_image(GTK_BUTTON(toggle_button), icon);
	item->button = toggle_button;

	int num_tool = pv_general_get_parray_num((void **)this->tools);
	int x = num_tool % 2;
	int y = num_tool / 2;
	printf("item:%s:%d, %d\n", info->name, x, y);
	gtk_grid_attach(GTK_GRID(this->toolpanel), toggle_button, x, y, 1, 1);

	g_signal_connect(toggle_button, "clicked",
			G_CALLBACK(_cb_et_tool_panel_clicked_button), NULL);

	EtToolPanelItem **items = realloc(this->tools, sizeof(EtToolPanelItem *) * (num_tool + 2));
	if(NULL == items){
		et_critical("");
		return false;
	}
	items[num_tool + 1] = NULL;
	items[num_tool] = item;
	this->tools = items;

	gtk_widget_show_all(this->toolpanel);

	return true;
}

EtToolPanel *et_tool_panel_init()
{
	EtToolPanel *this = (EtToolPanel *)malloc(sizeof(EtToolPanel));
	if(NULL == this){
		et_critical("");
		return NULL;
	}

	this->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	this->widget = this->box;
	this->toolpanel = gtk_grid_new();
	if(NULL == this->toolpanel){
		et_critical("");
		return NULL;
	}
	gtk_widget_set_name (GTK_WIDGET(this->toolpanel), "toolpanel");
	gtk_widget_set_size_request(this->toolpanel, 20, 20);

	gtk_box_pack_start(GTK_BOX(this->box), this->toolpanel,
			false, false, 3);

	this->tools = NULL;

	int num_tool = et_tool_get_num();
	if(num_tool <= 0){
		et_bug("");
		return NULL;
	}
	for(int i = 0; i < num_tool; i++){
		const EtToolInfo *info = et_tool_get_info_from_id(i);
		if(NULL == info){
			et_error("");
			return NULL;
		}
		if(!_et_tool_panel_add_tool(this, info)){
			et_error("");
			return NULL;
		}
	}

	this->slot_change = NULL;
	this->slot_change_data = NULL;

	if(!_et_tool_panel_set_current_tool_id(this, et_etaion_get_tool_id())){
		et_error("");
		return NULL;
	}

	_et_tool_panel = this;

	return _et_tool_panel;
}

GtkWidget *et_tool_panel_get_widget_frame()
{
	EtToolPanel *this = _et_tool_panel;
	if(NULL == this){
		et_bug("");
		return false;
	}

	return this->widget;
}

EtToolId _et_tool_panel_tool_id_from_button(const EtToolPanel *this, const GtkWidget *button)
{
	if(NULL == this){
		et_bug("");
		return -1;
	}

	int num_tool = pv_general_get_parray_num((void **)this->tools);
	for(int i = 0; i < num_tool; i++){
		if((this->tools[i])->button == button){
			et_debug("%d\n", (this->tools[i])->tool_id);
			return (this->tools[i])->tool_id;
		}
	}

	et_bug("");
	return -1;
}

GtkWidget *_et_tool_panel_get_tool_button_from_id(const EtToolPanel *this, EtToolId tool_id)
{
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	int num_tool = pv_general_get_parray_num((void **)this->tools);
	for(int i = 0; i < num_tool; i++){
		if((this->tools[i])->tool_id == tool_id){
			return (this->tools[i])->button;
		}
	}

	et_bug("");
	return NULL;
}

bool _et_tool_panel_set_current_tool_from_button(EtToolPanel *this, const GtkWidget *button)
{
	if(NULL == this){
		et_bug("");
		return false;
	}

	// ** check exist tool.
	EtToolId tool_id = _et_tool_panel_tool_id_from_button(this, button);
	if(tool_id < 0){
		et_bug("");
		return false;
	}

	int num_tool = pv_general_get_parray_num((void **)this->tools);
	for(int i = 0; i < num_tool; i++){
		EtToolPanelItem *tool = this->tools[i];
		if(tool->button == button){
			GtkWidget *icon = gtk_image_new_from_pixbuf(tool->icon_focus);
			gtk_button_set_image(GTK_BUTTON(tool->button), icon);
		}else{
			GtkWidget *icon = gtk_image_new_from_pixbuf(tool->icon_unfocus);
			gtk_button_set_image(GTK_BUTTON(tool->button), icon);
		}
	}

	return true;
}

bool _et_tool_panel_set_current_tool_id(EtToolPanel *this, EtToolId tool_id)
{
	GtkWidget *button = _et_tool_panel_get_tool_button_from_id(this, tool_id);
	if(NULL == button){
		et_bug("");
		return false;
	}

	if(!_et_tool_panel_set_current_tool_from_button(this, button)){
		et_bug("");
		return false;
	}

	return true;
}

bool et_tool_panel_set_current_tool_id(EtToolId tool_id)
{
	EtToolPanel *this = _et_tool_panel;
	if(NULL == this){
		et_bug("");
		return false;
	}

	return _et_tool_panel_set_current_tool_id(this, tool_id);
}

bool et_tool_panel_set_slot_change(EtToolPanelSlotChange slot, gpointer data)
{
	EtToolPanel *this = _et_tool_panel;
	if(NULL == this){
		et_bug("");
		return false;
	}

	if(NULL != this->slot_change){
		et_bug("");
		return false;
	}

	this->slot_change = slot;
	this->slot_change_data = data;

	return true;
}

void _cb_et_tool_panel_clicked_button(GtkWidget *button, gpointer data)
{
	EtToolPanel *this = _et_tool_panel;
	if(NULL == this){
		et_bug("");
		return;
	}

	EtToolId tool_id = _et_tool_panel_tool_id_from_button(this, button);
	if(!_signal_et_tool_panel_change(this, tool_id)){
		et_warning("");
	}
}

