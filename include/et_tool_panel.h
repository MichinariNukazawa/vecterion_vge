#ifndef include_ET_TOOL_PANEL_H
#define include_ET_TOOL_PANEL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc.h"
#include "pv_render_context.h"
#include "et_state.h"
#include "et_tool.h"

struct EtToolPanel;
typedef struct EtToolPanel EtToolPanel;

typedef bool (*EtToolPanelSlotChange)(EtToolId tool_id, gpointer data);

/** @param tool_id of current. */
EtToolPanel *et_tool_panel_init();
GtkWidget *et_tool_panel_get_widget_frame();
bool et_tool_panel_set_current_tool_id(EtToolId tool_id);
//EtToolId et_tool_panel_get_current_tool_id();
bool et_tool_panel_set_slot_change(EtToolPanelSlotChange slot, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_TOOL_PANEL_H
