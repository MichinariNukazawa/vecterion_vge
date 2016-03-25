#ifndef include_ET_DOC_HISTORY_H
#define include_ET_DOC_HISTORY_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_vg.h"
#include "pv_focus.h"

typedef struct EtDocHistoryItem{
	PvFocus 	focus;
	PvVg		*vg;
}EtDocHistoryItem;

typedef struct EtDocHistory{
	int ix_current;
	int ix_redo;
	int ix_undo;

	int num_history;
	EtDocHistoryItem hist_work;
	EtDocHistoryItem *hists;
}EtDocHistory;

EtDocHistory *et_doc_history_new(const PvVg *vg);
EtDocHistoryItem *et_doc_history_get_from_relative(EtDocHistory *hist, int relative);
bool et_doc_history_save_with_focus(EtDocHistory *hist);
bool et_doc_history_undo(EtDocHistory *hist);
bool et_doc_history_redo(EtDocHistory *hist);
int et_doc_history_get_num_undo(EtDocHistory *hist);
int et_doc_history_get_num_redo(EtDocHistory *hist);


#ifdef include_ET_TEST
void et_doc_history_debug_print(const EtDocHistory *hist);
#endif // include_ET_TEST

#endif // include_ET_DOC_HISTORY_H
