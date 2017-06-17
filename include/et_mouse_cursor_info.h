/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_MOUSE_CURSOR_INFO_H
#define include_ET_MOUSE_CURSOR_INFO_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

typedef enum{
	EtMouseCursorId_Resize_UpRight,
	EtMouseCursorId_Resize_UpLeft,
	EtMouseCursorId_Resize_DownRight,
	EtMouseCursorId_Resize_DownLeft,
	EtMouseCursorId_Rotate_UpRight,
	EtMouseCursorId_Rotate_UpLeft,
	EtMouseCursorId_Rotate_DownRight,
	EtMouseCursorId_Rotate_DownLeft,
}EtMouseCursorId;

typedef struct{
	EtMouseCursorId id;
	const char *filepath;
	GdkPixbuf *pixbuf;
	GdkCursor *cursor;
	int center_x;
	int center_y;
}EtMouseCursorInfo;

bool et_mouse_cursor_info_init(const char *dirpath_application_base);
const EtMouseCursorInfo *et_mouse_cursor_get_info_from_id(EtMouseCursorId);

#ifdef include_ET_TEST
bool et_mouse_cursor_info_init_for_unittest(const char *dirpath_application_base);
#endif // include_ET_TEST

#endif // include_ET_MOUSE_CURSOR_INFO_H

