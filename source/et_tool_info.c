#include "et_tool_info.h"

#include <math.h>
#include "et_error.h"
#include "pv_element_general.h"
#include "pv_element.h"
#include "pv_element_info.h"
#include "pv_focus.h"
#include "pv_rotate.h"
#include "et_doc.h"
#include "et_doc_manager.h"
#include "et_etaion.h"
#include "et_color_panel.h"
#include "et_focus_rel.h"
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

	if(pv_rect_is_inside(ul, point.x, point.y)){
		return EdgeKind_Resize_UpLeft;
	}else if(pv_rect_is_inside(ur, point.x, point.y)){
		return EdgeKind_Resize_UpRight;
	}else if(pv_rect_is_inside(dr, point.x, point.y)){
		return EdgeKind_Resize_DownRight;
	}else if(pv_rect_is_inside(dl, point.x, point.y)){
		return EdgeKind_Resize_DownLeft;
	}else if(pv_rect_is_inside(ul_rotate, point.x, point.y)){
		return EdgeKind_Rotate_UpLeft;
	}else if(pv_rect_is_inside(ur_rotate, point.x, point.y)){
		return EdgeKind_Rotate_UpRight;
	}else if(pv_rect_is_inside(dr_rotate, point.x, point.y)){
		return EdgeKind_Rotate_DownRight;
	}else if(pv_rect_is_inside(dl_rotate, point.x, point.y)){
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
static void _move_elements(
		EtDocId doc_id,
		EtMouseAction mouse_action)
{
	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	EtFocusRel *_focus_rel = et_focus_rel_new(focus);

	int num = pv_general_get_parray_num((void **)focus->elements);
	int num_rel = pv_general_get_parray_num((void **)_focus_rel->element_rels);
	if(num_rel != num){
		et_bug("%d %d", num, num_rel);
		goto finally;
	}

	const PvVg *vg = et_doc_get_current_vg_ref_from_id(doc_id);
	et_assert(vg);

	PvElement **elements = focus->elements;

	PvPoint move = {.x = 0, .y = 0};
	for(int i = 0; i < num; i++){
		PvElement *focus_element = elements[i];
		const PvElement *src_element = et_element_rel_get_element_from_vg_const(
				_focus_rel->element_rels[i], vg);

		const PvElementInfo *info = pv_element_get_info_from_kind(focus_element->kind);
		et_assertf(info, "%d", focus_element->kind);

		if(0 == i){
			move = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

			PvPoint src_point = info->func_get_point_by_anchor_points(src_element);
			PvPoint dst_point = pv_point_add(src_point, move);
			info->func_set_point_by_anchor_points(focus_element, dst_point);

			if(mouse_action.snap.is_snap_for_pixel){
				PvPoint snap_point = info->func_get_point_by_anchor_points(focus_element);
				snap_point.x = round(snap_point.x);
				snap_point.y = round(snap_point.y);
				info->func_set_point_by_anchor_points(focus_element, snap_point);

				move = pv_point_sub(snap_point, src_point);
			}

		}else{
			PvPoint src_point = info->func_get_point_by_anchor_points(src_element);
			PvPoint dst_point = pv_point_add(src_point, move);
			info->func_set_point_by_anchor_points(focus_element, dst_point);
		}
	}

finally:
	et_focus_rel_free(_focus_rel);

	return;
}

static PvRect _get_rect_resize(PvPoint base, EdgeKind edge_kind, PvRect src_rect, PvPoint move, PvPoint scale)
{
	PvRect rect = src_rect;
	rect.x -= base.x;
	rect.y -= base.y;

	switch(edge_kind){
		case EdgeKind_Resize_UpLeft:
			rect.x *= scale.x;
			rect.y *= scale.y;

			rect.x += move.x;
			rect.w *= scale.x;
			rect.y += move.y;
			rect.h *= scale.y;
			break;
		case EdgeKind_Resize_UpRight:
			rect.x *= scale.x;
			rect.y *= scale.y;

			//rect.x
			rect.w *= scale.x;
			rect.y += move.y;
			rect.h *= scale.y;
			break;
		case EdgeKind_Resize_DownLeft:
			rect.x *= scale.x;
			rect.y *= scale.y;

			rect.x += move.x;
			rect.w *= scale.x;
			//rect.y
			rect.h *= scale.y;
			break;
		case EdgeKind_Resize_DownRight:
			rect.x *= scale.x;
			rect.y *= scale.y;

			//rect.x
			rect.w *= scale.x;
			//rect.y
			rect.h *= scale.y;
			break;
		default:
			et_assert(false);
			break;
	}

	rect.x += base.x;
	rect.y += base.y;

	return rect;
}

static EdgeKind _resize_elements(
		EtDocId doc_id,
		EtMouseAction mouse_action,
		EdgeKind _src_edge_kind,
		PvRect src_extent_rect)
{
	EdgeKind dst_edge_kind = _src_edge_kind;

	PvPoint diff = pv_point_div_value(mouse_action.diff_down, mouse_action.scale);

	PvPoint size_after;
	switch(dst_edge_kind){
		case EdgeKind_Resize_UpLeft:
			size_after.x = (src_extent_rect.w - diff.x);
			size_after.y = (src_extent_rect.h - diff.y);
			break;
		case EdgeKind_Resize_UpRight:
			size_after.x = (src_extent_rect.w + diff.x);
			size_after.y = (src_extent_rect.h - diff.y);
			break;
		case EdgeKind_Resize_DownLeft:
			size_after.x = (src_extent_rect.w - diff.x);
			size_after.y = (src_extent_rect.h + diff.y);
			break;
		case EdgeKind_Resize_DownRight:
			size_after.x = (src_extent_rect.w + diff.x);
			size_after.y = (src_extent_rect.h + diff.y);
			break;
		default:
			et_assert(false);
			break;
	}

	PvPoint scale = {
		.x = size_after.x / src_extent_rect.w,
		.y = size_after.y / src_extent_rect.h,
	};
	const double DELTA_OF_RESIZE = 0.001;
	scale.x = ((DELTA_OF_RESIZE > src_extent_rect.w)? DELTA_OF_RESIZE : scale.x); //! @todo fix
	scale.y = ((DELTA_OF_RESIZE > src_extent_rect.h)? DELTA_OF_RESIZE : scale.y);

	//! @todo minus to not implement.
	scale.x = ((scale.x > DELTA_OF_RESIZE) ? scale.x : DELTA_OF_RESIZE);
	scale.y = ((scale.y > DELTA_OF_RESIZE) ? scale.y : DELTA_OF_RESIZE);

	PvPoint base = {
		.x = src_extent_rect.x,
		.y = src_extent_rect.y,
	};

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	EtFocusRel *_focus_rel = et_focus_rel_new(focus);

	int num = pv_general_get_parray_num((void **)focus->elements);
	int num_rel = pv_general_get_parray_num((void **)_focus_rel->element_rels);
	if(num_rel != num){
		et_bug("%d %d", num, num_rel);
		goto finally;
	}

	const PvVg *vg = et_doc_get_current_vg_ref_from_id(doc_id);
	et_assert(vg);

	PvElement **elements = focus->elements;
	int num_ = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < num_; i++){
		PvElement *element = elements[i];
		const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);

		const PvElement *src_element = et_element_rel_get_element_from_vg_const(
				_focus_rel->element_rels[i], vg);
		const PvRect src_rect = info->func_get_rect_by_anchor_points(src_element);

		PvRect dst_rect = _get_rect_resize(base, _src_edge_kind, src_rect, diff, scale);

		info->func_set_rect_by_anchor_points(element, dst_rect);
	}

finally:
	et_focus_rel_free(_focus_rel);

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

typedef enum{
	EtFocusElementMouseActionMode_None,
	EtFocusElementMouseActionMode_Move,
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

	return pv_element_bezier_new_from_rect(rect);
}

static bool _et_tool_focus_element_mouse_action(EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
{
	static PvElement *_touch_element = NULL;
	static bool _is_already_focus = false;
	static bool _is_move = false;
	static EtFocusElementMouseActionMode _mode = EtFocusElementMouseActionMode_None;
	static EdgeKind _mode_edge = EdgeKind_None;
	static PvRect src_extent_rect;

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	et_assertf(focus, "%d", doc_id);

	PvRect _src_extent_rect = PvRect_Default;
	EdgeKind _edge = EdgeKind_None;

	if(pv_focus_is_focused(focus)){
		int num = pv_general_get_parray_num((void **)focus->elements);
		if(0 < num){
			_src_extent_rect = _get_rect_extent_from_elements(focus->elements);
			_edge = _get_outside_touch_edge_kind_from_rect(
					_src_extent_rect, mouse_action.point, mouse_action.scale);
		}
	}

	if(EtFocusElementMouseActionMode_None == _mode){
		*cursor = _get_cursor_from_edge(_edge);
	}else{
		*cursor = _get_cursor_from_edge(_mode_edge);
	}

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				_is_already_focus = false;
				_is_move = false;


				_mode_edge = _edge;
				src_extent_rect = _src_extent_rect;

				switch(_edge){
					case EdgeKind_Resize_UpLeft:
					case EdgeKind_Resize_UpRight:
					case EdgeKind_Resize_DownLeft:
					case EdgeKind_Resize_DownRight:
						{
							_mode = EtFocusElementMouseActionMode_Resize;
						}
						break;
					case EdgeKind_Rotate_UpLeft:
					case EdgeKind_Rotate_UpRight:
					case EdgeKind_Rotate_DownLeft:
					case EdgeKind_Rotate_DownRight:
						{
							_mode = EtFocusElementMouseActionMode_Rotate;
						}
						break;
					case EdgeKind_None:
					default:
						{
							_mode = EtFocusElementMouseActionMode_Move;

							_touch_element = _get_touch_element(doc_id, mouse_action.point);

							if(NULL == _touch_element){
								et_assertf(pv_focus_clear_to_parent_layer(focus), "%d", doc_id);
								_mode = EtFocusElementMouseActionMode_FocusingByArea;
							}else{
								_is_already_focus = pv_focus_is_exist_element(focus, _touch_element);
								pv_focus_add_element(focus, _touch_element);
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

				if(!_is_move)
				{
					const int PX_SENSITIVE_OF_MOVE = 3;

					if( PX_SENSITIVE_OF_MOVE > abs(mouse_action.diff_down.x)
							&& PX_SENSITIVE_OF_MOVE > abs(mouse_action.diff_down.y))
					{
						break;
					}else{
						_is_move = true;
						if(EtFocusElementMouseActionMode_Move == _mode && !_is_already_focus){
							pv_focus_clear_set_element(focus, pv_focus_get_first_element(focus));
						}
					}
				}

				switch(_mode){
					case EtFocusElementMouseActionMode_Move:
						{
							_move_elements(doc_id, mouse_action);
						}
						break;
					case EtFocusElementMouseActionMode_Resize:
						{

							EdgeKind _edge_kind = _resize_elements(doc_id, mouse_action, _mode_edge, src_extent_rect);

							*cursor = _get_cursor_from_edge(_edge_kind);
						}
						break;
					case EtFocusElementMouseActionMode_Rotate:
						{

							EdgeKind _edge_kind = _rotate_elements(doc_id, mouse_action, _mode_edge, src_extent_rect);

							*cursor = _get_cursor_from_edge(_edge_kind);
						}
						break;
					default:
						break;
				}
			}
			break;
		case EtMouseAction_Up:
			{
				switch(_mode){
					case EtFocusElementMouseActionMode_Move:
						{
							if(_is_move){
								// NOP
							}else{
								if(0 == (mouse_action.state & GDK_SHIFT_MASK)){
									et_assertf(pv_focus_clear_set_element(focus, _touch_element), "%d", doc_id);
								}else{
									if(!_is_already_focus){
										// NOP
									}else{
										et_assertf(pv_focus_remove_element(focus, _touch_element), "%d", doc_id);
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

				_mode_edge = EdgeKind_None;
				et_doc_save_from_id(doc_id);

				_mode = EtFocusElementMouseActionMode_None;
			}
			break;
		default:
			break;
	}

	// edit draw
	{
		PvElement *element_group_edit_draw = pv_element_new(PvElementKind_Group);
		et_assert(element_group_edit_draw);

		if(pv_focus_is_focused(focus)){
			PvRect _after_extent_rect = _get_rect_extent_from_elements(focus->elements);
			int size = 5.0 / mouse_action.scale;
			for(int i = 0; i < 4; i++){
				PvPoint center = pv_rect_get_edge_point(_after_extent_rect, i);
				PvRect rect = get_square_rect_from_center_point_(center, size);
				PvElement *element_bezier = pv_element_bezier_new_from_rect(rect);
				et_assert(element_bezier);
				pv_element_append_child(element_group_edit_draw, NULL, element_bezier);
			}

			switch(_mode){
				case EtFocusElementMouseActionMode_Rotate:
					{
						// rotate center mark
						PvPoint center = pv_rect_get_center(_after_extent_rect);
						int diameter = 8.0 / mouse_action.scale;
						PvElement *element_center = _element_new_from_circle(center, diameter);
						et_assert(element_center);
						pv_element_append_child(element_group_edit_draw, NULL, element_center);
					}
					break;
				default:
					break;
			}

		}

		et_doc_set_element_group_edit_draw_from_id(doc_id, element_group_edit_draw);
		pv_element_remove_free_recursive(element_group_edit_draw);
	}

	return true;
}

/** @return new AnchorPoint index number. error:-1 */
static int _et_tool_bezier_add_anchor_point(EtDoc *doc, PvElement **_element, double x, double y)
{
	bool is_new = true;
	PvElement *element = *_element;
	PvElement *parent = NULL;
	if(NULL == element){
		parent = NULL;
		//element = NULL;
	}else{
		switch(element->kind){
			case PvElementKind_Bezier:
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

		element = pv_element_new(PvElementKind_Bezier);
		if(NULL == element){
			et_error("");
			return -1;
		}

		element->color_pair = et_color_panel_get_color_pair();
	}

	PvAnchorPoint anchor_point = {
		.points = {{0,0}, {x, y,}, {0,0}},
	};
	if(!pv_element_bezier_add_anchor_point(element, anchor_point)){
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

	return (pv_element_bezier_get_num_anchor_point(*_element) - 1);
}

static bool _et_tool_bezier_mouse_action(EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
{
	bool result = true;

	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		result = false;
		goto finally;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		et_error("");
		result = false;
		goto finally;
	}

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				et_debug(" x:%d, y:%d,",
						(int)mouse_action.point.x,
						(int)mouse_action.point.y);

				PvElement *_element = pv_focus_get_first_element(focus);

				bool is_closed = false;
				if(NULL != _element && PvElementKind_Bezier == _element->kind){
					PvElementBezierData *_data = (PvElementBezierData *) _element->data;
					if(NULL == _data){
						et_error("");
						result = false;
						goto finally;
					}
					if(_data->is_close){
						// if already closed is goto new anchor_point
						_element = NULL;
					}else{
						if(0 < _data->anchor_points_num){
							if(_is_bound_point(
										PX_SENSITIVE_OF_TOUCH,
										_data->anchor_points[0].points[PvAnchorPointIndex_Point],
										mouse_action.point)
							  ){
								// ** do close anchor_point
								_data->is_close = true;
								is_closed = true;
								pv_focus_clear_set_element_index(focus, _element, 0);
							}
						}
					}
				}

				// ** new anchor_point
				if(!is_closed){
					int index = _et_tool_bezier_add_anchor_point(
							doc, &_element,
							mouse_action.point.x, mouse_action.point.y);
					if(0 > index){
						et_error("");
						result = false;
						goto finally;
					}else{
						pv_focus_clear_set_element_index(focus, _element, index);
					}
				}

			}
			break;
		case EtMouseAction_Up:
			break;
		case EtMouseAction_Move:
			{
				if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
					break;
				}

				PvElement *_element = pv_focus_get_first_element(focus);
				if(NULL == _element || PvElementKind_Bezier != _element->kind){
					et_error("");
					result = false;
					goto finally;
				}

				PvElementBezierData *_data =(PvElementBezierData *) _element->data;
				if(NULL == _data){
					et_error("");
					result = false;
					goto finally;
				}
				PvAnchorPoint *ap = NULL;
				if(_data->is_close){
					ap = &_data->anchor_points[0];
				}else{
					ap = &_data->anchor_points[_data->anchor_points_num - 1];
				}

				PvPoint p_ap = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_Point);
				PvPoint p_diff = pv_point_sub(p_ap, mouse_action.point);
				if(fabs(p_diff.x) < PX_SENSITIVE_OF_TOUCH
						&& fabs(p_diff.y) < PX_SENSITIVE_OF_TOUCH)
				{
					pv_anchor_point_set_handle_zero(
							ap,
							PvAnchorPointIndex_Point);
				}else{
					pv_anchor_point_set_handle(
							ap,
							PvAnchorPointIndex_Point,
							mouse_action.point);
				}
			}
			break;
		case EtMouseAction_Unknown:
		default:
			et_bug("0x%x", mouse_action.action);
			break;
	}

finally:
	et_doc_set_element_group_edit_draw_from_id(doc_id, NULL);

	return result;
}

static bool _get_touch_element_and_index(
		PvElement *element, gpointer data, int level)
{
	RecursiveDataGetFocus *_data = data;

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	assert(info);
	assert(info->func_get_num_anchor_point);

	int index = -1;
	PvAnchorPoint *anchor_points = info->func_new_anchor_points(element);

	const PvElementBezierData *data_ = element->data;
	assert(data_);
	int num = data_->anchor_points_num;
	for(int i = 0; i < num; i++){
		PvAnchorPoint ap = anchor_points[i];
		if(_is_bound_point(
					PX_SENSITIVE_OF_TOUCH,
					ap.points[PvAnchorPointIndex_Point],
					_data->g_point))
		{
			index = i;
		}
	}
	free(anchor_points);

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
	PvPoint move = PvPoint_Default;
	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
		const PvElementInfo *info = pv_element_get_info_from_kind(focus->elements[i]->kind);
		assert(info);
		if(0 == i){
			assert(info->func_get_anchor_point);
			PvAnchorPoint prev = info->func_get_anchor_point(focus->elements[i], focus->index);
			move = pv_point_sub(mouse_action.point, prev.points[PvAnchorPointIndex_Point]);
			assert(info->func_set_anchor_point_point);
			info->func_set_anchor_point_point(focus->elements[i], focus->index, mouse_action.point);
		}else{
			assert(info->func_move_anchor_point_point);
			info->func_move_anchor_point_point(focus->elements[i], focus->index, move);
		}
	}

	return true;
}

static bool _et_tool_edit_anchor_point_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
{
	bool result = true;

	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		result = false;
		goto finally;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		et_error("");
		result = false;
		goto finally;
	}

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
					pv_focus_clear_to_parent_layer(focus);
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

finally:
	et_doc_set_element_group_edit_draw_from_id(doc_id, NULL);

	return result;
}

/*! 
 * @fixme implement and usable only to PvElementKind_Bezier
 * @return anchor points of focusing. Not focus:NULL.
 */
PvAnchorPoint *_get_focus_anchor_point(const PvFocus *focus){
	// ** get focusing AnchorPoint
	int num = pv_general_get_parray_num((void **)focus->elements);
	if(num < 1){
		return NULL;
	}
	PvElement *element = focus->elements[0];
	if(PvElementKind_Bezier != element->kind){
		return NULL;
	}
	assert(element->data);
	PvElementBezierData *data = element->data;
	int num_ap = pv_element_bezier_get_num_anchor_point(element);
	assert(focus->index < num_ap);
	PvAnchorPoint *ap = &(data->anchor_points[focus->index]);

	return ap;
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
	PvAnchorPoint *ap = _get_focus_anchor_point(focus);
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

static bool _et_tool_edit_anchor_point_handle_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action, GdkCursor **cursor)
{
	bool result = true;

	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		result = false;
		goto finally;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		et_error("");
		result = false;
		goto finally;
	}


	// static PvAnchorPointIndex handle = PvAnchorPointIndex_Point; //!< Handle not grub.
	static int handle = -1;
	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				handle = _edit_anchor_point_handle_grub_focus(focus, mouse_action);
				PvAnchorPoint *ap = _get_focus_anchor_point(focus);
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

				PvAnchorPoint *ap = _get_focus_anchor_point(focus);
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

finally:
	et_doc_set_element_group_edit_draw_from_id(doc_id, NULL);

	return result;
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
		.func_mouse_action = _et_tool_focus_element_mouse_action,
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
		.func_mouse_action = _et_tool_bezier_mouse_action,
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
		.func_mouse_action = _et_tool_edit_anchor_point_mouse_action,
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
		.func_mouse_action = _et_tool_edit_anchor_point_handle_mouse_action,
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

