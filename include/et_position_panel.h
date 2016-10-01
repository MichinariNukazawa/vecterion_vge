#ifndef include_ET_POSITION_PANEL_H
#define include_ET_POSITION_PANEL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_state.h"
#include "et_doc_id.h"
#include "et_doc.h"

struct EtPositionPanel;
typedef struct EtPositionPanel EtPositionPanel;

EtPositionPanel *et_position_panel_init();
GtkWidget *et_position_panel_get_widget_frame();
// PvPosition et_position_panel_get_position();

// @brief kick from change current document.
void slot_et_position_panel_from_etaion_change_state(EtState state, gpointer data);
void slot_et_position_panel_from_doc_change(EtDoc *doc, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_POSITION_PANEL_H
