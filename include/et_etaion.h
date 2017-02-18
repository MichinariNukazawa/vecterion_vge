#ifndef include_ET_ETAION_H
#define include_ET_ETAION_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "et_mouse_action.h"
#include "et_key_action.h"
#include "pv_element.h"
#include "et_state.h"
#include "et_tool_id.h"

struct EtEtaion;
typedef struct EtEtaion EtEtaion;

//! @ ChangeState is signaling when change (current) doc_id, document (content), focus.
typedef void (*EtEtaionSlotChangeState)(EtState state, gpointer data);
typedef void (*EtEtaionSlotChangeToolId)(EtToolId tool_id, gpointer data);

struct EtEtaion{
	EtState state;
	EtToolId tool_id;

	const char *application_path;
	GtkWidget *widget_on_mouse_cursor;

	EtEtaionSlotChangeState *slot_change_states;
	gpointer *slot_change_state_datas;
	EtEtaionSlotChangeToolId *slot_change_tool_ids;
	gpointer *slot_change_tool_id_datas;
};

EtEtaion *et_etaion_init();

bool et_etaion_set_current_doc_id(EtDocId doc_id);
EtDocId et_etaion_get_current_doc_id();
bool et_etaion_set_is_extent_view(bool is_extent_view);
bool et_etaion_get_is_extent_view();
EtToolId et_etaion_get_tool_id();
EtDocId et_etaion_get_current_doc_id();

void et_etaion_set_application_base_dir_from_execute_path(const char *execute_path);
const char *et_etaion_get_application_base_dir();

// @brief focus(Layer)の姉妹Layerを追加
bool et_etaion_append_new_layer(EtDocId doc_id);
// @brief focus(Layer)の子Layerを追加
bool et_etaion_append_new_layer_child(EtDocId doc_id);
/** @brief focus(Layer)および子Elementを複製して親レイヤーに追加 */
bool et_etaion_copy_layer(EtDocId doc_id);
/** @brief focus(Layer)および子Elementを削除 */
bool et_etaion_remove_delete_layer(EtDocId doc_id);
/** @brief focus(非Layer)Elementsを削除 */
bool et_etaion_remove_delete_by_focusing(EtDocId doc_id);

bool slot_et_etaion_from_mouse_action(EtDocId doc_id, EtMouseAction mouse_action);
bool slot_et_etaion_from_key_action(EtKeyAction key_action);
bool slot_et_etaion_change_tool(EtToolId tool_id, gpointer data);

void et_etaion_set_widget_on_mouse_cursor(GtkWidget *);

int et_etaion_add_slot_change_state(EtEtaionSlotChangeState slot, gpointer data);
int et_etaion_add_slot_change_tool_id(EtEtaionSlotChangeToolId slot, gpointer data);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_ETAION_H
