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
#include "et_mouse_cursor.h"


bool et_tool_info_is_init_ = false; // check initialized tools

EtToolInfo et_tool_infos_[];


static int PX_SENSITIVE_OF_TOUCH = 16;
const int PX_SENSITIVE_OF_MOVE = 3;


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


static EdgeKind get_outside_touch_edge_kind_from_rect_(PvRect rect, PvPoint point, double scale);


static EtToolInfo *et_tool_get_info_from_id_(EtToolId tool_id);

static EdgeKind get_outside_touch_edge_kind_from_rect_(PvRect rect, PvPoint point, double scale)
{
	const int PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH = 16;
	const double SENSE = PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH / scale;
	// resize
	PvRect ul = {
		(rect.x - (SENSE / 2.0)),
		(rect.y - (SENSE / 2.0)),
		SENSE, SENSE};
	PvRect ur = {
		(rect.x + rect.w - (SENSE / 2.0)),
		(rect.y - (SENSE / 2.0)),
		SENSE, SENSE};
	PvRect dl = {
		(rect.x - (SENSE / 2.0)),
		(rect.y + rect.h - (SENSE / 2.0)),
		SENSE, SENSE};
	PvRect dr = {
		(rect.x + rect.w - (SENSE / 2.0)),
		(rect.y + rect.h - (SENSE / 2.0)),
		SENSE, SENSE};
	// rotate
	PvRect ul_rotate = {(rect.x - SENSE),	(rect.y - SENSE),	SENSE, SENSE};
	PvRect ur_rotate = {(rect.x + rect.w),	(rect.y - SENSE),	SENSE, SENSE};
	PvRect dl_rotate = {(rect.x - SENSE),	(rect.y + rect.h),	SENSE, SENSE};
	PvRect dr_rotate = {(rect.x + rect.w),	(rect.y + rect.h),	SENSE, SENSE};

	if(pv_rect_is_inside(ul, point)){
		return EdgeKind_Resize_UpLeft;
	}else if(pv_rect_is_inside(ur, point)){
		return EdgeKind_Resize_UpRight;
	}else if(pv_rect_is_inside(dr, point)){
		return EdgeKind_Resize_DownRight;
	}else if(pv_rect_is_inside(dl, point)){
		return EdgeKind_Resize_DownLeft;
	}else if(pv_rect_is_inside(ul_rotate, point)){
		return EdgeKind_Rotate_UpLeft;
	}else if(pv_rect_is_inside(ur_rotate, point)){
		return EdgeKind_Rotate_UpRight;
	}else if(pv_rect_is_inside(dr_rotate, point)){
		return EdgeKind_Rotate_DownRight;
	}else if(pv_rect_is_inside(dl_rotate, point)){
		return EdgeKind_Rotate_DownLeft;
	}else{
		return EdgeKind_None;
	}
}

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

bool et_tool_info_init(const char *dirpath_application_base)
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
		assert(gdk_display_get_default());
		info->mouse_cursor = gdk_cursor_new_from_pixbuf(
				gdk_display_get_default(),
				info->icon_cursor,
				0, 0);
		et_assertf(info->mouse_cursor, "%d", tool_id);
	}

	et_tool_info_is_init_ = true;

	et_debug("initialized EtToolInfo num:%zu", num_tool);

	return true;
}

static bool is_bound_point_(int radius, PvPoint p1, PvPoint p2)
{
	if(
			(p1.x - radius) < p2.x
			&& p2.x < (p1.x + radius)
			&& (p1.y - radius) < p2.y
			&& p2.y < (p1.y + radius)
	  ){
		return true;
	}else{
		return false;
	}
}

static PvRect get_rect_extent_from_elements_(PvElement **elements)
{
	PvRect rect_extent = PvRect_Default;
	size_t num = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < (int)num; i++){
		const PvElement *element = elements[i];
		const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
		et_assertf(info, "%d", element->kind);

		PvRect rect = info->func_get_rect_by_anchor_points(element);

		if(0 == i){
			rect_extent = rect;
		}else{
			rect_extent = pv_rect_expand(rect_extent, rect);
		}
	}


	return rect_extent;
}

typedef struct RecursiveDataGetFocus{
	PvElement **p_element;
	PvAnchorPoint **p_anchor_point;
	PvPoint g_point;
}RecursiveDataGetFocus;

static bool func_recursive_get_touch_element_(PvElement *element, gpointer data, int level)
{
	RecursiveDataGetFocus *data_ = data;
	et_assert(data_);

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	et_assertf(info, "%d", element->kind);

	bool is_touch = false;
	bool ret = info->func_is_touch_element(
			&is_touch,
			element,
			PX_SENSITIVE_OF_TOUCH,
			data_->g_point.x,
			data_->g_point.y);
	if(!ret){
		et_bug("");
		return false;
	}
	if(is_touch){
		// ** detect is stop search.
		*(data_->p_element) = element;
		return false;
	}

	return true;
}

static PvElement *get_touch_element_(EtDocId doc_id, PvPoint g_point)
{
	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	et_assertf(vg, "%d", doc_id);

	PvElement *element = NULL;
	RecursiveDataGetFocus rec_data = {
		.p_element = &element,
		.g_point = g_point,
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_asc(
				vg->element_root,
				func_recursive_get_touch_element_,
				NULL,
				&rec_data,
				&error)){
		et_abortf("level:%d", error.level);
	}

	return element;
}

/*! @return is move */
static void translate_elements_(
		EtDocId doc_id,
		EtMouseAction mouse_action)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvPoint move = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

	size_t num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num; i++){
		PvElement *element = focus->elements[i];

		// remove work appearance
		element->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

		// append work appearance
		PvAppearance appearance0 = {
			.kind = PvAppearanceKind_Translate,
			.translate = (PvAppearanceTranslateData){
				.move = move,
			},
		};
		*(element->etaion_work_appearances[0]) = appearance0;
		element->etaion_work_appearances[1]->kind = PvAppearanceKind_None;
	}

	return;
}

PvPoint pv_resize_point_(PvPoint point, PvPoint resize, PvPoint center)
{
	PvPoint rel = pv_point_sub(point, center);
	rel = pv_point_mul(rel, resize);
	return pv_point_add(rel, center);
}

static PvPoint pv_resize_diff_(PvPoint size, PvPoint resize)
{
	PvPoint diff_ = {
		.x = (size.x - (size.x * resize.x)),
		.y = (size.y - (size.y * resize.y)),
	};

	return diff_;
}

static void get_resize_in_rect_(
		PvPoint *move_upleft_,
		PvPoint *resize_,
		EdgeKind dst_edge_kind,
		PvRect src_extent_rect,
		PvPoint move)
{
	PvPoint move_upleft = (PvPoint){0,0};
	PvPoint size_after;
	switch(dst_edge_kind){
		case EdgeKind_Resize_UpLeft:
			move_upleft = (PvPoint){move.x, move.y};
			size_after.x = (src_extent_rect.w - move.x);
			size_after.y = (src_extent_rect.h - move.y);
			break;
		case EdgeKind_Resize_UpRight:
			move_upleft = (PvPoint){0, move.y};
			size_after.x = (src_extent_rect.w + move.x);
			size_after.y = (src_extent_rect.h - move.y);
			break;
		case EdgeKind_Resize_DownLeft:
			move_upleft = (PvPoint){move.x, 0};
			size_after.x = (src_extent_rect.w - move.x);
			size_after.y = (src_extent_rect.h + move.y);
			break;
		case EdgeKind_Resize_DownRight:
			move_upleft = (PvPoint){0,0};
			size_after.x = (src_extent_rect.w + move.x);
			size_after.y = (src_extent_rect.h + move.y);
			break;
		default:
			et_abort();
			break;
	}

	PvPoint resize = {
		.x = size_after.x / src_extent_rect.w,
		.y = size_after.y / src_extent_rect.h,
	};

	if(src_extent_rect.w < DELTA_OF_RESIZE){
		resize.x = 1;
	}
	if(src_extent_rect.h < DELTA_OF_RESIZE){
		resize.y = 1;
	}

	*move_upleft_ = move_upleft;
	*resize_ = resize;
}
static EdgeKind resize_elements_(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		EdgeKind src_edge_kind_,
		PvRect src_extent_rect)
{
	EdgeKind dst_edge_kind = src_edge_kind_;

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvPoint move = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

	PvPoint move_upleft;
	PvPoint resize;
	get_resize_in_rect_(&move_upleft, &resize, dst_edge_kind, src_extent_rect, move);

	//! @todo delta needed?
	/*
	   const double DELTA_OF_RESIZE = 0.001;
	   resize.x = ((fabs(resize.x) > DELTA_OF_RESIZE) ? resize.x : DELTA_OF_RESIZE);
	   resize.y = ((fabs(resize.y) > DELTA_OF_RESIZE) ? resize.y : DELTA_OF_RESIZE);
	 */

	size_t num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num; i++){
		PvElement *element = focus->elements[i];
		const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);

		PvRect rect_ = info->func_get_rect_by_anchor_points(element);

		// remove work appearance
		element->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

		// calculate work appearance
		PvPoint pos_ = {.x = rect_.x, .y = rect_.y};
		PvPoint src_extent_pos = {.x = src_extent_rect.x, .y = src_extent_rect.y};
		PvPoint move_upleft_ = pv_resize_diff_(pv_point_sub(pos_, src_extent_pos), resize);
		PvPoint move_upleft_by_resize_ = {
			.x = ((rect_.w - (rect_.w * resize.x)) / 2),
			.y = ((rect_.h - (rect_.h * resize.y)) / 2),
		};
		PvPoint translate_move_ = {
			// rect of elements, elements in element, element resize offset
			.x = move_upleft.x - move_upleft_.x - move_upleft_by_resize_.x,
			.y = move_upleft.y - move_upleft_.y - move_upleft_by_resize_.y,
		};

		// append work appearance
		PvAppearance appearance0 = {
			.kind = PvAppearanceKind_Translate,
			.translate = (PvAppearanceTranslateData){
				.move = translate_move_,
			},
		};
		PvAppearance appearance1 = {
			.kind = PvAppearanceKind_Resize,
			.resize = (PvAppearanceResizeData){
				.resize = resize,
			},
		};
		*(element->etaion_work_appearances[0]) = appearance0;
		*(element->etaion_work_appearances[1]) = appearance1;
		element->etaion_work_appearances[2]->kind = PvAppearanceKind_None;
	}

	return dst_edge_kind;
}

static double get_degree_from_radian_(double radian)
{
	return radian * (180.0 / M_PI);
	// return radian * 180.0 / (atan(1.0) * 4.0);
}

static double get_degree_from_point(PvPoint point)
{
	double radian = atan2(point.y, point.x);
	double degree = get_degree_from_radian_(radian);
	return degree;
}

static EdgeKind rotate_elements_(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		EdgeKind src_edge_kind_,
		PvRect src_extent_rect)
{
	EdgeKind dst_edge_kind = src_edge_kind_;

	PvPoint diff = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

	PvPoint extent_center = {
		.x = src_extent_rect.x + (src_extent_rect.w / 2),
		.y = src_extent_rect.y + (src_extent_rect.h / 2),
	};

	PvPoint dst_point = mouse_action.point;
	PvPoint src_point = pv_point_sub(dst_point, diff);

	double src_degree = get_degree_from_point(pv_point_sub(extent_center, src_point));
	double dst_degree = get_degree_from_point(pv_point_sub(extent_center, dst_point));

	double degree = dst_degree - src_degree;

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvElement **elements = focus->elements;
	size_t num_ = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < (int)num_; i++){
		PvElement *element = elements[i];
		const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);

		// remove work appearance
		element->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

		// calculate work appearance
		PvRect src_rect = info->func_get_rect_by_anchor_points(element);
		PvPoint src_center = pv_rect_get_center(src_rect);
		PvPoint dst_center = pv_rotate_point(src_center, degree, extent_center);
		PvPoint move = pv_point_sub(dst_center, src_center);

		// append work appearance
		PvAppearance appearance0 = {
			.kind = PvAppearanceKind_Rotate,
			.rotate = (PvAppearanceRotateData){
				.degree = degree,
			},
		};
		PvAppearance appearance1 = {
			.kind = PvAppearanceKind_Translate,
			.translate = (PvAppearanceTranslateData){
				.move = move,
			},
		};
		*(element->etaion_work_appearances[0]) = appearance0;
		*(element->etaion_work_appearances[1]) = appearance1;
		element->etaion_work_appearances[2]->kind = PvAppearanceKind_None;
	}

	return dst_edge_kind;
}

typedef struct {
	PvFocus *focus;
	PvRect rect;
	int offset;
}RecursiveInlineFocusingByAreaData;

static bool recursive_inline_focusing_by_area_(PvElement *element, gpointer data, int level)
{
	RecursiveInlineFocusingByAreaData *data_ = data;
	PvFocus *focus = data_->focus;
	PvRect rect = data_->rect;
	int offset = data_->offset;

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	bool is_overlap = false;
	if(!info->func_is_overlap_rect(&is_overlap, element, offset, rect)){
		return false;
	}
	if(is_overlap){
		pv_focus_add_element(focus, element);
		size_t num = info->func_get_num_anchor_point(element);
		for(int i = 0; i < (int)num; i++){
			PvAnchorPoint *anchor_point = info->func_get_anchor_point(element, i);
			et_assert(anchor_point);

			if(pv_rect_is_inside(rect, pv_anchor_point_get_point(anchor_point))){
				pv_focus_add_anchor_point(focus, element, anchor_point);
			}
		}
	}

	return true;
}

static PvRect focusing_by_area_(
		EtDocId doc_id,
		EtMouseAction mouse_action)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);
	pv_focus_clear_to_first_layer(focus);

	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	et_assertf(vg, "%d", doc_id);

	PvPoint diff = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);
	PvRect rect = {
		.x = mouse_action.point.x - diff.x,
		.y = mouse_action.point.y - diff.y,
		.w = diff.x,
		.h = diff.y,
	};
	rect = pv_rect_abs_size(rect);

	RecursiveInlineFocusingByAreaData data = {
		.focus = focus,
		.rect = rect,
		.offset = 0,
	};
	PvElementRecursiveError error;
	bool ret = pv_element_recursive_desc_before(
			vg->element_root,
			recursive_inline_focusing_by_area_,
			&data,
			&error);

	et_assert(ret);

	return rect;
}

typedef enum{
	EtFocusElementMouseActionMode_None,
	EtFocusElementMouseActionMode_Translate,
	EtFocusElementMouseActionMode_FocusingByArea,
	EtFocusElementMouseActionMode_Resize,
	EtFocusElementMouseActionMode_Rotate,
	EtFocusElementMouseActionMode_Handle,
}EtFocusElementMouseActionMode;

static GdkCursor *get_cursor_from_edge_(EdgeKind edge)
{
	EtMouseCursorId mouse_cursor_id;
	switch(edge){
		case EdgeKind_Resize_UpLeft:
			mouse_cursor_id = EtMouseCursorId_Resize_UpLeft;
			break;
		case EdgeKind_Resize_UpRight:
			mouse_cursor_id = EtMouseCursorId_Resize_UpRight;
			break;
		case EdgeKind_Resize_DownLeft:
			mouse_cursor_id = EtMouseCursorId_Resize_DownLeft;
			break;
		case EdgeKind_Resize_DownRight:
			mouse_cursor_id = EtMouseCursorId_Resize_DownRight;
			break;
		case EdgeKind_Rotate_UpLeft:
			mouse_cursor_id = EtMouseCursorId_Rotate_UpLeft;
			break;
		case EdgeKind_Rotate_UpRight:
			mouse_cursor_id = EtMouseCursorId_Rotate_UpRight;
			break;
		case EdgeKind_Rotate_DownLeft:
			mouse_cursor_id = EtMouseCursorId_Rotate_DownLeft;
			break;
		case EdgeKind_Rotate_DownRight:
			mouse_cursor_id = EtMouseCursorId_Rotate_DownRight;
			break;
		case EdgeKind_None:
		default:
			return NULL;
			break;
	}

	GdkCursor *cursor = NULL;
	const EtMouseCursorInfo *info_cursor =
		et_mouse_cursor_get_info_from_id(mouse_cursor_id);
	cursor = info_cursor->cursor;

	return cursor;
}

PvRect get_square_rect_from_center_point_(PvPoint center, double size)
{
	return (PvRect){
		.x = center.x - (size / 2.0),
			.y = center.y - (size / 2.0),
			.w = size,
			.h = size,
	};
}

static PvElement *element_new_from_circle_(PvPoint center, double size)
{
	PvRect rect = {
		center.x - (size / 2),
		center.y - (size / 2),
		size,
		size,
	};

	return pv_element_curve_new_from_rect(rect);
}

static PvElement *group_edit_(EtDocId doc_id, EtFocusElementMouseActionMode mode_, PvRect focusing_mouse_rect, double scale)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvElement *element_group_edit_draw = pv_element_new(PvElementKind_Group);
	et_assert(element_group_edit_draw);

	PvRect after_extent_rect_ = get_rect_extent_from_elements_(focus->elements);
	if(pv_focus_is_focused(focus)){
		int size = 5.0 / scale;
		for(int i = 0; i < 4; i++){
			PvPoint center = pv_rect_get_edge_point(after_extent_rect_, i);
			PvRect rect = get_square_rect_from_center_point_(center, size);
			PvElement *element_curve = pv_element_curve_new_from_rect(rect);
			et_assert(element_curve);
			pv_element_append_child(element_group_edit_draw, NULL, element_curve);
		}
	}

	switch(mode_){
		case EtFocusElementMouseActionMode_Rotate:
			{
				if(pv_focus_is_focused(focus)){
					// rotate center mark
					PvPoint center = pv_rect_get_center(after_extent_rect_);
					int diameter = 8.0 / scale;
					PvElement *element_center = element_new_from_circle_(center, diameter);
					et_assert(element_center);
					pv_element_append_child(element_group_edit_draw, NULL, element_center);
				}
			}
			break;
		case EtFocusElementMouseActionMode_FocusingByArea:
			{
				PvElement *element_curve = pv_element_curve_new_from_rect(focusing_mouse_rect);
				et_assert(element_curve);
				element_curve->color_pair.colors[PvColorPairGround_ForGround] = PvColor_Working;
				element_curve->color_pair.colors[PvColorPairGround_BackGround] = PvColor_None;
				element_curve->stroke.width = 1.0 / scale;
				pv_element_append_child(element_group_edit_draw, NULL, element_curve);
			}
		default:
			break;
	}

	return element_group_edit_draw;
}

static bool func_edit_element_mouse_action_(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	static PvElement *touch_element_ = NULL;
	static bool is_already_focus_ = false;
	static bool is_move_ = false;
	static EtFocusElementMouseActionMode mode_ = EtFocusElementMouseActionMode_None;
	static EdgeKind mode_edge_ = EdgeKind_None;
	static PvRect src_extent_rect_;

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvRect src_extent_rect = PvRect_Default;
	EdgeKind edge = EdgeKind_None;

	if(pv_focus_is_focused(focus)){
		src_extent_rect = get_rect_extent_from_elements_(focus->elements);
		edge = get_outside_touch_edge_kind_from_rect_(
				src_extent_rect, mouse_action.point, mouse_action.scale);
	}

	PvRect focusing_mouse_rect = {mouse_action.point.x, mouse_action.point.y, 0, 0};

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				is_already_focus_ = false;
				is_move_ = false;
				mode_edge_ = edge;
				src_extent_rect_ = src_extent_rect;

				switch(edge){
					case EdgeKind_Resize_UpLeft:
					case EdgeKind_Resize_UpRight:
					case EdgeKind_Resize_DownLeft:
					case EdgeKind_Resize_DownRight:
						{
							mode_ = EtFocusElementMouseActionMode_Resize;
						}
						break;
					case EdgeKind_Rotate_UpLeft:
					case EdgeKind_Rotate_UpRight:
					case EdgeKind_Rotate_DownLeft:
					case EdgeKind_Rotate_DownRight:
						{
							mode_ = EtFocusElementMouseActionMode_Rotate;
						}
						break;
					case EdgeKind_None:
					default:
						{
							mode_ = EtFocusElementMouseActionMode_Translate;

							touch_element_ = get_touch_element_(doc_id, mouse_action.point);

							if(NULL == touch_element_){
								et_assertf(pv_focus_clear_to_first_layer(focus), "%d", doc_id);
								mode_ = EtFocusElementMouseActionMode_FocusingByArea;
							}else{
								is_already_focus_ = pv_focus_is_exist_element(focus, touch_element_);
								pv_focus_add_element(focus, touch_element_);
							}
						}
				}
			}
			break;
		case EtMouseAction_Move:
			{
				if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
					break;
				}

				if(0 != (mouse_action.state & GDK_SHIFT_MASK)){
					break;
				}

				if(!is_move_)
				{
					if( PX_SENSITIVE_OF_MOVE > abs(mouse_action.diff_down.x)
							&& PX_SENSITIVE_OF_MOVE > abs(mouse_action.diff_down.y))
					{
						break;
					}else{
						is_move_ = true;
						if(EtFocusElementMouseActionMode_Translate == mode_ && !is_already_focus_){
							pv_focus_clear_set_element(focus, pv_focus_get_first_element(focus));
						}
					}
				}

				switch(mode_){
					case EtFocusElementMouseActionMode_Translate:
						{
							translate_elements_(doc_id, mouse_action);
						}
						break;
					case EtFocusElementMouseActionMode_Resize:
						{
							resize_elements_(doc_id, mouse_action, mode_edge_, src_extent_rect_);
						}
						break;
					case EtFocusElementMouseActionMode_Rotate:
						{
							rotate_elements_(doc_id, mouse_action, mode_edge_, src_extent_rect_);
						}
						break;
					case EtFocusElementMouseActionMode_FocusingByArea:
						{
							focusing_mouse_rect = focusing_by_area_(doc_id, mouse_action);
						}
						break;
					default:
						break;
				}
			}
			break;
		case EtMouseAction_Up:
			{
				switch(mode_){
					case EtFocusElementMouseActionMode_Translate:
						{
							if(is_move_){
								// NOP
							}else{
								if(0 == (mouse_action.state & GDK_SHIFT_MASK)){
									et_assertf(pv_focus_clear_set_element(focus, touch_element_), "%d", doc_id);
								}else{
									if(!is_already_focus_){
										// NOP
									}else{
										et_assertf(pv_focus_remove_element(focus, touch_element_), "%d", doc_id);
									}
								}
							}
						}
						break;
					default:
						break;
				}

				// apply work appearances
				size_t num_ = pv_general_get_parray_num((void **)focus->elements);
				for(int i = 0; i < (int)num_; i++){
					PvElement *element = focus->elements[i];
					const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
					info->func_apply_appearances(element, element->etaion_work_appearances);
					element->etaion_work_appearances[0]->kind = PvAppearanceKind_None;
				}

				mode_edge_ = EdgeKind_None;
				mode_ = EtFocusElementMouseActionMode_None;

				et_doc_save_from_id(doc_id);
			}
			break;
		default:
			break;
	}

	// ** mouse cursor
	if(EtFocusElementMouseActionMode_None == mode_){ // tool is not active
		*cursor = get_cursor_from_edge_(edge);
	}else{
		*cursor = get_cursor_from_edge_(mode_edge_);
	}

	// ** focusing view by tool
	*edit_draw_element = group_edit_(doc_id, mode_, focusing_mouse_rect, mouse_action.scale);

	return true;
}

static void add_anchor_point_down_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action, bool *is_reverse)
{
	PvElement *element_ = pv_focus_get_first_element(focus);
	et_assert(element_);
	PvElementCurveData *data_ = (PvElementCurveData *) element_->data;
	et_assert(data_);

	PvAnchorPoint *anchor_point = pv_focus_get_first_anchor_point(focus);

	int index = 0;
	size_t num = 0;

	if(PvElementKind_Curve == element_->kind){
		index = pv_anchor_path_get_index_from_anchor_point(data_->anchor_path, anchor_point);
		num = pv_anchor_path_get_anchor_point_num(data_->anchor_path);
	}

	if(PvElementKind_Curve != element_->kind
			|| NULL == anchor_point
			|| (!((0 == index) || index == ((int)num - 1)))
			|| pv_anchor_path_get_is_close(data_->anchor_path)){
		// add new ElementCurve.

		anchor_point = pv_anchor_point_new_from_point(mouse_action.point);
		et_assert(anchor_point);
		PvElement *new_element = pv_element_curve_new_set_anchor_point(anchor_point);
		et_assert(new_element);
		pv_element_append_on_focusing(element_, new_element);

		new_element->color_pair = et_color_panel_get_color_pair();
		new_element->stroke = et_stroke_panel_get_stroke();

		element_ = new_element;
	}else{
		// edit focused ElementCurve
		int index_end = 0;
		PvAnchorPoint *head_ap = NULL;
		if(0 == index && 1 < num){
			index_end = (num - 1);
			*is_reverse = true;
		}else if(index == ((int)num - 1)){
			index_end = 0;
			index = num;
			*is_reverse = false;
		}else{
			et_abortf("%d %zu", index, num);
		}
		head_ap = pv_anchor_path_get_anchor_point_from_index(
				data_->anchor_path,
				index_end,
				PvAnchorPathIndexTurn_Disable);
		et_assertf(head_ap, "%d %d %zu", index_end, index, num);

		if(is_bound_point_(
					PX_SENSITIVE_OF_TOUCH,
					head_ap->points[PvAnchorPointIndex_Point],
					mouse_action.point)){
			// ** do close anchor_point
			pv_anchor_path_set_is_close(data_->anchor_path, true);
			anchor_point = head_ap;
		}else{
			// add AnchorPoint for ElementCurve.
			anchor_point = pv_anchor_point_new_from_point(mouse_action.point);
			et_assert(anchor_point);

			pv_element_curve_append_anchor_point(element_, anchor_point, index);
		}
	}

	pv_focus_clear_set_anchor_point(focus, element_, anchor_point);
}

static bool focused_anchor_point_move_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action, bool is_reverse)
{
	if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
		return true;
	}

	PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
	if(NULL == ap){
		return true;
	}

	PvElement *element_ = pv_focus_get_first_element(focus);
	et_assert(element_);
	if(PvElementKind_Curve != element_->kind){
		et_error("");
		return false;
	}

	PvElementCurveData *data_ = (PvElementCurveData *) element_->data;
	et_assert(data_);

	PvPoint p_ap = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_Point);
	PvPoint p_diff = pv_point_sub(p_ap, mouse_action.point);
	if(fabs(p_diff.x) < PX_SENSITIVE_OF_TOUCH && fabs(p_diff.y) < PX_SENSITIVE_OF_TOUCH){
		pv_anchor_point_set_handle_zero(ap, PvAnchorPointIndex_Point);
	}else{
		pv_anchor_point_set_handle(ap, PvAnchorPointIndex_Point, mouse_action.point);

		if(is_reverse){
			// AnchorPoint is head in AnchorPath
			pv_anchor_point_reverse_handle(ap);
		}
	}

	return true;
}

static bool func_add_anchor_point_mouse_action_(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(doc, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	bool result = true;

	static bool is_reverse;

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				add_anchor_point_down_(doc, focus, mouse_action, &is_reverse);
			}
			break;
		case EtMouseAction_Up:
			{
				et_doc_save_from_id(doc_id);
			}
			break;
		case EtMouseAction_Move:
			{
				result = focused_anchor_point_move_(doc, focus, mouse_action, is_reverse);
			}
			break;
		case EtMouseAction_Unknown:
		default:
			et_bug("0x%x", mouse_action.action);
			break;
	}

	return result;
}

static bool get_touch_anchor_point_recursive_inline_(
		PvElement *element, gpointer data_0, int level)
{
	RecursiveDataGetFocus *data_ = data_0;

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	assert(info);

	size_t num = info->func_get_num_anchor_point(element);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *anchor_point = info->func_get_anchor_point(element, i);

		if(is_bound_point_(
					PX_SENSITIVE_OF_TOUCH,
					anchor_point->points[PvAnchorPointIndex_Point],
					data_->g_point))
		{
			*(data_->p_anchor_point) = anchor_point;
			*(data_->p_element) = element;
			return false;
		}
	}

	return true;
}

void get_touch_anchor_point_(
		PvElement **p_element,
		PvAnchorPoint **p_anchor_point,
		EtDocId doc_id,
		PvPoint point)
{
	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		et_error("");
		return;
	}

	RecursiveDataGetFocus rec_data = {
		.p_element = p_element,
		.p_anchor_point = p_anchor_point,
		.g_point = point,
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_asc(vg->element_root,
				get_touch_anchor_point_recursive_inline_,
				NULL,
				&rec_data,
				&error)){
		et_error("level:%d", error.level);
		return;
	}

	return;
}

/*! @return handle(PvAnchorPointHandle) not grub: -1 */
int edit_anchor_point_handle_bound_handle_(const PvAnchorPoint *ap, EtMouseAction mouse_action)
{
	// ** grub handle.
	PvPoint p_point = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_Point);
	PvPoint p_prev = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_HandlePrev);
	PvPoint p_next = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_HandleNext);

	if(is_bound_point_(
				PX_SENSITIVE_OF_TOUCH,
				p_prev,
				mouse_action.point))
	{
		return PvAnchorPointIndex_HandlePrev;
	}
	if(is_bound_point_(
				PX_SENSITIVE_OF_TOUCH,
				p_next,
				mouse_action.point))
	{
		return PvAnchorPointIndex_HandleNext;
	}
	if(is_bound_point_(
				PX_SENSITIVE_OF_TOUCH,
				p_point,
				mouse_action.point))
	{
		return PvAnchorPointIndex_Point;
	}

	return -1;
}

/*! @brief
 * @return handle(PvAnchorPointHandle) not grub: -1 */
static int edit_anchor_point_handle_grub_focus_(PvFocus *focus, EtMouseAction mouse_action)
{
	int handle = -1; //!< Handle not grub.

	//! first check already focus AnchorPoint.
	const PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
	if(NULL != ap){
		handle = edit_anchor_point_handle_bound_handle_(ap, mouse_action);
		if(-1 != handle){
			return handle;
		}
	}

	//! change focus (to anchor points in already focus first element).
	const PvElementInfo *info = pv_element_get_info_from_kind(focus->elements[0]->kind);
	et_assertf(info, "%d", focus->elements[0]->kind);

	size_t num = info->func_get_num_anchor_point(focus->elements[0]);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap_ = info->func_get_anchor_point(focus->elements[0], i);
		handle = edit_anchor_point_handle_bound_handle_(ap_, mouse_action);
		if(-1 != handle){
			// ** change focus.
			pv_focus_clear_set_element_index(focus, focus->elements[0], i);
			return handle;
		}
	}

	return handle;
}

static bool translate_anchor_points_(EtDocId doc_id, EtMouseAction mouse_action)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	assert(focus);

	size_t num = pv_general_get_parray_num((void **)focus->anchor_points);
	for(int i = 0; i < (int)num; i++){
		//! @todo use element_kind.func of AnchorPoint
		// info->func_set_anchor_point_point(focus->anchor_points[i], ap, mouse_action.point);
		pv_anchor_point_move_point(focus->anchor_points[i], mouse_action.move);
	}

	return true;
}

static void edit_anchor_point_handle_down_(int *handle, PvFocus *focus, EtMouseAction mouse_action)
{
	*handle = edit_anchor_point_handle_grub_focus_(focus, mouse_action);
	PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
	if(NULL != ap && PvAnchorPointIndex_Point == *handle){
		pv_anchor_point_set_handle_zero(
				ap,
				PvAnchorPointIndex_Point);
	}
}

static void edit_anchor_point_handle_move_(int handle, PvFocus *focus, EtMouseAction mouse_action)
{
	if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
		return;
	}

	PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
	if(NULL == ap){
		return;
	}

	PvPoint p_ap = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_Point);
	PvPoint p_diff = pv_point_sub(p_ap, mouse_action.point);
	if(fabs(p_diff.x) < PX_SENSITIVE_OF_TOUCH
			&& fabs(p_diff.y) < PX_SENSITIVE_OF_TOUCH)
	{
		pv_anchor_point_set_handle_zero(
				ap,
				handle);
	}else{
		pv_anchor_point_set_handle(
				ap,
				handle,
				mouse_action.point);
	}
}

static bool func_edit_anchor_point_mouse_action_(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	static PvElement *touch_element_ = NULL;
	static PvAnchorPoint *touch_anchor_point_ = NULL;
	static bool is_already_focus_ = false;
	static bool is_move_ = false;
	static EtFocusElementMouseActionMode mode_ = EtFocusElementMouseActionMode_None;
	static EdgeKind mode_edge_ = EdgeKind_None;
	static int handle = -1;
	//	static PvRect src_extent_rect_;

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assert(focus);

	PvRect src_extent_rect = PvRect_Default;
	EdgeKind edge = EdgeKind_None;

	if(pv_focus_is_focused(focus)){
		src_extent_rect = get_rect_extent_from_elements_(focus->elements);
		edge = get_outside_touch_edge_kind_from_rect_(
				src_extent_rect, mouse_action.point, mouse_action.scale);
	}

	PvRect focusing_mouse_rect = {mouse_action.point.x, mouse_action.point.y, 0, 0};

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				touch_element_ = NULL;
				touch_anchor_point_ = NULL;
				is_already_focus_ = false;
				is_move_ = false;
				mode_edge_ = edge;
				handle = -1;
				//				src_extent_rect_ = src_extent_rect;

				get_touch_anchor_point_(
						&touch_element_,
						&touch_anchor_point_,
						doc_id,
						mouse_action.point);
				if(NULL != touch_anchor_point_){
					is_already_focus_ = pv_focus_is_exist_anchor_point(focus, touch_element_, touch_anchor_point_);
					et_assert(pv_focus_add_anchor_point(focus, touch_element_, touch_anchor_point_));

					mode_edge_ = EdgeKind_None;
					mode_ = EtFocusElementMouseActionMode_Translate;
					break;
				}

				switch(edge){
					case EdgeKind_Resize_UpLeft:
					case EdgeKind_Resize_UpRight:
					case EdgeKind_Resize_DownLeft:
					case EdgeKind_Resize_DownRight:
						{
							mode_ = EtFocusElementMouseActionMode_Resize;
						}
						break;
					case EdgeKind_Rotate_UpLeft:
					case EdgeKind_Rotate_UpRight:
					case EdgeKind_Rotate_DownLeft:
					case EdgeKind_Rotate_DownRight:
						{
							mode_ = EtFocusElementMouseActionMode_Rotate;
						}
						break;
					case EdgeKind_None:
					default:
						{
							edit_anchor_point_handle_down_(&handle, focus, mouse_action);
							if(-1 != handle){
								mode_ = EtFocusElementMouseActionMode_Handle;
							}else{
								et_assertf(pv_focus_clear_to_first_layer(focus), "%d", doc_id);
								mode_ = EtFocusElementMouseActionMode_FocusingByArea;
							}
						}
				}
			}
			break;
		case EtMouseAction_Move:
			{
				if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
					break;
				}

				if(!is_move_)
				{
					if( PX_SENSITIVE_OF_MOVE > abs(mouse_action.diff_down.x)
							&& PX_SENSITIVE_OF_MOVE > abs(mouse_action.diff_down.y))
					{
						break;
					}else{
						is_move_ = true;
						if(EtFocusElementMouseActionMode_Translate == mode_ && !is_already_focus_){
							pv_focus_clear_set_anchor_point(
									focus,
									touch_element_,
									touch_anchor_point_);
						}
					}
				}

				switch(mode_){
					case EtFocusElementMouseActionMode_Translate:
						{
							translate_anchor_points_(doc_id, mouse_action);
						}
						break;
					case EtFocusElementMouseActionMode_Resize:
						{
						}
						break;
					case EtFocusElementMouseActionMode_Rotate:
						{
						}
						break;
					case EtFocusElementMouseActionMode_FocusingByArea:
						{
							focusing_mouse_rect = focusing_by_area_(doc_id, mouse_action);
						}
						break;
					case EtFocusElementMouseActionMode_Handle:
						{
							edit_anchor_point_handle_move_(handle, focus, mouse_action);
						}
						break;
					default:
						break;
				}
			}
			break;
		case EtMouseAction_Up:
			{
				switch(mode_){
					case EtFocusElementMouseActionMode_Translate:
						{
							if(is_move_){
								// NOP
							}else{
								if(0 == (mouse_action.state & GDK_SHIFT_MASK)){
									et_assertf(pv_focus_clear_set_anchor_point(focus, touch_element_, touch_anchor_point_), "%d", doc_id);
								}else{
									if(!is_already_focus_){
										// NOP
									}else{
										et_assertf(pv_focus_remove_anchor_point(focus, touch_element_, touch_anchor_point_), "%d", doc_id);
									}
								}
							}
						}
						break;
					default:
						break;
				}

				mode_edge_ = EdgeKind_None;
				mode_ = EtFocusElementMouseActionMode_None;

				et_doc_save_from_id(doc_id);
			}
			break;
		case EtMouseAction_Unknown:
		default:
			et_bug("0x%x", mouse_action.action);
			break;
	}

	// ** mouse cursor
	if(EtFocusElementMouseActionMode_None == mode_){ // tool is not active
		*cursor = get_cursor_from_edge_(edge);
	}else{
		*cursor = get_cursor_from_edge_(mode_edge_);
	}

	// ** focusing view by tool //! @todo to AnchorPoint Edit
	*edit_draw_element = group_edit_(doc_id, mode_, focusing_mouse_rect, mouse_action.scale);

	return true;
}

static bool func_edit_anchor_point_handle_mouse_action_(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assert(focus);

	// static PvAnchorPointIndex handle = PvAnchorPointIndex_Point; //!< Handle not grub.
	static int handle = -1;
	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				edit_anchor_point_handle_down_(&handle, focus, mouse_action);
			}
			break;
		case EtMouseAction_Move:
			{
				edit_anchor_point_handle_move_(handle, focus, mouse_action);
			}
			break;
		case EtMouseAction_Up:
			{
				et_doc_save_from_id(doc_id);
			}
			break;
		case EtMouseAction_Unknown:
		default:
			et_bug("0x%x", mouse_action.action);
			break;
	}

	return true;
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

static bool add_basic_shape_element_down_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action)
{
	PvElement *parent_layer = pv_focus_get_first_layer(focus);
	et_assert(parent_layer);

	PvElement *element = pv_element_basic_shape_new_from_kind(PvBasicShapeKind_Rect);
	et_assert(element);

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	et_assert(info);

	PvRect rect = {
		mouse_action.point.x,
		mouse_action.point.y,
		1,
		1,
	};
	info->func_set_rect_by_anchor_points(element, rect);

	pv_element_append_child(parent_layer, NULL, element);

	pv_focus_clear_set_element(focus, element);

	return true;
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
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(doc, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	static EtFocusElementMouseActionMode mode_ = EtFocusElementMouseActionMode_None;
	static PvRect src_extent_rect_;

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				add_basic_shape_element_down_(doc, focus, mouse_action);
				src_extent_rect_ = get_rect_extent_from_elements_(focus->elements);
				mode_ = EtFocusElementMouseActionMode_Resize;
			}
			break;
		case EtMouseAction_Move:
			{
				if(EtFocusElementMouseActionMode_Resize == mode_){
					static EdgeKind mode_edge_ = EdgeKind_Resize_DownLeft;
					resize_elements_(doc_id, mouse_action, mode_edge_, src_extent_rect_);
				}
			}
			break;
		case EtMouseAction_Up:
			{
				PvElement *element = pv_focus_get_first_element(focus);
				et_assert(element);
				const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
				et_assert(info);
				info->func_apply_appearances(element, element->etaion_work_appearances);
				element->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

				mode_ = EtFocusElementMouseActionMode_None;

				et_doc_save_from_id(doc_id);
			}
			break;
		default:
			break;
	}

	return true;
}

static bool func_knife_anchor_point_mouse_action_(
		EtDocId doc_id,
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

