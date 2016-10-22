#ifndef include_ET_TOOL_INFO_H
#define include_ET_TOOL_INFO_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_tool_id.h"
#include "et_doc_id.h"
#include "et_mouse_action.h"



typedef bool (*EtToolFuncMouseAction)(EtDocId doc_id, EtMouseAction mouse_action);

typedef struct EtToolInfo{
	EtToolId tool_id;
	const char *name;
	GdkPixbuf *icon;
	GdkPixbuf *icon_focus;
	GdkPixbuf *cursor;

	const char *filepath_icon;
	const char *filepath_cursor;

	EtToolFuncMouseAction		func_mouse_action;
}EtToolInfo;

bool et_tool_info_init();
int et_tool_get_num();
const EtToolInfo *et_tool_get_info_from_id(EtToolId tool_id);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_TOOL_INFO_H

