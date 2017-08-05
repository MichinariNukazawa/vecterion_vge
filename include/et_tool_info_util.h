/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_TOOL_INFO_UTIL_H
#define include_ET_TOOL_INFO_UTIL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_vg.h"
#include "pv_focus.h"
#include "pv_element.h"
#include "et_mouse_action.h"
#include "pv_snap_context.h"

#define PX_SENSITIVE_OF_TOUCH		(16)
#define PX_SENSITIVE_OF_MOVE		(3)

typedef enum{
	EdgeKind_None,
	EdgeKind_Resize_UpRight,
	EdgeKind_Resize_UpLeft,
	EdgeKind_Resize_DownRight,
	EdgeKind_Resize_DownLeft,
	EdgeKind_Rotate_UpRight,
	EdgeKind_Rotate_UpLeft,
	EdgeKind_Rotate_DownRight,
	EdgeKind_Rotate_DownLeft,
}EdgeKind;

typedef enum{
	EtFocusElementMouseActionMode_None,
	EtFocusElementMouseActionMode_Translate,
	EtFocusElementMouseActionMode_FocusingByArea,
	EtFocusElementMouseActionMode_Resize,
	EtFocusElementMouseActionMode_Rotate,
	EtFocusElementMouseActionMode_Handle,
}EtFocusElementMouseActionMode;

bool et_tool_info_util_func_edit_element_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor);

bool et_tool_info_util_func_edit_anchor_point_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor);

bool et_tool_info_util_func_edit_anchor_point_handle_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor);

bool et_tool_info_util_func_add_anchor_point_handle_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor,
		PvColorPair color_pair,
		PvStroke stroke
		);

bool et_tool_info_util_func_add_figure_shape_element_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor,
		PvColorPair color_pair,
		PvStroke stroke
		);

//! @todo temporary function on header.

bool is_bound_point_(int radius, PvPoint p1, PvPoint p2);
PvRect get_rect_extent_from_elements_(PvElement **elements);

#ifdef include_ET_TEST

EdgeKind resize_elements_(
		PvElement **elements,
		const PvSnapContext *snap_context,
		EtMouseAction mouse_action,
		EdgeKind src_edge_kind_,
		PvRect src_extent_rect);

#endif // include_ET_TEST

#endif // include_ET_TOOL_INFO_UTIL_H

