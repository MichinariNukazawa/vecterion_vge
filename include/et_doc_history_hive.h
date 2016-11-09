#ifndef include_ET_DOC_HISTORY_HIVE_H
#define include_ET_DOC_HISTORY_HIVE_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_vg.h"
#include "pv_focus.h"
#include "et_doc_history.h"

typedef struct EtDocHistoryHive{
	int ix_current;
	int ix_redo;
	int ix_undo;

	int num_history;
	EtDocHistory hist_work;
	EtDocHistory *hists;
}EtDocHistoryHive;

EtDocHistoryHive *et_doc_history_hive_new(const PvVg *vg);
void et_doc_history_hive_free(EtDocHistoryHive *);
EtDocHistory *et_doc_history_get_from_relative(EtDocHistoryHive *hist, int relative);
EtDocHistory *et_doc_history_hive_get_current(EtDocHistoryHive *hist);
bool et_doc_history_hive_save_with_focus(EtDocHistoryHive *hist);
bool et_doc_history_hive_undo(EtDocHistoryHive *hist);
bool et_doc_history_hive_redo(EtDocHistoryHive *hist);
int et_doc_history_hive_get_num_undo(EtDocHistoryHive *hist);
int et_doc_history_hive_get_num_redo(EtDocHistoryHive *hist);


#ifdef include_ET_TEST
void et_doc_history_hive_debug_print(const EtDocHistoryHive *hist);
#endif // include_ET_TEST

#endif // include_ET_DOC_HISTORY_HIVE_H
