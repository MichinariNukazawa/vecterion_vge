#ifndef include_ET_MOUSE_CURSOR_H
#define include_ET_MOUSE_CURSOR_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

typedef enum{
	EtMouseCursorId_Resize_UpRight,
	EtMouseCursorId_Resize_UpLeft,
	EtMouseCursorId_Resize_DownRight,
	EtMouseCursorId_Resize_DownLeft,
}EtMouseCursorId;

typedef struct{
	EtMouseCursorId id;
	const char *filepath;
	GdkPixbuf *pixbuf;
	GdkCursor *cursor;
	int center_x;
	int center_y;
}EtMouseCursorInfo;

bool et_mouse_cursor_info_init();
const EtMouseCursorInfo *et_mouse_cursor_get_info_from_id(EtMouseCursorId);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_MOUSE_CURSOR_H

