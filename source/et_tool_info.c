#include "et_tool_info.h"

#include "et_error.h"
#include "pv_element_general.h"
#include "pv_element.h"
#include "pv_element_infos.h"
#include "pv_focus.h"
#include "et_doc.h"
#include "et_doc_manager.h"
#include "et_etaion.h"

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
	PvPoint g_point;
}RecursiveDataGetFocus;

static bool _et_tool_func_pv_element_recurse_get_focus_element(
		PvElement *element, gpointer data, int level)
{
	RecursiveDataGetFocus *_data = data;
	PvElement **p_element = _data->p_element;

	const PvElementInfo *elem_info = pv_element_get_info_from_kind(element->kind);
	if(NULL == elem_info || NULL == elem_info->func_is_touch_element){
		et_bug("%p", elem_info);
		goto error;
	}
	bool is_touch = false;
	bool ret = elem_info->func_is_touch_element(
			&is_touch,
			element,
			_data->g_point.x,
			_data->g_point.y);
	if(!ret){
		et_error("");
		goto error;
	}
	if(is_touch){
		// ** detect is stop search.
		*p_element = element;
		return false;
	}

	return true;
error:
	return false;
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
				_et_tool_func_pv_element_recurse_get_focus_element,
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

	if(0 == (mouse_action.state & GDK_BUTTON1_MASK)){
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
		if(NULL == info || NULL == info->func_move_element){
			et_bug("%p", info);
			return false;
		}

		if(!info->func_move_element(focus_element,
					move.x, move.y)){
			et_error("");
			return false;
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

static bool _et_tool_bezier_add_point(EtDoc *doc, PvElement **_element, double x, double y)
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
			return false;
		}
	}

	PvAnchorPoint anchor_point = {
		.points = {{0,0}, {x, y,}, {0,0}},
	};
	if(!pv_element_bezier_add_anchor_point(element, anchor_point)){
		et_error("");
		return false;
	}

	if(is_new){
		if(NULL == parent){
			parent = pv_vg_get_layer_top(et_doc_get_vg_ref(doc));
			if(NULL == parent){
				et_error("");
				return false;
			}
		}

		if(!pv_element_append_child(parent, 
					NULL, element)){
			et_error("");
			return false;
		}
	}

	*_element = element;

	return true;
}

static int _et_etaion_radius_path_detect = 6;
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
					PvElementBezierData *_data =(PvElementBezierData *) _element->data;
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
										_et_etaion_radius_path_detect,
										_data->anchor_points[0].points[PvAnchorPointIndex_Point],
										mouse_action.point)
							  ){
								// ** do close anchor_point
								_data->is_close = true;
								is_closed = true;
							}
						}
					}
				}

				// ** new anchor_point
				if(!is_closed){
					if(!_et_tool_bezier_add_point(doc, &_element,
								mouse_action.point.x, mouse_action.point.y)){
						et_error("");
						return false;
					}else{
						pv_focus_clear_set_element(focus, _element);
					}
				}

			}
			break;
		case EtMouseAction_Up:
			break;
		case EtMouseAction_Move:
			{
				if(0 == (mouse_action.state & GDK_BUTTON1_MASK)){
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
				pv_element_bezier_anchor_point_set_handle(ap, PvAnchorPointIndex_Point,
						mouse_action.point);
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
		.tool_id = 0, 
		.name = "Focus Element",
		.icon = NULL,
		.icon_focus = NULL,
		.cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_element_allow_24x24.svg",
		.func_mouse_action = _et_tool_focus_element_mouse_action,
	},
	{
		.tool_id = 1, 
		.name = "Anchor Point",
		.icon = NULL,
		.icon_focus = NULL,
		.cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_put_allow_24x24.svg",
		.func_mouse_action = _et_tool_bezier_mouse_action,
	},
};

int et_tool_get_num()
{
	return sizeof(_et_tool_infos) / sizeof(EtToolInfo);
}

