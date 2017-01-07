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
#include "et_etaion.h"
#include "et_color_panel.h"
#include "et_stroke_panel.h"
#include "et_mouse_cursor.h"


bool _et_tool_info_is_init = false; // check initialized tools

EtToolInfo _et_tool_infos[];


static int PX_SENSITIVE_OF_TOUCH = 16;

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

static EdgeKind _get_outside_touch_edge_kind_from_rect(PvRect rect, PvPoint point, double scale);


static EtToolInfo *_et_tool_get_info_from_id(EtToolId tool_id);

static EdgeKind _get_outside_touch_edge_kind_from_rect(PvRect rect, PvPoint point, double scale)
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

static GdkPixbuf *_conv_new_icon_focus(GdkPixbuf *pb_src)
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

bool et_tool_info_init()
{
	et_assert(false == _et_tool_info_is_init);

	int num_tool = et_tool_get_num();
	for(int tool_id = 0; tool_id < num_tool; tool_id++){
		EtToolInfo *info = _et_tool_get_info_from_id(tool_id);
		et_assertf(info, "%d", tool_id);

		// ** make image(cursor,icons)
		GError *error = NULL;
		info->icon_cursor = gdk_pixbuf_new_from_file(info->filepath_cursor, &error);
		et_assertf(info->icon_cursor, "%d, %s", tool_id, error->message);

		info->icon = info->icon_cursor;
		info->icon_focus = _conv_new_icon_focus(info->icon);

		// ** mouse cursor
		assert(gdk_display_get_default());
		info->mouse_cursor = gdk_cursor_new_from_pixbuf(
				gdk_display_get_default(),
				info->icon_cursor,
				0, 0);
		et_assertf(info->mouse_cursor, "%d", tool_id);
	}

	_et_tool_info_is_init = true;

	et_debug("initialized EtToolInfo num:%d", num_tool);

	return true;
}



static bool _is_bound_point(int radius, PvPoint p1, PvPoint p2)
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

static PvRect _get_rect_extent_from_elements(PvElement **elements)
{
	PvRect rect_extent = PvRect_Default;
	int num = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < num; i++){
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
	int *p_index;
	PvPoint g_point;
}RecursiveDataGetFocus;

static bool _func_recursive_get_touch_element(PvElement *element, gpointer data, int level)
{
	RecursiveDataGetFocus *_data = data;
	et_assert(_data);

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	et_assertf(info, "%d", element->kind);

	bool is_touch = false;
	bool ret = info->func_is_touch_element(
			&is_touch,
			element,
			PX_SENSITIVE_OF_TOUCH,
			_data->g_point.x,
			_data->g_point.y);
	if(!ret){
		et_bug("");
		return false;
	}
	if(is_touch){
		// ** detect is stop search.
		*(_data->p_element) = element;
		return false;
	}

	return true;
}

static PvElement *_get_touch_element(EtDocId doc_id, PvPoint g_point)
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
				_func_recursive_get_touch_element,
				NULL,
				&rec_data,
				&error)){
		et_assertf(false, "level:%d", error.level);
	}

	return element;
}

/*! @return is move */
static void _translate_elements(
		EtDocId doc_id,
		EtMouseAction mouse_action)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvPoint move = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
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

static EdgeKind _resize_elements(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		EdgeKind _src_edge_kind,
		PvRect src_extent_rect)
{
	EdgeKind dst_edge_kind = _src_edge_kind;

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvPoint move = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

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
			et_assert(false);
			break;
	}

	PvPoint resize = {
		.x = size_after.x / src_extent_rect.w,
		.y = size_after.y / src_extent_rect.h,
	};

	//! @todo delta needed?
	/*
	   const double DELTA_OF_RESIZE = 0.001;
	   resize.x = ((fabs(resize.x) > DELTA_OF_RESIZE) ? resize.x : DELTA_OF_RESIZE);
	   resize.y = ((fabs(resize.y) > DELTA_OF_RESIZE) ? resize.y : DELTA_OF_RESIZE);
	 */

	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
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

static double _get_degree_from_radian(double radian)
{
	return radian * (180.0 / M_PI);
	// return radian * 180.0 / (atan(1.0) * 4.0);
}

static double get_degree_from_point(PvPoint point)
{
	double radian = atan2(point.y, point.x);
	double degree = _get_degree_from_radian(radian);
	return degree;
}

static EdgeKind _rotate_elements(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		EdgeKind _src_edge_kind,
		PvRect src_extent_rect)
{
	EdgeKind dst_edge_kind = _src_edge_kind;

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
	int num_ = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < num_; i++){
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

static bool _recursive_inline_focusing_by_area(PvElement *element, gpointer data, int level)
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
	}

	return true;
}

static PvRect _focusing_by_area(
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
			_recursive_inline_focusing_by_area,
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
}EtFocusElementMouseActionMode;

static GdkCursor *_get_cursor_from_edge(EdgeKind edge)
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

static PvElement *_element_new_from_circle(PvPoint center, double size)
{
	PvRect rect = {
		center.x - (size / 2),
		center.y - (size / 2),
		size,
		size,
	};

	return pv_element_curve_new_from_rect(rect);
}

static void group_edit_(EtDocId doc_id, EtFocusElementMouseActionMode _mode, PvRect focusing_mouse_rect, double scale)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvElement *element_group_edit_draw = pv_element_new(PvElementKind_Group);
	et_assert(element_group_edit_draw);

	PvRect _after_extent_rect = _get_rect_extent_from_elements(focus->elements);
	if(pv_focus_is_focused(focus)){
		int size = 5.0 / scale;
		for(int i = 0; i < 4; i++){
			PvPoint center = pv_rect_get_edge_point(_after_extent_rect, i);
			PvRect rect = get_square_rect_from_center_point_(center, size);
			PvElement *element_curve = pv_element_curve_new_from_rect(rect);
			et_assert(element_curve);
			pv_element_append_child(element_group_edit_draw, NULL, element_curve);
		}
	}

	switch(_mode){
		case EtFocusElementMouseActionMode_Rotate:
			{
				if(pv_focus_is_focused(focus)){
					// rotate center mark
					PvPoint center = pv_rect_get_center(_after_extent_rect);
					int diameter = 8.0 / scale;
					PvElement *element_center = _element_new_from_circle(center, diameter);
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


	et_doc_set_element_group_edit_draw_from_id(doc_id, element_group_edit_draw);
	pv_element_remove_free_recursive(element_group_edit_draw);
}

static bool _func_edit_element_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
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
		src_extent_rect = _get_rect_extent_from_elements(focus->elements);
		edge = _get_outside_touch_edge_kind_from_rect(
				src_extent_rect, mouse_action.point, mouse_action.scale);
	}

	PvRect focusing_mouse_rect;

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

							touch_element_ = _get_touch_element(doc_id, mouse_action.point);

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
					const int PX_SENSITIVE_OF_MOVE = 3;

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
							_translate_elements(doc_id, mouse_action);
						}
						break;
					case EtFocusElementMouseActionMode_Resize:
						{

							_resize_elements(doc_id, mouse_action, mode_edge_, src_extent_rect_);
						}
						break;
					case EtFocusElementMouseActionMode_Rotate:
						{

							_rotate_elements(doc_id, mouse_action, mode_edge_, src_extent_rect_);
						}
						break;
					case EtFocusElementMouseActionMode_FocusingByArea:
						{
							focusing_mouse_rect = _focusing_by_area(doc_id, mouse_action);
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
				et_doc_save_from_id(doc_id);

				mode_ = EtFocusElementMouseActionMode_None;
			}
			break;
		default:
			break;
	}

	// ** mouse cursor
	if(EtFocusElementMouseActionMode_None == mode_){ // tool is not active
		*cursor = _get_cursor_from_edge(edge);
	}else{
		*cursor = _get_cursor_from_edge(mode_edge_);
	}

	// ** focusing view by tool
	group_edit_(doc_id, mode_, focusing_mouse_rect, mouse_action.scale);

	return true;
}

/** @return new AnchorPoint index number. error:-1 */
static int _et_tool_curve_add_anchor_point(EtDoc *doc, PvElement **_element, double x, double y)
{
	bool is_new = true;
	PvElement *element = *_element;
	PvElement *parent = NULL;
	if(NULL == element){
		parent = NULL;
		//element = NULL;
	}else{
		switch(element->kind){
			case PvElementKind_Curve:
				parent = element->parent;
				//element = element;
				is_new = false;
				break;
			case PvElementKind_Layer:
			case PvElementKind_Group:
				parent = *_element;
				element = NULL;
				break;
			default:
				parent = element->parent;
				element = NULL;
		}
	}

	if(is_new){
		et_doc_save_from_id(et_doc_get_id(doc));

		element = pv_element_new(PvElementKind_Curve);
		if(NULL == element){
			et_error("");
			return -1;
		}

		element->color_pair = et_color_panel_get_color_pair();
		element->stroke = et_stroke_panel_get_stroke();
	}

	PvAnchorPoint anchor_point = pv_anchor_point_from_point((PvPoint){x, y,});
	if(!pv_element_curve_add_anchor_point(element, anchor_point)){
		et_error("");
		return -1;
	}

	if(is_new){
		if(NULL == parent){
			parent = pv_vg_get_layer_top(et_doc_get_vg_ref(doc));
			if(NULL == parent){
				et_error("");
				return -1;
			}
		}

		if(!pv_element_append_child(parent, 
					NULL, element)){
			et_error("");
			return -1;
		}
	}

	*_element = element;

	return (pv_element_curve_get_num_anchor_point(*_element) - 1);
}

static void add_anchor_point_down_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action)
{
	PvElement *_element = pv_focus_get_first_element(focus);

	bool is_closed = false;
	if(NULL != _element && PvElementKind_Curve == _element->kind){
		PvElementCurveData *_data = (PvElementCurveData *) _element->data;
		et_assert(_data);
		if(pv_anchor_path_get_is_close(_data->anchor_path)){
			// if already closed is goto new anchor_point
			_element = NULL;
		}else{
			if(0 < pv_anchor_path_get_anchor_point_num(_data->anchor_path)){
				const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(_data->anchor_path, 0, PvAnchorPathIndexTurn_Disable);
				if(_is_bound_point(
							PX_SENSITIVE_OF_TOUCH,
							ap->points[PvAnchorPointIndex_Point],
							mouse_action.point)
				  ){
					// ** do close anchor_point
					is_closed = true;
					pv_anchor_path_set_is_close(_data->anchor_path, is_closed);
					bool ret = pv_focus_clear_set_element_index(focus, _element, 0);
					et_assert(ret);
				}
			}
		}
	}

	// ** new anchor_point
	if(!is_closed){
		int index = _et_tool_curve_add_anchor_point(
				doc, &_element,
				mouse_action.point.x, mouse_action.point.y);
		et_assert(0 <= index);
		pv_focus_clear_set_element_index(focus, _element, index);
	}
}

static bool focused_anchor_point_move_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action)
{
	if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
		return true;
	}

	PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
	if(NULL == ap){
		return true;
	}

	PvElement *_element = pv_focus_get_first_element(focus);
	et_assert(_element);
	if(PvElementKind_Curve != _element->kind){
		et_error("");
		return false;
	}

	PvElementCurveData *_data = (PvElementCurveData *) _element->data;
	et_assert(_data);

	PvPoint p_ap = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_Point);
	PvPoint p_diff = pv_point_sub(p_ap, mouse_action.point);
	if(fabs(p_diff.x) < PX_SENSITIVE_OF_TOUCH && fabs(p_diff.y) < PX_SENSITIVE_OF_TOUCH){
		pv_anchor_point_set_handle_zero(ap, PvAnchorPointIndex_Point);
	}else{
		pv_anchor_point_set_handle(ap, PvAnchorPointIndex_Point, mouse_action.point);
	}

	return true;
}

static bool _func_add_anchor_point_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	et_assertf(doc, "%d", doc_id);
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	bool result = true;

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				add_anchor_point_down_(doc, focus, mouse_action);
			}
			break;
		case EtMouseAction_Up:
			break;
		case EtMouseAction_Move:
			{
				result = focused_anchor_point_move_(doc, focus, mouse_action);
			}
			break;
		case EtMouseAction_Unknown:
		default:
			et_bug("0x%x", mouse_action.action);
			break;
	}

	et_doc_set_element_group_edit_draw_from_id(doc_id, NULL);

	return result;
}

static bool _get_touch_element_and_index(
		PvElement *element, gpointer data_0, int level)
{
	RecursiveDataGetFocus *_data = data_0;

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	assert(info);

	int index = -1;

	size_t num = info->func_get_num_anchor_point(element);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint ap = info->func_get_anchor_point(element, i);

		if(_is_bound_point(
					PX_SENSITIVE_OF_TOUCH,
					ap.points[PvAnchorPointIndex_Point],
					_data->g_point))
		{
			index = i;
		}
	}

	if(0 <= index){
		*(_data->p_index) = index;
		*(_data->p_element) = element;
		return false;
	}

	return true;
}

void _et_tool_focus_element_mouse_action_get_touch_anchor_point(
		PvElement **_element,
		int *p_index,
		EtDocId doc_id,
		PvPoint point)
{
	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		et_error("");
		goto error;
	}

	RecursiveDataGetFocus rec_data = {
		.p_element = _element,
		.p_index = p_index,
		.g_point = point,
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_asc(vg->element_root,
				_get_touch_element_and_index,
				NULL,
				&rec_data,
				&error)){
		et_error("level:%d", error.level);
		goto error;
	}

	return;
error:
	return;
}

static bool _move_elements_anchor_points(EtDocId doc_id, EtMouseAction mouse_action)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	assert(focus);

	/*! current focusing AnchorPoint (0 == index) direct set mouse position.
	 * else sub focusing AnchorPoints position is move difference
	 *		from current focusing AnchorPoint
	 */
	PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
	if(NULL == ap){
		return true;
	}
	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
		const PvElementInfo *info = pv_element_get_info_from_kind(focus->elements[i]->kind);
		assert(info);
		if(info->func_is_exist_anchor_point(focus->elements[i], ap)){

			info->func_set_anchor_point_point(focus->elements[i], ap, mouse_action.point);
			return true;
		}
	}

	return true;
}

static bool _func_edit_anchor_point_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assert(focus);

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				int index = -1;
				PvElement *_element = NULL;
				_et_tool_focus_element_mouse_action_get_touch_anchor_point(
						&_element,
						&index,
						doc_id,
						mouse_action.point);
				if(index < 0){
					pv_focus_clear_to_first_layer(focus);
				}else{
					pv_focus_clear_set_element_index(focus, _element, index);
				}
			}
			break;
		case EtMouseAction_Move:
			{
				if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
					break;
				}

				if(!_move_elements_anchor_points(doc_id, mouse_action)){
					et_error("");
					break;
				}
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

	et_doc_set_element_group_edit_draw_from_id(doc_id, NULL);

	return true;
}

/*! @return handle(PvAnchorPointHandle) not grub: -1 */
int _edit_anchor_point_handle_bound_handle(PvAnchorPoint ap, EtMouseAction mouse_action)
{
	// ** grub handle.
	PvPoint p_point = pv_anchor_point_get_handle(&ap, PvAnchorPointIndex_Point);
	PvPoint p_prev = pv_anchor_point_get_handle(&ap, PvAnchorPointIndex_HandlePrev);
	PvPoint p_next = pv_anchor_point_get_handle(&ap, PvAnchorPointIndex_HandleNext);

	if(_is_bound_point(
				PX_SENSITIVE_OF_TOUCH,
				p_prev,
				mouse_action.point))
	{
		return PvAnchorPointIndex_HandlePrev;
	}
	if(_is_bound_point(
				PX_SENSITIVE_OF_TOUCH,
				p_next,
				mouse_action.point))
	{
		return PvAnchorPointIndex_HandleNext;
	}
	if(_is_bound_point(
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
static int _edit_anchor_point_handle_grub_focus(PvFocus *focus, EtMouseAction mouse_action)
{
	int handle = -1; //!< Handle not grub.

	//! first check already focus AnchorPoint.
	PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
	if(NULL != ap){
		handle = _edit_anchor_point_handle_bound_handle(*ap, mouse_action);
		if(-1 != handle){
			return handle;
		}
	}

	//! change focus (to anchor points in already focus first element).
	const PvElementInfo *info = pv_element_get_info_from_kind(focus->elements[0]->kind);
	et_assertf(info, "%d", focus->elements[0]->kind);

	int num = info->func_get_num_anchor_point(focus->elements[0]);
	for(int i = 0; i < num; i++){
		const PvAnchorPoint ap_ = info->func_get_anchor_point(focus->elements[0], i);
		handle = _edit_anchor_point_handle_bound_handle(ap_, mouse_action);
		if(-1 != handle){
			// ** change focus.
			pv_focus_clear_set_element_index(focus, focus->elements[0], i);
			return handle;
		}
	}

	return handle;
}

static bool _func_edit_anchor_point_handle_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assert(focus);

	// static PvAnchorPointIndex handle = PvAnchorPointIndex_Point; //!< Handle not grub.
	static int handle = -1;
	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				handle = _edit_anchor_point_handle_grub_focus(focus, mouse_action);
				PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
				if(NULL != ap && PvAnchorPointIndex_Point == handle){
					pv_anchor_point_set_handle_zero(
							ap,
							PvAnchorPointIndex_Point);
				}
			}
			break;
		case EtMouseAction_Move:
			{
				if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
					break;
				}

				PvAnchorPoint *ap = pv_focus_get_first_anchor_point(focus);
				if(NULL == ap){
					break;
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

	et_doc_set_element_group_edit_draw_from_id(doc_id, NULL);

	return true;
}

static void curve_element_split_from_index_(PvElement *elements[2], PvElement *element, int index)
{
	et_assertf(PvElementKind_Curve == element->kind, "%d", element->kind);

	PvElementCurveData *data = element->data;
	PvAnchorPath *anchor_path = data->anchor_path;
	if(pv_anchor_path_get_is_close(anchor_path)){
		bool ret = pv_anchor_path_split_anchor_point_from_index(anchor_path, index);
		et_assert(ret);

		elements[0] = NULL;
		elements[1] = NULL;
	}else{
		PvElement *head_element = pv_element_curve_copy_new_range(element, 0, index);
		et_assert(head_element);

		size_t num = pv_anchor_path_get_anchor_point_num(anchor_path);
		PvElement *foot_element = pv_element_curve_copy_new_range(element, index, ((int)num - 1));
		et_assert(foot_element);

		elements[0] = head_element;
		elements[1] = foot_element;
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
	int num = info->func_get_num_anchor_point(element);
	for(int i = 0; i < num; i++){
		const PvAnchorPoint ap = info->func_get_anchor_point(element, i);
		if(_is_bound_point(
					PX_SENSITIVE_OF_TOUCH,
					ap.points[PvAnchorPointIndex_Point],
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

	PvElement *elements[2] = {NULL, NULL,};
	curve_element_split_from_index_(elements, element, index);
	if(NULL != elements[0] && NULL != elements[1]){
		pv_focus_clear_to_first_layer(focus);
		PvElement *parent_layer = pv_focus_get_first_layer(focus);
		pv_element_append_child(parent_layer, element, elements[0]);
		pv_element_append_child(parent_layer, element, elements[1]);
		pv_element_remove_free_recursive(element);
		pv_focus_add_element(focus, elements[0]);
		pv_focus_add_element(focus, elements[1]);
	}

	return true;
}

static bool _func_knife_anchor_point_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
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

void pv_anchor_path_get_subdivide_anchor_ponts_form_percent(
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

int element_curve_insert_anchor_point_from_index_percent_(
		PvElement *element, int index, double percent)
{
	PvElementCurveData *data = (PvElementCurveData *)element->data;
	et_assert(data);

	PvAnchorPoint dst_aps[3];
	pv_anchor_path_get_subdivide_anchor_ponts_form_percent(dst_aps, data->anchor_path, index, percent);
	PvAnchorPoint *ap = pv_anchor_path_insert_anchor_point(data->anchor_path, &dst_aps[1], index);
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
	   int new_index = pv_anchor_path_insert_anchor_point(data->anchor_path, ap, index);
	 */
	// @todo handle.

	et_debug("%d", new_index);
	return new_index;
}

static int insert_anchor_point_down_(EtDoc *doc, PvFocus *focus, EtMouseAction mouse_action)
{
	size_t num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num; i++){
		if(PvElementKind_Curve != focus->elements[i]->kind){
			continue;
		}

		int percent = -1;
		int index = -1;
		element_curve_get_nearest_index_percent_(
				&index, &percent,
				PX_SENSITIVE_OF_TOUCH,
				focus->elements[i], mouse_action);
		if(-1 != index){
			int new_index = element_curve_insert_anchor_point_from_index_percent_(
					focus->elements[i], index, percent);
			if(-1 != new_index){
				bool ret = pv_focus_clear_set_element_index(
						focus,
						pv_focus_get_first_element(focus),
						new_index);
				et_assertf(ret, "%d", new_index);
			}
			return new_index;
		}
	}

	return -1;
}

static bool _func_insert_anchor_point_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
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

EtToolInfo _et_tool_infos[] = {
	{
		.tool_id = EtToolId_FocusElement, 
		.name = "Focus Element",
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_element_allow_24x24.svg",
		.func_mouse_action = _func_edit_element_mouse_action,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = 1, 
		.name = "Add Anchor Point",
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_put_allow_24x24.svg",
		.func_mouse_action = _func_add_anchor_point_mouse_action,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = 2,
		.name = "Edit Anchor Point",
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_edit_allow_24x24.svg",
		.func_mouse_action = _func_edit_anchor_point_mouse_action,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = 3,
		.name = "Edit Anchor Point Handle",
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_handle_allow_24x24.svg",
		.func_mouse_action = _func_edit_anchor_point_handle_mouse_action,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = 4,
		.name = "Knife Anchor Point",
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_knife_24x24.svg",
		.func_mouse_action = _func_knife_anchor_point_mouse_action,
		.mouse_cursor = NULL,
	},
	{
		.tool_id = 5,
		.name = "Insert Anchor Point",
		.icon = NULL,
		.icon_focus = NULL,
		.icon_cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_put_allow_plus_24x24.svg",
		.func_mouse_action = _func_insert_anchor_point_mouse_action,
		.mouse_cursor = NULL,
	},
};

int et_tool_get_num()
{
	return sizeof(_et_tool_infos) / sizeof(EtToolInfo);
}

static EtToolInfo *_et_tool_get_info_from_id(EtToolId tool_id)
{
	int num_tool = et_tool_get_num();
	if(tool_id < 0 || num_tool <= tool_id){
		et_error("");
		return NULL;
	}

	return &_et_tool_infos[tool_id];
}

const EtToolInfo *et_tool_get_info_from_id(EtToolId tool_id)
{
	et_assert(_et_tool_info_is_init);

	return _et_tool_get_info_from_id(tool_id);
}

