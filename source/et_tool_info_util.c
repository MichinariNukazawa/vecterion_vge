#include "et_tool_info_util.h"

#include <math.h>
#include "et_error.h"
#include "pv_element_general.h"
#include "pv_element.h"
#include "pv_element_info.h"
#include "pv_rotate.h"
#include "et_mouse_cursor_info.h"


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

PvRect get_rect_extent_from_elements_(PvElement **elements)
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

PvRect get_rect_extent_from_elements_apply_appearances_(PvElement **elements)
{
	PvRect rect_extent = PvRect_Default;
	size_t num = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < (int)num; i++){
		const PvElement *element = elements[i];
		const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
		et_assertf(info, "%d", element->kind);

		PvElement *simplify = pv_element_copy_recursive(element);
		info->func_apply_appearances(simplify, element->etaion_work_appearances);
		PvRect rect = info->func_get_rect_by_anchor_points(simplify);
		pv_element_free(simplify);

		if(0 == i){
			rect_extent = rect;
		}else{
			rect_extent = pv_rect_expand(rect_extent, rect);
		}
	}


	return rect_extent;
}

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

static PvElement *get_touch_element_(PvVg *vg, PvPoint g_point)
{
	et_assert(vg);

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

static PvRectEdgeKind get_rect_edge_kind_of_quadrant_(PvPoint point)
{
	if(point.x < 0){
		if(point.y < 0){
			return PvRectEdgeKind_UpLeft;
		}else{
			return PvRectEdgeKind_DownLeft;
		}
	}else{
		if(point.y < 0){
			return PvRectEdgeKind_UpRight;
		}else{
			return PvRectEdgeKind_DownRight;
		}
	}
}

static double round_(double v, double between)
{
	return round(v/between) * between;
}

static PvPoint get_snap_for_grid_point_(PvPoint src_point, const PvSnapContext *snap_context)
{
	et_assert(snap_context);

	PvPoint dst_point;
	dst_point.x = round_(src_point.x, snap_context->grid.x);
	dst_point.y = round_(src_point.y, snap_context->grid.y);

	return dst_point;
}

static PvPoint get_snap_for_degree_point_(PvPoint src_point, const PvSnapContext *snap_context)
{
	et_assert(snap_context);

	PvPoint dst_point;
	PvPoint w = {
		.x = ((0 > src_point.x)? -1 : 1),
		.y = ((0 > src_point.y)? -1 : 1),
	};
	double hypotenuse = sqrt((src_point.x * src_point.x) + (src_point.y * src_point.y));
	for(int i = 0; i < (int)snap_context->num_snap_for_degree; i++){
		double radian = get_radian_from_degree(snap_context->degrees[i]);
		PvPoint dst_point_;
		dst_point_.x = sin(radian) * hypotenuse * w.x;
		dst_point_.y = cos(radian) * hypotenuse * w.y;
		if(0 == i){
			dst_point = dst_point_;
		}else{
			if(pv_point_distance(src_point, dst_point_) < pv_point_distance(src_point, dst_point)){
				dst_point = dst_point_;
			}
		}
	}

	return dst_point;
}

PvPoint get_snap_move_from_point_(
		const PvSnapContext *snap_context,
		PvPoint src_point)
{
	et_assert(snap_context);

	PvPoint dst_point = get_snap_for_grid_point_(src_point, snap_context);
	PvPoint diff = pv_point_sub(dst_point, src_point);
	return diff;
}

static PvPoint get_snap_move_from_rect_edge_(
		PvRectEdgeKind rect_edge_kind,
		const PvSnapContext *snap_context,
		PvRect rect)
{
	et_assert(snap_context);

	PvPoint src_point = pv_rect_get_edge_point(rect, rect_edge_kind);
	PvPoint diff = get_snap_move_from_point_(snap_context, src_point);
	return diff;
}

void translate_elements_inline_(PvFocus *focus, PvPoint move)
{
	et_assert(focus);

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

/*! @return is move */
static void translate_elements_(
		PvFocus *focus,
		const PvSnapContext *snap_context,
		EtMouseAction mouse_action)
{
	et_assert(focus);
	et_assert(snap_context);

	PvPoint move = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

	if(snap_context->is_snap_for_degree){
		move = get_snap_for_degree_point_(move, snap_context);
	}
	if(snap_context->is_snap_for_grid){
		PvRectEdgeKind rect_edge_kind = get_rect_edge_kind_of_quadrant_(mouse_action.diff_down);
		PvRect extent_rect = get_rect_extent_from_elements_(focus->elements);
		extent_rect = pv_rect_add_point(extent_rect, move);
		PvPoint snap_move_ = get_snap_move_from_rect_edge_(rect_edge_kind, snap_context, extent_rect);
		move = pv_point_add(move, snap_move_);
	}

	translate_elements_inline_(focus, move);

	return;
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

static PvPoint pv_resize_diff_(PvPoint size, PvPoint resize)
{
	PvPoint diff_ = {
		.x = (size.x - (size.x * resize.x)),
		.y = (size.y - (size.y * resize.y)),
	};

	return diff_;
}

PvRectEdgeKind get_rect_edge_kind_(EdgeKind edge_kind)
{
	switch(edge_kind){
		case EdgeKind_Resize_UpRight:
			return PvRectEdgeKind_UpRight;
		case EdgeKind_Resize_UpLeft:
			return PvRectEdgeKind_UpLeft;
		case EdgeKind_Resize_DownRight:
			return PvRectEdgeKind_DownRight;
		case EdgeKind_Resize_DownLeft:
			return PvRectEdgeKind_DownLeft;
		default:
			et_abortf("%d", edge_kind);
	}
}

EdgeKind resize_elements_(
		PvElement **elements,
		const PvSnapContext *snap_context,
		EtMouseAction mouse_action,
		EdgeKind src_edge_kind_,
		PvRect src_extent_rect)
{
	EdgeKind dst_edge_kind = src_edge_kind_;


	PvPoint move = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

	if(snap_context->is_snap_for_grid){
		PvRectEdgeKind rect_edge_kind = get_rect_edge_kind_(dst_edge_kind);
		PvRect extent_rect = get_rect_extent_from_elements_(elements);
		extent_rect = pv_rect_add_point(extent_rect, move);
		PvPoint snap_move_ = get_snap_move_from_rect_edge_(rect_edge_kind, snap_context, extent_rect);
		move = pv_point_add(move, snap_move_);
	}

	PvPoint move_upleft;
	PvPoint resize;
	get_resize_in_rect_(&move_upleft, &resize, dst_edge_kind, src_extent_rect, move);

	//! @todo delta needed?
	/*
	   const double DELTA_OF_RESIZE = 0.001;
	   resize.x = ((fabs(resize.x) > DELTA_OF_RESIZE) ? resize.x : DELTA_OF_RESIZE);
	   resize.y = ((fabs(resize.y) > DELTA_OF_RESIZE) ? resize.y : DELTA_OF_RESIZE);
	 */

	size_t num = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < (int)num; i++){
		PvElement *element = elements[i];
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
		PvFocus *focus,
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
		PvFocus *focus,
		PvVg *vg,
		EtMouseAction mouse_action)
{
	pv_focus_clear_to_first_layer(focus);

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

static PvRect get_square_rect_from_center_point_(PvPoint center, double size)
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

static PvElement *group_edit_(
		PvFocus *focus,
		EtFocusElementMouseActionMode mode_,
		PvRect focusing_mouse_rect,
		double scale)
{
	et_assert(focus);

	PvElement *element_group_edit_draw = pv_element_new(PvElementKind_Group);
	et_assert(element_group_edit_draw);

	PvRect after_extent_rect_ = get_rect_extent_from_elements_apply_appearances_(focus->elements);
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

bool et_tool_info_util_func_edit_element_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{

	static PvElement *touch_element_ = NULL;
	static bool is_already_focus_ = false;
	static bool is_move_ = false;
	static EtFocusElementMouseActionMode mode_ = EtFocusElementMouseActionMode_None;
	static EdgeKind mode_edge_ = EdgeKind_None;
	static PvRect src_extent_rect_when_down_;

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
				src_extent_rect_when_down_ = src_extent_rect;

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

							touch_element_ = get_touch_element_(vg, mouse_action.point);

							if(NULL == touch_element_){
								et_assert(pv_focus_clear_to_first_layer(focus));
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
		case EtMouseAction_Up:
			{
				if(EtMouseAction_Move == mouse_action.action){
					if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
						break;
					}
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
							translate_elements_(
									focus,
									snap_context,
									mouse_action);
						}
						break;
					case EtFocusElementMouseActionMode_Resize:
						{
							resize_elements_(
									focus->elements,
									snap_context,
									mouse_action,
									mode_edge_,
									src_extent_rect_when_down_);
						}
						break;
					case EtFocusElementMouseActionMode_Rotate:
						{
							rotate_elements_(focus, mouse_action, mode_edge_, src_extent_rect_when_down_);
						}
						break;
					case EtFocusElementMouseActionMode_FocusingByArea:
						{
							focusing_mouse_rect = focusing_by_area_(focus, vg, mouse_action);
						}
						break;
					default:
						break;
				}

				if(EtMouseAction_Up == mouse_action.action){
					switch(mode_){
						case EtFocusElementMouseActionMode_Translate:
							{
								if(is_move_){
									// NOP
								}else{
									if(0 == (mouse_action.state & GDK_SHIFT_MASK)){
										pv_focus_clear_set_element(focus, touch_element_);
									}else{
										if(!is_already_focus_){
											// NOP
										}else{
											pv_focus_remove_element(focus, touch_element_);
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

					*is_save = true;
				}
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
	*edit_draw_element = group_edit_(focus, mode_, focusing_mouse_rect, mouse_action.scale);

	return true;
}

bool is_bound_point_(int radius, PvPoint p1, PvPoint p2)
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

static void get_touch_anchor_point_(
		PvElement **p_element,
		PvAnchorPoint **p_anchor_point,
		PvVg *vg,
		PvPoint point)
{

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
				p_point,
				mouse_action.point))
	{
		return PvAnchorPointIndex_Point;
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
				p_prev,
				mouse_action.point))
	{
		return PvAnchorPointIndex_HandlePrev;
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

static void edit_anchor_point_handle_move_(
		int handle,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		EtMouseAction mouse_action)
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
		PvPoint point = mouse_action.point;
		if(snap_context->is_snap_for_grid){
			point = get_snap_for_grid_point_(point, snap_context);
		}

		pv_anchor_point_set_handle(
				ap,
				handle,
				point);
	}
}

static bool translate_anchor_points_(
		PvFocus *focus,
		const PvSnapContext *snap_context,
		PvPoint src_point,
		EtMouseAction mouse_action)
{
	assert(focus);
	et_assert(snap_context);

	size_t num = pv_general_get_parray_num((void **)focus->anchor_points);
	if(0 == num){
		return true;
	}

	PvPoint move = mouse_action.move;
	if(snap_context->is_snap_for_degree){
		PvPoint move_v = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);
		move_v = get_snap_for_degree_point_(move_v, snap_context);
		PvPoint dst_point = pv_point_add(src_point, move_v);

		PvPoint current_ap_point = pv_anchor_point_get_point(focus->anchor_points[0]);
		move = pv_point_sub(dst_point, current_ap_point);
	}
	if(snap_context->is_snap_for_grid){
		PvPoint move_v = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);
		src_point = pv_point_add(src_point, move_v);
		PvPoint dst_point = get_snap_for_grid_point_(src_point, snap_context);

		PvPoint current_ap_point = pv_anchor_point_get_point(focus->anchor_points[0]);
		move = pv_point_sub(dst_point, current_ap_point);
	}

	for(int i = 0; i < (int)num; i++){
		//! @todo use element_kind.func of AnchorPoint
		// info->func_set_anchor_point_point(focus->anchor_points[i], ap, mouse_action.point);
		pv_anchor_point_move_point(focus->anchor_points[i], move);
	}

	return true;
}

bool et_tool_info_util_func_edit_anchor_point_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
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
	static PvPoint touched_ap_point;

	//	static PvRect src_extent_rect_when_down_;

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
				//				src_extent_rect_when_down_ = src_extent_rect;

				get_touch_anchor_point_(
						&touch_element_,
						&touch_anchor_point_,
						vg,
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
								et_assert(pv_focus_clear_to_first_layer(focus));
								mode_ = EtFocusElementMouseActionMode_FocusingByArea;
							}
						}
				}
			}
			break;
		case EtMouseAction_Move:
		case EtMouseAction_Up:
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

						size_t num = pv_general_get_parray_num((void **)focus->anchor_points);
						if(0 < num){
							touched_ap_point = pv_anchor_point_get_point(focus->anchor_points[0]);
						}
					}
				}

				switch(mode_){
					case EtFocusElementMouseActionMode_Translate:
						{
							translate_anchor_points_(
									focus,
									snap_context,
									touched_ap_point,
									mouse_action);
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
							focusing_mouse_rect = focusing_by_area_(focus, vg, mouse_action);
						}
						break;
					case EtFocusElementMouseActionMode_Handle:
						{
							edit_anchor_point_handle_move_(
									handle,
									focus,
									snap_context,
									mouse_action);
						}
						break;
					default:
						break;
				}
				if(EtMouseAction_Up == mouse_action.action){
					switch(mode_){
						case EtFocusElementMouseActionMode_Translate:
							{
								if(is_move_){
									// NOP
								}else{
									if(0 == (mouse_action.state & GDK_SHIFT_MASK)){
										et_assert(pv_focus_clear_set_anchor_point(focus, touch_element_, touch_anchor_point_));
									}else{
										if(!is_already_focus_){
											// NOP
										}else{
											et_assert(pv_focus_remove_anchor_point(focus, touch_element_, touch_anchor_point_));
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

					*is_save = true;
				}
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
	*edit_draw_element = group_edit_(focus, mode_, focusing_mouse_rect, mouse_action.scale);

	return true;
}

bool et_tool_info_util_func_edit_anchor_point_handle_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor)
{
	// static PvAnchorPointIndex handle = PvAnchorPointIndex_Point; //!< Handle not grub.
	static int handle = -1;
	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				edit_anchor_point_handle_down_(&handle, focus, mouse_action);
			}
			break;
		case EtMouseAction_Move:
		case EtMouseAction_Up:
			{
				edit_anchor_point_handle_move_(
						handle,
						focus,
						snap_context,
						mouse_action);

				if(EtMouseAction_Up == mouse_action.action){
					*is_save = true;
				}
			}
			break;
		case EtMouseAction_Unknown:
		default:
			et_bug("0x%x", mouse_action.action);
			break;
	}

	return true;
}

static void add_anchor_point_down_(
		PvFocus *focus,
		const PvSnapContext *snap_context,
		EtMouseAction mouse_action,
		bool *is_reverse,
		PvColorPair color_pair,
		PvStroke stroke
		)
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

	PvPoint point = mouse_action.point;
	if(snap_context->is_snap_for_grid){
		point = get_snap_for_grid_point_(point, snap_context);
	}

	if(PvElementKind_Curve != element_->kind
			|| NULL == anchor_point
			|| (!((0 == index) || index == ((int)num - 1)))
			|| pv_anchor_path_get_is_close(data_->anchor_path)){
		// add new ElementCurve.

		anchor_point = pv_anchor_point_new_from_point(point);
		et_assert(anchor_point);
		PvElement *new_element = pv_element_curve_new_set_anchor_point(anchor_point);
		et_assert(new_element);
		pv_element_append_on_focusing(element_, new_element);

		new_element->color_pair = color_pair;
		new_element->stroke = stroke;
		/*
		   new_element->color_pair = et_color_panel_get_color_pair();
		   new_element->stroke = et_stroke_panel_get_stroke();
		 */

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
			anchor_point = pv_anchor_point_new_from_point(point);
			et_assert(anchor_point);

			pv_element_curve_append_anchor_point(element_, anchor_point, index);
		}
	}

	pv_focus_clear_set_anchor_point(focus, element_, anchor_point);
}

static bool focused_anchor_point_move_(
		PvFocus *focus,
		const PvSnapContext *snap_context,
		EtMouseAction mouse_action,
		bool is_reverse)
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
		PvPoint point = mouse_action.point;
		if(snap_context->is_snap_for_grid){
			point = get_snap_for_grid_point_(point, snap_context);
		}

		pv_anchor_point_set_handle(ap, PvAnchorPointIndex_Point, point);

		if(is_reverse){
			// AnchorPoint is head in AnchorPath
			pv_anchor_point_reverse_handle(ap);
		}
	}

	return true;
}

bool et_tool_info_util_func_add_anchor_point_handle_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor,
		PvColorPair color_pair,
		PvStroke stroke)
{
	static bool is_reverse;

	bool result = true;

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				add_anchor_point_down_(
						focus,
						snap_context,
						mouse_action,
						&is_reverse,
						color_pair,
						stroke
						);
			}
			break;
		case EtMouseAction_Move:
		case EtMouseAction_Up:
			{
				result = focused_anchor_point_move_(
						focus,
						snap_context,
						mouse_action,
						is_reverse);

				if(EtMouseAction_Up == mouse_action.action){
					*is_save = true;
				}
			}
			break;
		case EtMouseAction_Unknown:
		default:
			et_bug("0x%x", mouse_action.action);
			break;
	}

	return result;
}

static PvElement *add_basic_shape_element_down_(
		PvFocus *focus,
		const PvSnapContext *snap_context,
		EtMouseAction mouse_action)
{
	PvElement *parent_layer = pv_focus_get_first_layer(focus);
	et_assert(parent_layer);

	PvElement *element = pv_element_basic_shape_new_from_kind(PvBasicShapeKind_Rect);
	et_assert(element);

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	et_assert(info);

	PvPoint point = mouse_action.point;
	if(snap_context->is_snap_for_grid){
		point = get_snap_for_grid_point_(point, snap_context);
	}

	PvRect rect = {
		point.x,
		point.y,
		1,
		1,
	};
	info->func_set_rect_by_anchor_points(element, rect);

	pv_element_append_child(parent_layer, NULL, element);

	pv_focus_clear_set_element(focus, element);

	return element;
}

bool et_tool_info_util_func_add_basic_shape_element_mouse_action(
		PvVg *vg,
		PvFocus *focus,
		const PvSnapContext *snap_context,
		bool *is_save,
		EtMouseAction mouse_action,
		PvElement **edit_draw_element,
		GdkCursor **cursor,
		PvColorPair color_pair,
		PvStroke stroke
		)
{
	static EtFocusElementMouseActionMode mode_ = EtFocusElementMouseActionMode_None;
	static PvRect src_extent_rect_when_down_;

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				PvElement *element = add_basic_shape_element_down_(
						focus,
						snap_context,
						mouse_action);
				element->color_pair = color_pair;
				element->stroke = stroke;

				src_extent_rect_when_down_ = get_rect_extent_from_elements_(focus->elements);
				mode_ = EtFocusElementMouseActionMode_Resize;
			}
			break;
		case EtMouseAction_Move:
		case EtMouseAction_Up:
			{
				if(EtFocusElementMouseActionMode_Resize == mode_){
					static EdgeKind mode_edge_ = EdgeKind_Resize_DownRight;
					resize_elements_(
							focus->elements,
							snap_context,
							mouse_action,
							mode_edge_,
							src_extent_rect_when_down_);
				}

				if(EtMouseAction_Up == mouse_action.action){
					PvElement *element = pv_focus_get_first_element(focus);
					et_assert(element);
					const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
					et_assert(info);
					info->func_apply_appearances(element, element->etaion_work_appearances);
					element->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

					mode_ = EtFocusElementMouseActionMode_None;

					*is_save = true;
				}
			}
			break;
		default:
			break;
	}

	return true;
}

