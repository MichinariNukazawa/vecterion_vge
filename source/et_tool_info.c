#include "et_tool_info.h"

#include <math.h>
#include "et_error.h"
#include "pv_element_general.h"
#include "pv_element.h"
#include "pv_element_info.h"
#include "pv_focus.h"
#include "et_doc.h"
#include "et_doc_manager.h"
#include "et_etaion.h"
#include "et_color_panel.h"


EtToolInfo _et_tool_infos[];


bool _et_tool_info_is_init = false; // check initialized tools

static EtToolInfo *_et_tool_get_info_from_id(EtToolId tool_id);

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
	et_debug("tool_num:%d", num_tool);
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

	return true;
}



static int _px_sensitive = 16;

static bool _et_etaion_is_bound_point(int radius, PvPoint p1, PvPoint p2)
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

typedef struct RecursiveDataGetFocus{
	PvElement **p_element;
	int *p_index;
	PvPoint g_point;
}RecursiveDataGetFocus;

static bool _get_touch_element(
		PvElement *element, gpointer data, int level)
{
	RecursiveDataGetFocus *_data = data;
	PvElement **p_element = _data->p_element;

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	et_assertf(info, "%d", element->kind);

	bool is_touch = false;
	bool ret = info->func_is_touch_element(
			&is_touch,
			element,
			_px_sensitive,
			_data->g_point.x,
			_data->g_point.y);
	if(!ret){
		et_error("");
		return false;
	}
	if(is_touch){
		// ** detect is stop search.
		*p_element = element;
		return false;
	}

	return true;
}

static bool _et_tool_focus_element_mouse_action_get_focus_element(
		PvElement **p_element,
		EtDocId doc_id,
		PvPoint g_point)
{
	PvVg *vg = et_doc_get_vg_ref_from_id(doc_id);
	if(NULL == vg){
		et_error("");
		return false;
	}

	RecursiveDataGetFocus rec_data = {
		.p_element = p_element,
		.g_point = g_point,
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_asc(vg->element_root,
				_get_touch_element,
				NULL,
				&rec_data,
				&error)){
		et_error("level:%d", error.level);
		return false;
	}

	return true;
}

static bool is_move_of_down = false;
static bool _et_tool_info_move(
		EtDocId doc_id, EtMouseAction mouse_action, PvElement **elements)
{
	const int pxdiff = 3;

	if(0 == (mouse_action.state & MOUSE_BUTTON_LEFT_MASK)){
		return true;
	}

	PvPoint move = {.x = 0, .y = 0};
	if(!is_move_of_down)
	{
		if( pxdiff < abs(mouse_action.diff_down.x)
				|| pxdiff < abs(mouse_action.diff_down.y))
		{
			is_move_of_down = true;

			move = mouse_action.diff_down;
		}
	}else{
		move = mouse_action.move;
	}

	int num = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < num; i++){
		PvElement *focus_element = elements[i];

		const PvElementInfo *info = pv_element_get_info_from_kind(focus_element->kind);
		et_assertf(info, "%d", focus_element->kind);
		et_assertf(info->func_move_element, "%d", focus_element->kind);

		if(0 == i){
			if(is_move_of_down){
				PvPoint point_prev = info->func_get_point_by_anchor_points(focus_element);
				info->func_move_element(focus_element, move.x, move.y);
				if(mouse_action.snap.is_snap_for_pixel){
					{
						PvPoint point_snap = info->func_get_point_by_anchor_points(focus_element);
						point_snap.x = round(point_snap.x),
							point_snap.y = round(point_snap.y),
							info->func_set_point_by_anchor_points(focus_element, point_snap);
					}
					PvPoint point_dst = info->func_get_point_by_anchor_points(focus_element);
					move = pv_point_sub(point_dst, point_prev);
				}
			}

		}else{
			info->func_move_element(focus_element, move.x, move.y);
		}
	}

	return true;
}

static PvElement *_element_touch = NULL;
static PvElement *_element_touch_already = NULL;
static bool _et_tool_focus_element_mouse_action(
		EtDocId doc_id, EtMouseAction mouse_action)
{
	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				is_move_of_down = false;

				PvElement *element = NULL;
				if(!_et_tool_focus_element_mouse_action_get_focus_element(
							&element,
							doc_id,
							mouse_action.point))
				{
					et_error("");
					return false;
				}

				PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
				if(NULL == focus){
					et_error("");
					return false;
				}

				et_debug("%p", element);
				_element_touch = NULL;
				_element_touch_already = NULL;
				if(NULL == element){
					pv_focus_clear_to_parent_layer(focus);
				}else{
					_element_touch = element;
					if(pv_focus_is_exist_element(focus, element)){
						_element_touch_already = element;
					}

					if(!pv_focus_add_element(focus, element)){
						et_error("");
						return false;
					}
				}

			}
			break;
		case EtMouseAction_Move:
			{
				if(0 != (mouse_action.state & GDK_SHIFT_MASK)){
					break;
				}

				// one element if touch new element.
				PvElement *elements_tmp[2] = {NULL, NULL};
				PvElement **elements_move = NULL; // target elements.
				if(NULL == _element_touch_already){
					elements_tmp[0] = _element_touch;
					elements_move = elements_tmp;
				}else{
					PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
					if(NULL == focus){
						et_error("");
						break;
					}

					elements_move = focus->elements;
				}

				if(!_et_tool_info_move(doc_id, mouse_action, elements_move)){
					et_error("");
					break;
				}
			}
			break;
		case EtMouseAction_Up:
			{
				et_debug("%d, %p", is_move_of_down, _element_touch_already);
				PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
				if(NULL == focus){
					et_error("");
					return false;
				}

				if(0 == (mouse_action.state & GDK_SHIFT_MASK)){
					if(!is_move_of_down || NULL == _element_touch_already){
						if(NULL != _element_touch){
							if(!pv_focus_clear_set_element(focus, _element_touch)){
								et_error("");
								return false;
							}
						}
					}
				}else{
					if(!is_move_of_down){
						if(NULL != _element_touch_already){
							if(!pv_focus_remove_element(focus, _element_touch_already)){
								et_error("");
								return false;
							}
						}
					}
				}

				et_doc_save_from_id(doc_id);
			}
			break;
		default:
			break;
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

static bool _et_tool_bezier_mouse_action(EtDocId doc_id, EtMouseAction mouse_action)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		return false;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		et_error("");
		return false;
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
						return false;
					}
					if(_data->is_close){
						// if already closed is goto new anchor_point
						_element = NULL;
					}else{
						if(0 < _data->anchor_points_num){
							if(_et_etaion_is_bound_point(
										_px_sensitive,
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
						return false;
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
					return false;
				}

				PvElementBezierData *_data =(PvElementBezierData *) _element->data;
				if(NULL == _data){
					et_error("");
					return false;
				}
				PvAnchorPoint *ap = NULL;
				if(_data->is_close){
					ap = &_data->anchor_points[0];
				}else{
					ap = &_data->anchor_points[_data->anchor_points_num - 1];
				}

				PvPoint p_ap = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_Point);
				PvPoint p_diff = pv_point_sub(p_ap, mouse_action.point);
				if(fabs(p_diff.x) < _px_sensitive
						&& fabs(p_diff.y) < _px_sensitive)
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

	return true;
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
		if(_et_etaion_is_bound_point(
					_px_sensitive,
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

static bool _et_tool_info_move_anchor_points(EtDocId doc_id, EtMouseAction mouse_action)
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
		EtDocId doc_id, EtMouseAction mouse_action)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		return false;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		et_error("");
		return false;
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

				if(!_et_tool_info_move_anchor_points(doc_id, mouse_action)){
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

	return true;
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

	if(_et_etaion_is_bound_point(
				_px_sensitive,
				p_prev,
				mouse_action.point))
	{
		return PvAnchorPointIndex_HandlePrev;
	}
	if(_et_etaion_is_bound_point(
				_px_sensitive,
				p_next,
				mouse_action.point))
	{
		return PvAnchorPointIndex_HandleNext;
	}
	if(_et_etaion_is_bound_point(
				_px_sensitive,
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
		EtDocId doc_id, EtMouseAction mouse_action)
{
	EtDoc *doc = et_doc_manager_get_doc_from_id(doc_id);
	if(NULL == doc){
		et_error("");
		return false;
	}

	PvFocus *focus = et_doc_get_focus_ref_from_id(doc_id);
	if(NULL == focus){
		et_error("");
		return false;
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
				if(fabs(p_diff.x) < _px_sensitive
						&& fabs(p_diff.y) < _px_sensitive)
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

