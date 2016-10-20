#ifndef include_ET_SNAP_PANEL_H
#define include_ET_SNAP_PANEL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_snap.h"
#include "et_state.h"
#include "et_doc_id.h"
#include "et_doc.h"

struct EtSnapPanel;
typedef struct EtSnapPanel EtSnapPanel;

EtSnapPanel *et_snap_panel_init();
GtkWidget *et_snap_panel_get_widget_frame();
EtSnap et_snap_panel_get_snap();

// @brief kick from change current document.
void slot_et_snap_panel_from_etaion_change_state(EtState state, gpointer data);
void slot_et_snap_panel_from_doc_change(EtDoc *doc, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_SNAP_PANEL_H
