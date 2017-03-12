/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_COLOR_PANEL_H
#define include_ET_COLOR_PANEL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_color.h"
#include "et_state.h"
#include "et_doc_id.h"
#include "et_doc.h"

struct EtColorPanel;
typedef struct EtColorPanel EtColorPanel;

typedef void (*EtColorPanelSlotChangeColor)(PvColorPair color_pair, gpointer data);


EtColorPanel *et_color_panel_init();
GtkWidget *et_color_panel_get_widget_frame();
PvColorPair et_color_panel_get_color_pair();

/*
int et_color_panel_set_slot_change_color_pair(EtColorPanel *self,
		EtColorPanelSlotChange slot, gpointer data);
*/
// @brief kick from change current document.
void slot_et_color_panel_from_etaion_change_state(EtState state, gpointer data);
void slot_et_color_panel_from_doc_change(EtDoc *doc, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_COLOR_PANEL_H
