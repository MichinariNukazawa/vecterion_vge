#include "et_tool_info.h"

#include <math.h>
#include "et_error.h"
#include "pv_element_general.h"
#include "pv_element.h"
#include "pv_element_info.h"
#include "pv_focus.h"
#include "pv_rotate.h"
#include "pv_anchor_path.h"
#include "et_doc.h"
#include "et_doc_manager.h"
#include "et_color_panel.h"
#include "et_stroke_panel.h"
#include "et_tool_info_util.h"


bool et_tool_info_is_init_ = false; // check initialized tools

EtToolInfo et_tool_infos_[];



static EtToolInfo *et_tool_get_info_from_id_(EtToolId tool_id);

static GdkPixbuf *conv_new_icon_focus_(GdkPixbuf *pb_src)
{

	double w = gdk_pixbuf_get_width(pb_src);
	double h = gdk_pixbuf_get_height(pb_src);
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
	et_assert(surface);
	cairo_t *cr = cairo_create (surface);
	et_assert(cr);

	gdk_cairo_set_source_pixbuf (cr, pb_src, 0, 0);
	cairo_paint (cr);

	int T = 0;
	cairo_rectangle (cr, T, T, w - (T * 2), h - (T * 2));
	cairo_set_source_rgba (cr, 0.7, 0, 0, 0.3);
	cairo_set_line_width (cr, 2.0);
	cairo_stroke (cr);

	GdkPixbuf *pb = gdk_pixbuf_get_from_surface(surface, 0, 0, w, h);
	et_assert(pb);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}

static bool et_tool_info_init_inline_(const char *dirpath_application_base, bool is_test)
{
	et_assert(!et_tool_info_is_init_);

	size_t num_tool = et_tool_get_num();
	for(int tool_id = 0; tool_id < (int)num_tool; tool_id++){
		EtToolInfo *info = et_tool_get_info_from_id_(tool_id);
		et_assertf(info, "%d", tool_id);

		// ** make image(cursor,icons)
		char *path = g_strdup_printf("%s/%s", dirpath_application_base, info->filepath_cursor);
		et_assertf(path, "%s", info->filepath_cursor)
			GError *error = NULL;
		info->icon_cursor = gdk_pixbuf_new_from_file(path, &error);
		et_assertf(info->icon_cursor, "%d, '%s'", tool_id, error->message);
		free(path);

		info->icon = info->icon_cursor;
		info->icon_focus = conv_new_icon_focus_(info->icon);

		// ** mouse cursor
		if(is_test){
			info->mouse_cursor = (GdkCursor *)1;
		}else{
			assert(gdk_display_get_default());
			info->mouse_cursor = gdk_cursor_new_from_pixbuf(
					gdk_display_get_default(),
					info->icon_cursor,
					0, 0);
			et_assertf(info->mouse_cursor, "%d", tool_id);
		}
	}

	et_tool_info_is_init_ = true;

	et_debug("initialized EtToolInfo num:%zu", num_tool);

	return true;
}

bool et_tool_info_init(const char *dirpath_application_base)
{
	return et_tool_info_init_inline_(dirpath_application_base, false);
}

bool et_tool_info_init_for_unittest(const char *dirpath_application_base)
{
	return et_tool_info_init_inline_(dirpath_application_base, true);
}

PvPoint pv_resize_point_(PvPoint point, PvPoint resize, PvPoint center)
{
	PvPoint rel = pv_point_sub(point, center);
	rel = pv_point_mul(rel, resize);
	return pv_point_add(rel, center);
}

static bool func_edit_element_mouse_action_(
		EtDocId doc_id,
		PvDocumentPreference document_preference,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{

	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	et_assertf(vg, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	bool is_save = false;

	bool res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			&document_preference.snap_context,
			&is_save,
			mouse_action,
			edit_draw_element,
			cursor);

	if(is_save){
		et_doc_save_from_id(doc_id);
	}

	return res;
}

static bool func_add_anchor_point_mouse_action_(
		EtDocId doc_id,
		PvDocumentPreference document_preference,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	et_assertf(vg, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	bool is_save = false;

	PvColorPair color_pair = et_color_panel_get_color_pair();
	PvStroke stroke = et_stroke_panel_get_stroke();

	bool res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			&document_preference.snap_context,
			&is_save,
			mouse_action,
			edit_draw_element,
			cursor,
			color_pair,
			stroke);

	if(is_save){
		et_doc_save_from_id(doc_id);
	}

	return res;
}

static bool func_edit_anchor_point_mouse_action_(
		EtDocId doc_id,
		PvDocumentPreference document_preference,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	et_assertf(vg, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	bool is_save = false;

	bool res = et_tool_info_util_func_edit_anchor_point_mouse_action(
			vg,
			focus,
			&document_preference.snap_context,
			&is_save,
			mouse_action,
			edit_draw_element,
			cursor);

	if(is_save){
		et_doc_save_from_id(doc_id);
	}

	return res;
}

static bool func_edit_anchor_point_handle_mouse_action_(
		EtDocId doc_id,
		PvDocumentPreference document_preference,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	et_assertf(vg, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	bool is_save = false;

	bool res = et_tool_info_util_func_edit_anchor_point_handle_mouse_action(
			vg,
			focus,
			&document_preference.snap_context,
			&is_save,
			mouse_action,
			edit_draw_element,
			cursor);

	if(is_save){
		et_doc_save_from_id(doc_id);
	}

	return res;
}

static void curve_element_split_from_index_(PvElement *element, PvElement **p_element_post, int index)
{
	et_assertf(PvElementKind_Curve == element->kind, "%d", element->kind);

	PvElementCurveData *data = element->data;
	if(pv_anchor_path_get_is_close(data->anchor_path)){
		bool ret = pv_anchor_path_reorder_duplicate_open_from_index(data->anchor_path, index);
		et_assert(ret);

		*p_element_post = NULL;
	}else{
		PvAnchorPath *anchor_path_post = pv_anchor_path_split_new_from_index(data->anchor_path, index);
		et_assert(anchor_path_post);
		*p_element_post = pv_element_curve_new_set_anchor_path(anchor_path_post);
		et_assert(*p_element_post);
		pv_element_copy_property(*p_element_post, element);

		bool ret = pv_anchor_path_remove_delete_range(data->anchor_path, index + 1, -1);
		et_assert(ret);
	}
}

static int insert_anchor_point_down_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action);

static bool knife_anchor_point_down_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action)
{
	PvElement *element = pv_focus_get_first_element(focus);
	et_assert(element);

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	if(PvElementKind_Curve != element->kind){
		//! check element able edit and open path
		return true;
	}

	int index = -1;
	size_t num = info->func_get_num_anchor_point(element);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = info->func_get_anchor_point(element, i);
		if(is_bound_point_(
					PX_SENSITIVE_OF_TOUCH,
					ap->points[PvAnchorPointIndex_Point],
					mouse_action.point)){
			index = i;
			break;
		}
	}
	if(-1 == index){
		index = insert_anchor_point_down_(doc, focus, mouse_action);
	}
	if(-1 == index){
		return true;
	}

	PvElement *element_post = NULL;
	curve_element_split_from_index_(element, &element_post, index);
	if(NULL != element_post){
		pv_focus_clear_to_first_layer(focus);
		PvElement *parent_layer = pv_focus_get_first_layer(focus);
		pv_element_append_child(parent_layer, element, element_post);
		pv_focus_add_element(focus, element);
		pv_focus_add_element(focus, element_post);
	}

	return true;
}

static bool func_add_basic_shape_element_mouse_action_(
		EtDocId doc_id,
		PvDocumentPreference document_preference,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	et_assertf(vg, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	bool is_save = false;

	PvColorPair color_pair = et_color_panel_get_color_pair();
	PvStroke stroke = et_stroke_panel_get_stroke();

	bool res = et_tool_info_util_func_add_basic_shape_element_mouse_action(
			vg,
			focus,
			&document_preference.snap_context,
			&is_save,
			mouse_action,
			edit_draw_element,
			cursor,
			color_pair,
			stroke);

	if(is_save){
		et_doc_save_from_id(doc_id);
	}

	return res;
}

static bool func_knife_anchor_point_mouse_action_(
		EtDocId doc_id,
		PvDocumentPreference document_preference,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(doc, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				knife_anchor_point_down_(doc, focus, mouse_action);
			}
			break;
		case EtMouseAction_Up:
			{
				et_doc_save_from_id(doc_id);
			}
			break;
		default:
			break;
	}

	return true;
}

PvPoint pv_anchor_path_get_subdivide_point_from_percent(PvAnchorPath *anchor_path, int index, double per)
{
	PvAnchorPointP4 p4;
	bool ret = pv_anchor_path_get_anchor_point_p4_from_index(anchor_path, &p4, index);
	et_assert(ret);

	double t = per / 100.0;
	double tp = 1 - t;
	PvPoint p = {
		.x = t*t*t*p4.points[3].x + 3*t*t*tp*p4.points[2].x
			+ 3*t*tp*tp*p4.points[1].x + tp*tp*tp*p4.points[0].x,
		.y = t*t*t*p4.points[3].y + 3*t*t*tp*p4.points[2].y
			+ 3*t*tp*tp*p4.points[1].y + tp*tp*tp*p4.points[0].y,
	};

	return p;
}

/*
   {
   PvAnchorPointP4 p4;
   bool ret = pv_anchor_path_get_anchor_point_p4_from_index(anchor_path, &p4, index);
   et_assert(ret);

   double t = per / 100.0;
   double tp = 1 - t;
   PvPoint p = {
   .x = 3*(t*t*(p4.points[3].x-p4.points[2].x)
   +2*t*tp*(p4.points[2].x-p4.points[1].x)
   +tp*tp*(p4.points[1].x-p4.points[0].x)),
   .y = 3*(t*t*(p4.points[3].y-p4.points[2].y)
   +2*t*tp*(p4.points[2].y-p4.points[1].y)
   +tp*tp*(p4.points[1].y-p4.points[0].y)),
   };

   return p;
   }
 */

static void element_curve_get_nearest_index_percent_(
		int *index,
		int *percent,
		double px_sensitive,
		const PvElement *element,
		EtMouseAction mouse_action)
{
	double near_diff = px_sensitive + 1.0;
	*index = -1;
	*percent = -1;
	size_t num = pv_element_curve_get_num_anchor_point(element);
	for(int ix = 0; ix < (int)num; ix++){
		for(int per = 1; per <= 99; per++){
			PvElementCurveData *data = (PvElementCurveData *)element->data;
			et_assert(data);

			PvPoint point = pv_anchor_path_get_subdivide_point_from_percent(data->anchor_path, ix, per);
			double diff = pv_point_distance(point, mouse_action.point);
			if(diff < near_diff){
				near_diff = diff;
				*index = ix;
				*percent = per;
			}
		}
	}

	return;
}

void pv_anchor_path_get_subdivide_anchor_points_form_percent(
		PvAnchorPoint dst_aps[3],
		PvAnchorPath *anchor_path,
		int index,
		double percent)
{
	PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(anchor_path, index, PvAnchorPathIndexTurn_Disable);
	et_assert(ap_prev);
	PvAnchorPoint *ap_next = pv_anchor_path_get_anchor_point_from_index(anchor_path, index + 1, PvAnchorPathIndexTurn_OnlyLastInClosed);
	et_assert(ap_next);
	dst_aps[0] = *ap_prev;
	dst_aps[1] = PvAnchorPoint_Default;
	dst_aps[2] = *ap_next;

	PvPoint p0 = pv_anchor_point_get_point(ap_prev);
	PvPoint pn = pv_anchor_path_get_subdivide_point_from_percent(anchor_path, index, percent);
	PvPoint p1 = pv_anchor_point_get_point(ap_next);

	PvPoint p0_handle_next = pv_anchor_point_get_handle(ap_prev, PvAnchorPointIndex_HandleNext);
	PvPoint p1_handle_prev = pv_anchor_point_get_handle(ap_next, PvAnchorPointIndex_HandlePrev);

	PvPoint p0_handle_next_dst = pv_point_subdivide(p0, p0_handle_next, percent);
	PvPoint p1_handle_prev_dst = pv_point_subdivide(p1, p1_handle_prev, (100.0 - percent));

	PvPoint handle_center = pv_point_subdivide(p0_handle_next, p1_handle_prev, percent);
	PvPoint pn_handle_prev = pv_point_subdivide(p0_handle_next_dst, handle_center, percent);
	PvPoint pn_handle_next = pv_point_subdivide(handle_center, p1_handle_prev_dst, percent);

	pv_anchor_point_set_handle(&dst_aps[0], PvAnchorPointIndex_HandleNext, p0_handle_next_dst);
	pv_anchor_point_set_point(&dst_aps[1], pn);
	pv_anchor_point_set_handle(&dst_aps[1], PvAnchorPointIndex_HandlePrev, pn_handle_prev);
	pv_anchor_point_set_handle(&dst_aps[1], PvAnchorPointIndex_HandleNext, pn_handle_next);
	pv_anchor_point_set_handle(&dst_aps[2], PvAnchorPointIndex_HandlePrev, p1_handle_prev_dst);
}

static int element_curve_insert_anchor_point_from_index_percent_(
		PvElement *element, int index, double percent)
{
	PvElementCurveData *data = (PvElementCurveData *)element->data;
	et_assert(data);

	PvAnchorPoint dst_aps[3];
	pv_anchor_path_get_subdivide_anchor_points_form_percent(dst_aps, data->anchor_path, index, percent);
	PvAnchorPoint *ap = pv_anchor_path_insert_anchor_point(data->anchor_path, &dst_aps[1], index + 1);
	et_assert(ap);
	int new_index = index + 1;

	// last anchor_point -> first anchor_point
	int prev_index = new_index - 1;
	int next_index = new_index + 1;
	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	if(next_index == (int)num){
		next_index = 0;
	}

	bool ret;
	ret = pv_anchor_path_set_anchor_point_from_index(data->anchor_path, prev_index, &dst_aps[0]);
	et_assert(ret);
	ret = pv_anchor_path_set_anchor_point_from_index(data->anchor_path, next_index, &dst_aps[2]);
	et_assert(ret);

	/*
	   PvPoint point = pv_anchor_path_get_subdivide_point_from_percent(data->anchor_path, index, percent);
	   PvAnchorPoint ap = PvAnchorPoint_Default;
	   pv_anchor_point_set_point(&ap, point);
	   int new_index = pv_anchor_path_insert_anchor_point(data->anchor_path, ap, index - 1);
	 */
	// @todo handle.

	et_debug("%d", new_index);
	return new_index;
}

static int insert_anchor_point_down_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action)
{
	size_t num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num; i++){
		PvElement *element = focus->elements[i];
		if(PvElementKind_Curve != element->kind){
			continue;
		}

		int percent = -1;
		int index = -1;
		element_curve_get_nearest_index_percent_(
				&index,
				&percent,
				PX_SENSITIVE_OF_TOUCH,
				element,
				mouse_action);
		if(-1 != index){
			int new_index = element_curve_insert_anchor_point_from_index_percent_(
					element,
					index,
					percent);
			if(-1 != new_index){
				bool ret = pv_focus_clear_set_element_index(
						focus,
						element,
						new_index);
				et_assertf(ret, "%d", new_index);
			}
			return new_index;
		}
	}

	return -1;
}

static bool func_insert_anchor_point_mouse_action_(
		EtDocId doc_id,
		PvDocumentPreference document_preference,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(doc, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				insert_anchor_point_down_(doc, focus, mouse_action);
			}
			break;
		case EtMouseAction_Move:
			break;
		case EtMouseAction_Up:
			{
				et_doc_save_from_id(doc_id);
			}
			break;
		default:
			break;
	}

	return true;
}

EtToolInfo et_tool_infos_[] = {
	{
		.tool_id = EtToolId_EditElement,
		.name = "Edit Element",
		.is_element_tool = true,
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_element_allow_24x24.svg",
		.shortcuts = {
			{GDK_KEY_v, 0,},
			{0, (0),},
		},
		.func_mouse_action = func_edit_element_mouse_action_,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = EtToolId_EditAnchorPoint,
		.name = "Edit Anchor Point",
		.is_element_tool = false,
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_edit_allow_24x24.svg",
		.func_mouse_action = func_edit_anchor_point_mouse_action_,
		.shortcuts = {
			{GDK_KEY_a, 0,},
			{0, (0),},
		},
		.mouse_cursor = NULL,
	},
	{
		.tool_id = EtToolId_AddAnchorPoint,
		.name = "Add Anchor Point",
		.is_element_tool = false,
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_put_allow_24x24.svg",
		.shortcuts = {
			{GDK_KEY_p, 0,},
			{0, (0),},
		},
		.func_mouse_action = func_add_anchor_point_mouse_action_,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = EtToolId_EditAnchorPointHandle,
		.name = "Edit Anchor Point Handle",
		.is_element_tool = false,
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_handle_allow_24x24.svg",
		.shortcuts = {
			{GDK_KEY_c, (GDK_SHIFT_MASK),},
			{0, (0),},
		},
		.func_mouse_action = func_edit_anchor_point_handle_mouse_action_,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = EtToolId_AddBasicShapeElement,
		.name = "Add Basic Shape Element",
		.is_element_tool = true,
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_basic_shape_24x24.svg",
		.shortcuts = {
			{0, (0),},
		},
		.func_mouse_action = func_add_basic_shape_element_mouse_action_,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = EtToolId_KnifeAnchorPoint,
		.name = "Knife Anchor Point",
		.is_element_tool = false,
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_knife_24x24.svg",
		.shortcuts = {
			{GDK_KEY_c, 0,},
			{0, (0),},
		},
		.func_mouse_action = func_knife_anchor_point_mouse_action_,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = EtToolId_InsertAnchorPoint,
		.name = "Insert Anchor Point",
		.is_element_tool = false,
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_put_allow_plus_24x24.svg",
		.shortcuts = {
			{GDK_KEY_plus, (GDK_SHIFT_MASK),},
			{GDK_KEY_KP_Add, (GDK_SHIFT_MASK),},
			{0, (0),},
		},
		.func_mouse_action = func_insert_anchor_point_mouse_action_,
		.mouse_cursor = NULL,
	},
};

size_t et_tool_get_num()
{
	return sizeof(et_tool_infos_) / sizeof(EtToolInfo);
}

static EtToolInfo *et_tool_get_info_from_id_(EtToolId tool_id)
{
	size_t num_tool = et_tool_get_num();
	if(tool_id < 0 || (int)num_tool <= tool_id){
		et_error("");
		return NULL;
	}

	return &et_tool_infos_[tool_id];
}

const EtToolInfo *et_tool_get_info_from_id(EtToolId tool_id)
{
	et_assert(et_tool_info_is_init_);

	return et_tool_get_info_from_id_(tool_id);
}

