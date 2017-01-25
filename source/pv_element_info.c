#include "pv_element_info.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pv_error.h"
#include "pv_render_context.h"
#include "pv_color.h"
#include "pv_cairo.h"
#include "pv_rotate.h"



static PvRect _get_rect_extent_from_cr(cairo_t *cr)
{
	double x1, y1, x2, y2;
	cairo_stroke_extents(cr, &x1, &y1, &x2, &y2);
	PvRect rect = {0,0,0,0};
	rect.x = x1;
	rect.y = y1;
	rect.w = x2 - x1;
	rect.h = y2 - y1;

	return rect;
}



// ******** ********
// null functions. (ex. use method not implement)
//! @detail 無効なindexを引いた際に埋め込まれているダミー関数
// ******** ********


/** @brief write_svg未実装箇所に挿入する */
static int _func_notimpl_write_svg(
		InfoTargetSvg *target,
		const PvElement *element, const ConfWriteSvg *conf)
{
	if(NULL == target){
		pv_bug("");
		return -1;
	}

	if(NULL == target->xml_parent_node){
		pv_bug("");
		return -1;
	}

	if(NULL == element){
		pv_bug("");
		return -1;
	}

	if(NULL == conf){
		pv_bug("");
		return -1;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	if(NULL == info){
		pv_bug("%d", element->kind);
		return -1;
	}

	xmlNodePtr node = NULL;
	// node = xmlNewNode(NULL, BAD_CAST info->name);
	node = xmlNewComment(BAD_CAST info->name);

	xmlAddChild(target->xml_parent_node, node);
	target->xml_new_node = node;

	return 0;
}

static int _func_zero_get_num_anchor_point(
		const PvElement *element)
{
	return 0;
}

static bool _func_nop_is_exist_anchor_point(
		const PvElement *element,
		const PvAnchorPoint *ap)
{
	return false;
}

static PvAnchorPoint *_func_null_new_anchor_points(
		const PvElement *element)
{
	return NULL;
}

static PvAnchorPoint *_func_notimpl_get_anchor_point(
		const PvElement *element,
		const int index)
{
	return NULL;
}

static bool _func_notimpl_set_anchor_point_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint point)
{
	return true;
}

static bool _func_notimpl_move_anchor_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint move)
{
	return true;
}

static PvRect _func_notimpl_get_rect_by_anchor_points(
		const PvElement *element)
{
	return PvRect_Default;
}

static bool _func_notimpl_set_rect_by_anchor_points(
		PvElement *element,
		PvRect rect)
{
	return true;
}

static PvRect _func_notimpl_get_rect_by_draw(
		const PvElement *element)
{
	return PvRect_Default;
}

static void _func_nop_apply_appearances(
		PvElement *element,
		PvAppearance **appearances)
{
	return;
}


/* ****************
 * General
 **************** */

/** @brief 
 * arg1 NULL -> return NULL is not error. (malloc error to exit app)
 */
char *pv_general_new_str(const char *src)
{
	if(NULL == src){
		return NULL;
	}

	if(2000 < strlen(src)){
		pv_debug("len:%lu '%s'", strlen(src), src);
	}

	char *dst = (char *)malloc(sizeof(char) * (strlen(src) + 1));
	pv_assertf(dst, "%s", src);

	strcpy(dst, src);

	return dst;
}

// ** PvElementKindごとのdataのnew関数群

/* ****************
 * Group(Root,Layer,Group)
 **************** */

static gpointer _func_group_new_data()
{
	PvElementGroupData *data = (PvElementGroupData *)malloc(sizeof(PvElementGroupData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->name = NULL;

	return (gpointer)data;
}

static bool _func_group_free_data(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementGroupData *data = (PvElementGroupData *)_data;

	if(NULL != data->name){
		free(data->name);
	}

	free(data);

	return true;
}

static gpointer _func_group_copy_new_data(void *_data)
{
	if(NULL == _data){
		pv_bug("");
		return NULL;
	}

	PvElementGroupData *data = (PvElementGroupData *)_data;

	PvElementGroupData *new_data = (PvElementGroupData *)malloc(sizeof(PvElementGroupData));
	pv_assert(new_data);

	*new_data = *data;

	new_data->name = pv_general_new_str(data->name);

	return (gpointer)new_data;
}

static int _func_group_write_svg(
		InfoTargetSvg *target,
		const PvElement *element, const ConfWriteSvg *conf)
{
	if(NULL == target){
		pv_bug("");
		return -1;
	}

	if(NULL == target->xml_parent_node){
		pv_bug("");
		return -1;
	}

	if(NULL == element){
		pv_bug("");
		return -1;
	}

	if(NULL == conf){
		pv_bug("");
		return -1;
	}

	if(PvElementKind_Root == element->kind){
		return true;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	if(NULL == info){
		pv_bug("%d", element->kind);
		return -1;
	}

	xmlNodePtr node = NULL;
	node = xmlNewNode(NULL, BAD_CAST "g");

	// ** To Inkscape
	if(PvElementKind_Layer == element->kind){
		xmlNewProp(node, BAD_CAST "inkscape:groupmode", BAD_CAST "layer");
	}
	xmlNewProp(node, BAD_CAST "inkscape:label", BAD_CAST info->name);

	xmlAddChild(target->xml_parent_node, node);
	target->xml_parent_node = node;
	target->xml_new_node = node;

	return 0;
}

static bool _func_group_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	return true;
}

static bool _func_group_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		int offset,
		double gx,
		double gy)
{
	*is_touch = false;
	return true;
}

static bool _func_nop_is_overlap_rect(
		bool *is_overlap,
		const PvElement *element,
		int offset,
		PvRect rect)
{
	*is_overlap = false;
	return true;
}

bool _func_nop_remove_delete_anchor_point(
		PvElement *element,
		const PvAnchorPoint *anchor_point,
		PvElement **p_foot_element,
		bool *is_deleted_element)
{
	*p_foot_element = NULL;
	*is_deleted_element = false;

	return true;
}

static bool _pv_general_strcmp(char *str0, char *str1)
{
	if(NULL == str0 && NULL == str1){
		return true;
	}
	if(NULL == str0 || NULL == str1){
		return false;
	}

	return (0 == strcmp(str0, str1));
}

static bool _func_group_is_diff_one(
		bool *is_diff,
		const PvElement *element0,
		const PvElement *element1)
{
	PvElementGroupData *data0 = element0->data;
	PvElementGroupData *data1 = element1->data;
	if(NULL == data0 || NULL == data1){
		pv_bug("%p,%p", data0, data1);
		*is_diff = true;
		return false;
	}

	*is_diff = !_pv_general_strcmp(data0->name, data1->name);
	return true;
}

static bool _func_group_move_element(
		const PvElement *element,
		double gx,
		double gy)
{
	return true;
}

// ******** ********
// Curve
// ******** ********

typedef enum{
	PvElementPointKind_Normal,
	PvElementPointKind_Selected,
	PvElementPointKind_SubSelected,
	PvElementPointKind_AnchorHandle,
}PvElementPointKind;

typedef enum{
	PvSimplifyOption_All,
	PvSimplifyOption_ResizeOnly,
}PvSimplifyOption;

int _func_curve_get_num_anchor_point(const PvElement *element);

static bool _func_curve_move_element(
		const PvElement *element,
		double gx,
		double gy);

static PvRect _func_curve_get_rect_by_draw(
		const PvElement *element);

static PvRect _func_curve_get_rect_by_anchor_points(
		const PvElement *element);

static void _func_curve_apply_appearances(
		PvElement *element,
		PvAppearance **appearances);

static PvElement *_curve_simplify_new(
		const PvElement *element,
		PvRenderContext render_context,
		PvSimplifyOption option)
{
	pv_assert(element);

	PvElement *simplify = pv_element_copy_recursive(element);
	pv_assert(simplify);
	PvElementCurveData *simplify_data = simplify->data;
	pv_assert(simplify_data);

	// appearance
	_func_curve_apply_appearances(simplify, element->etaion_work_appearances);
	simplify->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

	// scale
	simplify->stroke.width *= render_context.scale;
	size_t num = pv_anchor_path_get_anchor_point_num(simplify_data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvPoint point = {0, 0};
		PvPoint scale = {render_context.scale, render_context.scale};
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(simplify_data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		pv_anchor_point_rescale(ap, scale, point);
	}

	return simplify;
}

void _curve_simplify_free(PvElement *self)
{
	pv_element_remove_free_recursive(self);
}

static void _curve_draw_point(cairo_t *cr, PvPoint gp, PvElementPointKind kind)
{
	cairo_arc (cr, gp.x, gp.y, 3.0, 0., 2 * M_PI);

	switch(kind){
		case PvElementPointKind_Selected:
			pv_cairo_set_source_rgba_workingcolor(cr);
			break;
		case PvElementPointKind_SubSelected:
			pv_cairo_set_source_rgba_subworkingcolor(cr);
			break;
		default:
			cairo_set_source_rgba (cr, 1, 1, 1, 1.0); // white
	}
	cairo_fill_preserve (cr);

	pv_cairo_set_source_rgba_workingcolor(cr);
	cairo_set_line_width(cr, 0.75);
	cairo_stroke (cr);
}

static void _draw_extent_from_rect(cairo_t *cr, PvRect rect)
{
	cairo_rectangle (cr, rect.x, rect.y, rect.w, rect.h);
	pv_cairo_set_source_rgba_workingcolor(cr);
	cairo_set_line_width(cr, 1.0);
	cairo_stroke(cr);
}

void _curve_command_path(
		cairo_t *cr,
		const PvElement *element)
{
	const PvElementCurveData *data = element->data;

	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	if(0 == num){
		return;
	}

	// ** path stroking
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		PvPoint point = pv_anchor_point_get_point(ap);
		if(0 == i){
			if(1 == num){
				cairo_rectangle (cr, point.x, point.y, 2, 2);
				cairo_fill (cr);
			}else{
				cairo_move_to(cr, point.x, point.y);
			}
		}else{
			const PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, (i - 1), PvAnchorPathIndexTurn_Disable);

			PvPoint first_point = pv_anchor_point_get_handle(ap_prev, PvAnchorPointIndex_HandleNext);
			PvPoint second_point = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_HandlePrev);

			cairo_curve_to(
					cr,
					first_point.x, first_point.y,
					second_point.x,  second_point.y,
					point.x, point.y);
		}

	}
	if(pv_anchor_path_get_is_close(data->anchor_path)){
		PvPoint point = pv_anchor_point_get_point(pv_anchor_path_get_anchor_point_from_index(data->anchor_path, 0, PvAnchorPathIndexTurn_Disable));

		const PvAnchorPoint *ap_last = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, ((int)num - 1), PvAnchorPathIndexTurn_Disable);
		PvPoint first_point = pv_anchor_point_get_handle(ap_last, PvAnchorPointIndex_HandleNext);

		const PvAnchorPoint *ap_first = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, 0, PvAnchorPathIndexTurn_Disable);
		PvPoint second_point = pv_anchor_point_get_handle(ap_first, PvAnchorPointIndex_HandlePrev);

		cairo_curve_to(
				cr,
				first_point.x, first_point.y,
				second_point.x,  second_point.y,
				point.x, point.y);
		cairo_close_path (cr);
	}
}

static void _func_curve_draw_inline(
		cairo_t *cr,
		const PvElement *element)
{
	_curve_command_path(cr, element);

	cairo_set_line_width(cr, element->stroke.width);

	//! fill
	PvCairoRgbaColor cc_f = pv_color_get_cairo_rgba(element->color_pair.colors[PvColorPairGround_BackGround]);
	cairo_set_source_rgba (cr, cc_f.r, cc_f.g, cc_f.b, cc_f.a);
	cairo_fill_preserve(cr);
	//! stroke
	const PvStrokeLinecapInfo *linecap_info = get_stroke_linecap_info_from_id(element->stroke.linecap);
	cairo_set_line_cap (cr, linecap_info->cairo_value);
	const PvStrokeLinejoinInfo *linejoin_info = get_stroke_linejoin_info_from_id(element->stroke.linejoin);
	cairo_set_line_join (cr, linejoin_info->cairo_value);
	PvCairoRgbaColor cc_s = pv_color_get_cairo_rgba(element->color_pair.colors[PvColorPairGround_ForGround]);
	cairo_set_source_rgba (cr, cc_s.r, cc_s.g, cc_s.b, cc_s.a);
	cairo_stroke_preserve(cr);

}

static char *_curve_new_str_from_anchor(const PvAnchorPoint ap_current, const PvAnchorPoint ap_prev)
{
	char *str = g_strdup_printf("%c %f %f %f %f %f %f",
			'C',
			ap_prev.points[PvAnchorPointIndex_Point].x
			+ ap_prev.points[PvAnchorPointIndex_HandleNext].x,
			ap_prev.points[PvAnchorPointIndex_Point].y
			+ ap_prev.points[PvAnchorPointIndex_HandleNext].y,
			ap_current.points[PvAnchorPointIndex_Point].x
			+ ap_current.points[PvAnchorPointIndex_HandlePrev].x,
			ap_current.points[PvAnchorPointIndex_Point].y
			+ ap_current.points[PvAnchorPointIndex_HandlePrev].y,
			ap_current.points[PvAnchorPointIndex_Point].x,
			ap_current.points[PvAnchorPointIndex_Point].y);

	return str;
}

static bool _node_add_stroke_props(xmlNodePtr node, PvStroke stroke)
{
	char *str = g_strdup_printf("%.3f", stroke.width);
	xmlNewProp(node, BAD_CAST "stroke-width", BAD_CAST str);
	g_free(str);

	const PvStrokeLinecapInfo *linecap_info = get_stroke_linecap_info_from_id(stroke.linecap);
	xmlNewProp(node, BAD_CAST "stroke-linecap", BAD_CAST linecap_info->name);

	const PvStrokeLinejoinInfo *linejoin_info = get_stroke_linejoin_info_from_id(stroke.linejoin);
	xmlNewProp(node, BAD_CAST "stroke-linejoin", BAD_CAST linejoin_info->name);

	return true;
}

static void _curve_draw_anchor_handle(
		cairo_t *cr,
		PvAnchorPoint ap,
		int ofs_index)
{
	PvPoint gp_current = ap.points[PvAnchorPointIndex_Point];
	PvPoint gp_prev = pv_anchor_point_get_handle(&ap, PvAnchorPointIndex_HandlePrev);
	PvPoint gp_next = pv_anchor_point_get_handle(&ap, PvAnchorPointIndex_HandleNext);

	if(0 == ofs_index){
		// ** current anchor_point
		cairo_set_line_width(cr, 1.0);
		pv_cairo_set_source_rgba_workingcolor(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_prev.x, gp_prev.y);
		cairo_stroke(cr);

		cairo_set_line_width(cr, 1.0);
		pv_cairo_set_source_rgba_workingcolor(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_next.x, gp_next.y);
		cairo_stroke(cr);

		_curve_draw_point(cr, gp_prev, PvElementPointKind_AnchorHandle);
		_curve_draw_point(cr, gp_next, PvElementPointKind_AnchorHandle);
	} else if(-1 == ofs_index) {
		// ** prev anchor_point
		cairo_set_line_width(cr, 1.0);
		pv_cairo_set_source_rgba_workingcolor(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_next.x, gp_next.y);
		cairo_stroke(cr);

		_curve_draw_point(cr, gp_next, PvElementPointKind_AnchorHandle);
	}else if(1 == ofs_index){
		// ** next anchor_point
		cairo_set_line_width(cr, 1.0);
		pv_cairo_set_source_rgba_workingcolor(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_prev.x, gp_prev.y);
		cairo_stroke(cr);

		_curve_draw_point(cr, gp_prev, PvElementPointKind_AnchorHandle);
	}else{
		pv_bug("%d", ofs_index);
	}
}

static void _anchor_point_rotate(PvAnchorPoint *ap, double degree, PvPoint center)
{
	PvPoint *pc = pv_anchor_point_get_point_ref(ap, PvAnchorPointIndex_Point);
	*pc = pv_rotate_point(*pc, degree, center);

	PvPoint *pp = pv_anchor_point_get_point_ref(ap, PvAnchorPointIndex_HandlePrev);
	*pp = pv_rotate_point(*pp, degree, (PvPoint){0,0});

	PvPoint *pn = pv_anchor_point_get_point_ref(ap, PvAnchorPointIndex_HandleNext);
	*pn = pv_rotate_point(*pn, degree, (PvPoint){0,0});
}

static void _curve_rotate(PvElement *element, double degree, PvPoint center)
{
	pv_assert(element);
	PvElementCurveData *data = element->data;
	pv_assert(data);

	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		_anchor_point_rotate(ap, degree, center);
	}
}

static gpointer _func_curve_new_data()
{
	PvElementCurveData *data = (PvElementCurveData *)malloc(sizeof(PvElementCurveData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->anchor_path = pv_anchor_path_new();

	return (gpointer)data;
}

static bool _func_curve_free_data(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementCurveData *data = (PvElementCurveData *)_data;

	pv_anchor_path_free(data->anchor_path);

	free(data);

	return true;
}

static gpointer _func_curve_copy_new_data(void *_data)
{
	if(NULL == _data){
		pv_bug("");
		return NULL;
	}

	PvElementCurveData *data = (PvElementCurveData *)_data;

	PvElementCurveData *new_data = (PvElementCurveData *)malloc(sizeof(PvElementCurveData));
	pv_assert(new_data);

	*new_data = *data;

	new_data->anchor_path = pv_anchor_path_copy_new(data->anchor_path);

	return (gpointer)new_data;
}

static int _func_curve_write_svg(
		InfoTargetSvg *target,
		const PvElement *element, const ConfWriteSvg *conf)
{
	if(NULL == target){
		pv_bug("");
		return -1;
	}

	if(NULL == target->xml_parent_node){
		pv_bug("");
		return -1;
	}

	if(NULL == element){
		pv_bug("");
		return -1;
	}

	if(NULL == conf){
		pv_bug("");
		return -1;
	}

	PvElementCurveData *data = (PvElementCurveData *)element->data;

	char *str_current = NULL;
	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap_ = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		const PvAnchorPoint ap = *ap_;

		char *str_point = NULL;
		if(0 == i){
			// first anchor_point
			str_point= g_strdup_printf("%c %f %f",
					(0 == i)? 'M':'L',
					ap.points[PvAnchorPointIndex_Point].x, 
					ap.points[PvAnchorPointIndex_Point].y
					);
		}else{
			// other (not first)(true last)
			const PvAnchorPoint *ap_prev_ = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, (i - 1), PvAnchorPathIndexTurn_Disable);
			const PvAnchorPoint ap_prev = *ap_prev_;
			str_point = _curve_new_str_from_anchor(ap, ap_prev);
		}

		if(NULL == str_point){
			pv_critical("");
			return -1;
		}

		char *str_prev = str_current;
		str_current = g_strdup_printf("%s %s",
				((NULL == str_prev)? "":str_prev),
				str_point
				);
		if(NULL == str_current){
			pv_critical("");
			return -1;
		}

		g_free(str_point);
		g_free(str_prev);
	}

	if(pv_anchor_path_get_is_close(data->anchor_path) && 0 < num){
		const PvAnchorPoint *ap_first = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, 0, PvAnchorPathIndexTurn_Disable);
		const PvAnchorPoint *ap_last = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, (num - 1), PvAnchorPathIndexTurn_Disable);
		char *str_end = _curve_new_str_from_anchor(*ap_first, *ap_last);
		char *str_prev = str_current;
		str_current = g_strjoin(" ", str_current, str_end, "Z", NULL);
		g_free(str_prev);
		if(NULL == str_current){
			pv_critical("");
			return -1;
		}
	}

	char *str_fg_color = pv_color_new_str_svg_rgba_simple(
			element->color_pair.colors[PvColorPairGround_ForGround]);
	assert(str_fg_color);
	char *str_bg_color = pv_color_new_str_svg_rgba_simple(
			element->color_pair.colors[PvColorPairGround_BackGround]);
	assert(str_bg_color);

	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "path");
	xmlNewProp(node, BAD_CAST "fill", BAD_CAST str_bg_color);
	xmlNewProp(node, BAD_CAST "stroke", BAD_CAST str_fg_color);
	xmlNewProp(node, BAD_CAST "d", BAD_CAST str_current);

	g_free(str_bg_color);
	g_free(str_fg_color);
	g_free(str_current);

	_node_add_stroke_props(node, element->stroke);

	xmlAddChild(target->xml_parent_node, node);
	//target->xml_parent_node = node;
	target->xml_new_node = node;

	return 0;
}

static bool _func_curve_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	PvElement *simplify = _curve_simplify_new(
			element,
			render_option.render_context,
			PvSimplifyOption_All);
	pv_assert(simplify);

	_func_curve_draw_inline(cr, simplify);
	cairo_new_path(cr);

	_curve_simplify_free(simplify);

	return true;
}

static bool _func_curve_draw_focusing(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	const PvRenderContext render_context = render_option.render_context;
	const PvFocus *focus = render_option.focus;

	const PvElementCurveData *data = element->data;
	pv_assert(data);

	PvElement *simplify = _curve_simplify_new(
			element,
			render_option.render_context,
			PvSimplifyOption_ResizeOnly);
	pv_assert(simplify);
	const PvElementCurveData *simplify_data = simplify->data;

	// ** stroke line
	_curve_command_path(cr, simplify);
	double LINE_WIDTH_FOCUS = 1.0;
	cairo_set_line_width(cr, LINE_WIDTH_FOCUS);
	pv_cairo_set_source_rgba_workingcolor(cr);
	cairo_stroke(cr);

	// ** anchor points
	int focus_index = pv_anchor_path_get_index_from_anchor_point(
			data->anchor_path, pv_focus_get_first_anchor_point(focus));
	size_t num = _func_curve_get_num_anchor_point(simplify);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
				data->anchor_path,
				i,
				PvAnchorPathIndexTurn_Disable);
		const PvAnchorPoint *ap_simplify = pv_anchor_path_get_anchor_point_from_index(
				simplify_data->anchor_path,
				i,
				PvAnchorPathIndexTurn_Disable);
		if(-1 != focus_index){
			int ofs_index = (i - focus_index);
			if(abs(ofs_index) <= 1){
				// * anchor handle. draw to focus and +-1
				_curve_draw_anchor_handle(cr, *ap_simplify, ofs_index);
			}
			if(0 == focus_index && i == ((int)num - 1) && pv_anchor_path_get_is_close(simplify_data->anchor_path)){
				// *anchor handle. to last AnchorPoint
				_curve_draw_anchor_handle(cr, *ap_simplify, -1);
			}
		}

		// draw AnchorPoint
		PvElementPointKind kind = PvElementPointKind_Normal;
		if(i == focus_index){
			kind = PvElementPointKind_Selected;
		}else if(pv_focus_is_exist_anchor_point(focus, NULL, ap)){
			kind = PvElementPointKind_SubSelected;
		}
		PvPoint p = ap_simplify->points[PvAnchorPointIndex_Point];
		_curve_draw_point(cr, p, kind);
	}

	// ** extent
	if(render_context.is_extent_view){
		_draw_extent_from_rect(cr, _func_curve_get_rect_by_anchor_points(simplify));
	}

	_curve_simplify_free(simplify);

	return true;
}

static bool _func_curve_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		int offset,
		double gx,
		double gy)
{
	*is_touch = false;

	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1,1);
	pv_assert(surface);
	cairo_t *cr = cairo_create (surface);
	pv_assert(cr);

	PvRenderContext render_context = PvRenderContext_Default;
	_curve_command_path(cr, element);

	double c_width = (element->stroke.width * render_context.scale) + offset;
	cairo_set_line_width(cr, c_width);

	//! @fixme bug fill area not detection.(down below side in fill.)
	*is_touch = cairo_in_stroke(cr, gx, gy) || cairo_in_fill(cr, gx, gy);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return true;
}

static bool _func_curve_is_overlap_rect(
		bool *is_overlap,
		const PvElement *element,
		int offset,
		PvRect rect)
{
	*is_overlap = false;

	const PvElementCurveData *data = element->data;
	pv_assert(data);

	rect = pv_rect_abs_size(rect);

	PvRect rect_offseted = {
		.x = rect.x - offset,
		.y = rect.y - offset,
		.w = rect.w + offset,
		.h = rect.h + offset,
	};

	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		PvPoint a_point = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_Point);
		if(pv_rect_is_inside(rect_offseted, a_point)){
			*is_overlap = true;
			return true;
		}
	}

	return true;
}

bool _func_curve_remove_delete_anchor_point(
		PvElement *element,
		const PvAnchorPoint *anchor_point,
		PvElement **p_foot_element,
		bool *is_deleted_element)
{
	*p_foot_element = NULL;
	*is_deleted_element = false;

	const PvElementCurveData *data = element->data;
	pv_assert(data);

	pv_assert(element->parent);

	if(pv_anchor_path_get_is_close(data->anchor_path)){
		int index = pv_anchor_path_get_index_from_anchor_point(data->anchor_path, anchor_point);
		pv_assert(0 <= index);

		bool ret = pv_anchor_path_reorder_open_from_index(data->anchor_path, index);
		pv_assert(ret);
		pv_anchor_path_set_is_close(data->anchor_path, false);
		pv_anchor_path_remove_delete_anchor_point(data->anchor_path, anchor_point);
		pv_anchor_path_remove_delete_range(data->anchor_path, 0, 0);
	}else{
		int index = pv_anchor_path_get_index_from_anchor_point(data->anchor_path, anchor_point);
		pv_assert(0 <= index);

		PvAnchorPath *anchor_path = pv_anchor_path_split_new_from_index_remove_delete(
				data->anchor_path,
				index);
		if(NULL != anchor_path){
			PvElement *element_2 = pv_element_curve_new_set_anchor_path(anchor_path);
			pv_assert(element_2);
			pv_element_copy_property(element_2, element);
			pv_element_append_child(element->parent, element, element_2);
			*p_foot_element = element_2;
		}
	}

	if(0 == pv_anchor_path_get_anchor_point_num(data->anchor_path)){
		pv_element_remove_free_recursive(element);
		*is_deleted_element = true;
	}

	return true;
}

static bool _func_curve_is_diff_one(
		bool *is_diff,
		const PvElement *element0,
		const PvElement *element1)
{
	const PvElementCurveData *data0 = element0->data;
	const PvElementCurveData *data1 = element1->data;
	if(NULL == data0 || NULL == data1){
		pv_bug("%p,%p", data0, data1);
		*is_diff = true;
		return false;
	}

	if(pv_anchor_path_is_diff(data0->anchor_path, data1->anchor_path)){
		*is_diff = true;
		return true;
	}

	*is_diff = false;
	return true;
}

static bool _func_curve_move_element(
		const PvElement *element,
		double gx,
		double gy)
{
	if(NULL == element || PvElementKind_Curve != element->kind){
		pv_error("%p", element);
		return false;
	}
	PvElementCurveData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}

	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		ap->points[PvAnchorPointIndex_Point].x += gx;
		ap->points[PvAnchorPointIndex_Point].y += gy;
	}

	return true;
}

int _func_curve_get_num_anchor_point(
		const PvElement *element)
{
	if(NULL == element || PvElementKind_Curve != element->kind){
		pv_error("%p", element);
		return false;
	}
	PvElementCurveData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}

	return pv_anchor_path_get_anchor_point_num(data->anchor_path);
}

static bool _func_curve_is_exist_anchor_point(
		const PvElement *element,
		const PvAnchorPoint *ap)
{
	PvElementCurveData *data = element->data;

	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap_ = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		if(ap_ == ap){
			return true;
		}
	}

	return false;
}

PvAnchorPoint *_func_curve_new_anchor_points(
		const PvElement *element)
{
	pv_bug("");
	return NULL;
}

static PvAnchorPoint *_func_curve_get_anchor_point(
		const PvElement *element,
		const int index)
{
	assert(element);
	assert(PvElementKind_Curve == element->kind);

	PvElementCurveData *data = element->data;

	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, index, PvAnchorPathIndexTurn_Disable);
	return ap;
}

static bool _func_curve_set_anchor_point_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint point)
{
	assert(element);
	assert(PvElementKind_Curve == element->kind);

	PvElementCurveData *data = element->data;
	assert(data);

	if(!pv_anchor_path_is_exist_anchor_point(data->anchor_path, ap)){
		pv_bug("");
		return false;
	}

	pv_anchor_point_set_point(ap, point);

	return true;
}

static bool _func_curve_move_anchor_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint move)
{
	assert(element);
	assert(PvElementKind_Curve == element->kind);

	PvElementCurveData *data = element->data;
	assert(data);

	if(!pv_anchor_path_is_exist_anchor_point(data->anchor_path, ap)){
		pv_bug("");
		return false;
	}

	ap->points[PvAnchorPointIndex_Point].x += move.x;
	ap->points[PvAnchorPointIndex_Point].y += move.y;

	return true;
}

static PvRect _func_curve_get_rect_by_anchor_points(
		const PvElement *element)
{
	const PvElementCurveData *data = (PvElementCurveData *)element->data;

	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	pv_assert(0 < num);

	PvPoint min = (PvPoint){0, 0};
	PvPoint max = (PvPoint){0, 0};
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		PvPoint point = ap->points[PvAnchorPointIndex_Point];
		if(0 == i){
			min = point;
			max = point;
		}else{
			min.x = (min.x < point.x)? min.x : point.x;
			min.y = (min.y < point.y)? min.y : point.y;
			max.x = (max.x > point.x)? max.x : point.x;
			max.y = (max.y > point.y)? max.y : point.y;
		}
	}

	PvRect rect = (PvRect){min.x, min.y, (max.x - min.x), (max.y - min.y)};

	return rect;
}

static PvPoint _curve_get_point_by_anchor_points(
		const PvElement *element)
{
	assert(element);
	assert(element->data);
	const PvElementCurveData *data = element->data;

	PvPoint point = PvPoint_Default;
	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		PvPoint a_point = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_Point);

		if(0 == i){
			point = a_point;
		}else{
			point.x = (point.x < a_point.x)? point.x : a_point.x;
			point.y = (point.y < a_point.y)? point.y : a_point.y;
		}
	}

	return point;
}

static void _curve_set_point_by_anchor_points(
		PvElement *element,
		const PvPoint point)
{
	assert(element);
	assert(element->data);
	PvElementCurveData *data = element->data;


	PvPoint point_prev = _curve_get_point_by_anchor_points(element);
	PvPoint move = pv_point_sub(point, point_prev);

	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);

		pv_anchor_point_move_point(ap, move);
	}

	return;
}


static bool _func_curve_set_rect_by_anchor_points(
		PvElement *element,
		PvRect rect)
{
	const PvElementCurveData *data = (PvElementCurveData *)element->data;
	pv_assert(data);

	const PvRect rect_src = _func_curve_get_rect_by_anchor_points(element);

	PvPoint point = {.x = rect.x, .y = rect.y,};
	_curve_set_point_by_anchor_points(element, point);

	PvPoint scale = {
		.x = rect.w / rect_src.w,
		.y = rect.h / rect_src.h,
	};
	if(rect_src.w < DELTA_OF_RESIZE){
		scale.x = 1;
	}
	if(rect_src.h < DELTA_OF_RESIZE){
		scale.y = 1;
	}


	size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		pv_anchor_point_rescale(ap, scale, point);
	}

	return true;
}

static PvRect _func_curve_get_rect_by_draw(
		const PvElement *element)
{
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1,1);
	pv_assert(surface);
	cairo_t *cr = cairo_create (surface);
	pv_assert(cr);

	_func_curve_draw_inline(cr, element);

	PvRect rect = _get_rect_extent_from_cr(cr);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return rect;
}

static void _func_curve_apply_appearances(
		PvElement *element,
		PvAppearance **appearances)
{
	assert(element);
	PvElementCurveData *data = element->data;
	assert(data);

	size_t num = pv_general_get_parray_num((void **)appearances);
	for(int i = 0; i < (int)num; i++){
		switch(appearances[i]->kind){
			case PvAppearanceKind_None:
				// End of fppearances
				return;
				break;
			case PvAppearanceKind_Translate:
				{
					PvPoint move = appearances[i]->translate.move;
					_func_curve_move_element(element, move.x, move.y);
				}
				break;
			case PvAppearanceKind_Resize:
				{
					PvRect src_rect = _func_curve_get_rect_by_anchor_points(element);
					PvRect dst_rect = src_rect;
					dst_rect.w *= appearances[i]->resize.resize.x;
					dst_rect.h *= appearances[i]->resize.resize.y;
					dst_rect.x += (src_rect.w - dst_rect.w) / 2;
					dst_rect.y += (src_rect.h - dst_rect.h) / 2;
					_func_curve_set_rect_by_anchor_points(element, dst_rect);
				}
				break;
			case PvAppearanceKind_Rotate:
				{
					PvRect rect = _func_curve_get_rect_by_anchor_points(element);
					PvPoint center = pv_rect_get_center(rect);
					double degree = appearances[i]->rotate.degree;
					_curve_rotate(element, degree, center);
				}
				break;
			default:
				pv_bug("Not implement. %d %d", i, appearances[i]->kind);
				return;
		}
	}

	return;
}



/* ****************
 * Raster
 **************** */

static gpointer _func_raster_new_data()
{
	PvElementRasterData *data = (PvElementRasterData *)malloc(sizeof(PvElementRasterData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->path = NULL;
	data->pixbuf = NULL;

	data->raster_appearances = pv_appearance_parray_new_from_num(Num_PvElementRasterAppearance);
	pv_assert(data->raster_appearances);
	data->raster_appearances[PvElementRasterAppearanceIndex_Translate]->kind = PvAppearanceKind_Translate;
	data->raster_appearances[PvElementRasterAppearanceIndex_Resize]->kind = PvAppearanceKind_Resize;
	data->raster_appearances[PvElementRasterAppearanceIndex_Resize]->resize.resize = (PvPoint){1,1};
	data->raster_appearances[PvElementRasterAppearanceIndex_Rotate]->kind = PvAppearanceKind_Rotate;

	data->anchor_path = pv_anchor_path_new();
	pv_assert(data->anchor_path);
	PvAnchorPoint ap = PvAnchorPoint_Default;
	for(int i = 0; i < 4; i++){
		pv_anchor_path_add_anchor_point(data->anchor_path, &ap);
	}

	return (gpointer)data;
}

static bool _func_raster_free_data(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementRasterData *data = (PvElementRasterData *)_data;

	if(NULL != data->path){
		free(data->path);
	}

	if(NULL != data->pixbuf){
		g_object_unref(G_OBJECT(data->pixbuf));
	}

	pv_appearance_parray_free(data->raster_appearances);

	free(data);

	return true;
}

static gpointer _func_raster_copy_new_data(void *_data)
{
	if(NULL == _data){
		pv_bug("");
		return NULL;
	}

	PvElementRasterData *data = (PvElementRasterData *)_data;

	PvElementRasterData *new_data = (PvElementRasterData *)malloc(sizeof(PvElementRasterData));
	pv_assert(new_data);

	*new_data = *data;

	new_data->path = pv_general_new_str(data->path);

	if(NULL != new_data->pixbuf){
		g_object_ref(G_OBJECT(new_data->pixbuf));
	}

	if(NULL != data->raster_appearances){
		new_data->raster_appearances = pv_appearance_parray_copy_new(data->raster_appearances);
		pv_assert(new_data->raster_appearances);
	}

	return (gpointer)new_data;
}

static void _func_raster_apply_appearances(
		PvElement *element,
		PvAppearance **appearances);

static PvElement *_raster_simplify_new(const PvElement *element, PvRenderContext render_context)
{
	pv_assert(element);

	PvElement *simplify = pv_element_copy_recursive(element);
	pv_assert(simplify);
	PvElementRasterData *simplify_data = simplify->data;
	pv_assert(simplify_data);

	// appearance
	_func_raster_apply_appearances(simplify, element->etaion_work_appearances);
	simplify->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

	// scale
	PvPoint *move = &(simplify_data->raster_appearances
			[PvElementRasterAppearanceIndex_Translate]->translate.move);
	*move = pv_point_mul_value(*move, render_context.scale);
	PvPoint *resize = &(simplify_data->raster_appearances
			[PvElementRasterAppearanceIndex_Resize]->resize.resize);
	*resize = pv_point_mul_value(*resize, render_context.scale);

	return simplify;
}

void _raster_simplify_free(PvElement *self)
{
	pv_element_remove_free_recursive(self);
}

static PvPoint get_pixbuf_size_(const GdkPixbuf *pb)
{
	PvPoint ret = {
		.x = gdk_pixbuf_get_width(pb),
		.y = gdk_pixbuf_get_height(pb),
	};

	return ret;
}

typedef enum{
	RasterDrawKind_Draw,
	RasterDrawKind_DrawFocusing,
	RasterDrawKind_IsTouch,
}RasterDrawKind;

static bool _raster_draw_inline(
		cairo_t *cr,
		const PvRenderContext render_context,
		const PvElement *element,
		double line_width,
		RasterDrawKind rasterDrawKind
		)
{
	bool result = true;

	pv_assert(element);
	PvElementRasterData *data = element->data;
	pv_assert(data);
	pv_assertf(data->pixbuf, "%p", data->pixbuf);

	PvElement *simplify = _raster_simplify_new(element, render_context);
	pv_assert(simplify);
	PvElementRasterData *simplify_data = simplify->data;
	pv_assert(simplify_data);

	PvPoint position = simplify_data->raster_appearances
		[PvElementRasterAppearanceIndex_Translate]->translate.move;
	PvPoint resize = simplify_data->raster_appearances
		[PvElementRasterAppearanceIndex_Resize]->resize.resize;
	double degree = simplify_data->raster_appearances
		[PvElementRasterAppearanceIndex_Rotate]->rotate.degree;

	PvPoint size = get_pixbuf_size_(data->pixbuf);
	size = pv_point_mul(size, resize);

	// effectively invisible to not draw
	const int PX_RASTER_MINIMUM = 0.00001;
	if(fabs(size.x)  < PX_RASTER_MINIMUM || fabs(size.y) < PX_RASTER_MINIMUM){
		pv_debug("%f, %f", size.x, size.y);
		goto finally;
	}

	// rotate
	cairo_save(cr);
	PvPoint center = {
		.x = position.x + (size.x / 2),
		.y = position.y + (size.y / 2),
	};
	cairo_translate(cr, center.x, center.y);
	cairo_rotate(cr, degree * (M_PI/180));
	cairo_translate(cr, -center.x, -center.y);

	// draw
	switch(rasterDrawKind){
		case RasterDrawKind_Draw:
			{
				cairo_translate(cr, position.x, position.y);
				cairo_scale(cr, resize.x, resize.y);

				gdk_cairo_set_source_pixbuf (cr, simplify_data->pixbuf, 0, 0);
				cairo_paint (cr);
			}
			break;
		case RasterDrawKind_DrawFocusing:
			{
				cairo_rectangle(cr, position.x, position.y, size.x, size.y);
				cairo_set_line_width(cr, line_width);
				pv_cairo_set_source_rgba_workingcolor(cr);
				cairo_stroke(cr);
			}
			break;
		case RasterDrawKind_IsTouch:
			{
				cairo_rectangle(cr, position.x, position.y, size.x, size.y);
				cairo_set_line_width(cr, line_width);
				pv_cairo_set_source_rgba_workingcolor(cr);
				cairo_stroke_preserve(cr);
				cairo_fill_preserve(cr);
			}
			break;
		default:
			pv_assertf(false, "%d", rasterDrawKind);
			break;
	}
	cairo_restore(cr); // end rorate

finally:
	_raster_simplify_free(simplify);

	return result;
}

static bool _func_raster_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	bool ret = _raster_draw_inline(cr, render_option.render_context, element, 0, RasterDrawKind_Draw);
	return ret;
}

static bool _func_raster_draw_focusing(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	bool ret = _raster_draw_inline(cr, render_option.render_context, element, 1.0, RasterDrawKind_DrawFocusing);
	return ret;
}

static bool _func_raster_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		int offset,
		double gx,
		double gy)
{
	*is_touch = false;

	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1,1);
	pv_assert(surface);
	cairo_t *cr = cairo_create (surface);
	pv_assert(cr);

	PvRenderContext render_context = PvRenderContext_Default;
	double c_width = (element->stroke.width * render_context.scale) + offset;
	bool ret = _raster_draw_inline(cr, render_context, element, c_width, RasterDrawKind_IsTouch);
	pv_assert(ret);

	//! @fixme bug fill area not detection.(down below side in fill.)
	*is_touch = cairo_in_stroke(cr, gx, gy) || cairo_in_fill(cr, gx, gy);

	cairo_surface_destroy (surface);

	return true;
}

static bool _func_raster_is_overlap_rect(
		bool *is_overlap,
		const PvElement *element,
		int offset,
		PvRect rect)
{
	*is_overlap = false;

	rect = pv_rect_abs_size(rect);

	return true;
}

static bool _func_raster_is_diff_one(
		bool *is_diff,
		const PvElement *element0,
		const PvElement *element1)
{
	const PvElementRasterData *data0 = element0->data;
	const PvElementRasterData *data1 = element1->data;
	if(NULL == data0 || NULL == data1){
		pv_bug("%p,%p", data0, data1);
		*is_diff = true;
		return false;
	}

	if(pv_appearance_parray_is_diff(data0->raster_appearances, data1->raster_appearances)){
		*is_diff = true;
		return true;
	}

	// ** dataX->path is not check.

	// ** TODO: check raster pixbuf.
	if(data0->pixbuf != data1->pixbuf){
		*is_diff = true;
		return true;
	}

	*is_diff = false;
	return true;
}

static bool _func_raster_move_element(
		const PvElement *element,
		double gx,
		double gy)
{
	if(NULL == element || PvElementKind_Raster != element->kind){
		pv_error("%p", element);
		return false;
	}
	PvElementRasterData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}

	PvPoint position = data->raster_appearances[PvElementRasterAppearanceIndex_Translate]->translate.move;
	position.x += gx;
	position.y += gy;
	data->raster_appearances[PvElementRasterAppearanceIndex_Translate]->translate.move = position;

	return true;
}

static PvAnchorPoint *_func_raster_get_anchor_point(
		const PvElement *element,
		const int index)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	PvPoint position = data->raster_appearances[PvElementRasterAppearanceIndex_Translate]->translate.move;

	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, index, PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);

	PvPoint point = {
		.x = position.x,
		.y = position.y,
	};
	pv_anchor_point_set_handle(ap, PvAnchorPointIndex_Point, point);

	return ap;
}

static bool _func_raster_set_anchor_point_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint point)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	data->raster_appearances
		[PvElementRasterAppearanceIndex_Translate]->translate.move
		= point;

	return true;
}

static bool _func_raster_move_anchor_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint move)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	//! @todo not implement, already implement is only upleft anchor_point.
	{
		PvPoint position = data->raster_appearances[PvElementRasterAppearanceIndex_Translate]->translate.move;
		position.x += move.x;
		position.y += move.y;
		data->raster_appearances[PvElementRasterAppearanceIndex_Translate]->translate.move = position;
	}

	return true;
}

static PvRect pv_rect_expand_from_point_(PvRect rect, PvPoint point, bool is_init)
{
	if(is_init){
		rect.x = point.x;
		rect.y = point.y;
		rect.w = 0;
		rect.h = 0;
	}else{
		PvPoint dr = {
			.x = rect.x + rect.w,
			.y = rect.y + rect.h,
		};
		rect.x = (rect.x < point.x)? rect.x : point.x;
		rect.y = (rect.y < point.y)? rect.y : point.y;
		dr.x = (dr.x > point.x)? dr.x : point.x;
		dr.y = (dr.y > point.y)? dr.y : point.y;
		rect.w = dr.x - rect.x;
		rect.h = dr.y - rect.y;
	}

	return rect;
}

static PvRect _func_raster_get_rect_by_anchor_points(
		const PvElement *element)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	PvPoint position = data->raster_appearances[PvElementRasterAppearanceIndex_Translate]->translate.move;
	PvPoint resize = data->raster_appearances[PvElementRasterAppearanceIndex_Resize]->resize.resize;

	PvRect rect = PvRect_Default;
	rect.x = position.x;
	rect.y = position.y;
	rect.w = gdk_pixbuf_get_width(data->pixbuf) * resize.x;
	rect.h = gdk_pixbuf_get_height(data->pixbuf) * resize.y;

	PvPoint center = pv_rect_get_center(rect);
	PvRect expand = PvRect_Default;
	for(int i = 0; i < 4; i++){
		PvPoint p = pv_rect_get_edge_point(rect, i);
		p = pv_rotate_point(
				p, 
				data->raster_appearances[PvElementRasterAppearanceIndex_Rotate]->rotate.degree,
				center);
		expand = pv_rect_expand_from_point_(expand, p, i == 0);
	}

	return expand;
}

static bool _func_raster_set_rect_by_anchor_points(
		PvElement *element,
		PvRect rect)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	PvRect src_rect = _func_raster_get_rect_by_anchor_points(element);

	data->raster_appearances[PvElementRasterAppearanceIndex_Translate]->translate.move
		= (PvPoint){.x = rect.x, .y = rect.y};
	PvPoint scale_ = {
		.x = rect.w / src_rect.w,
		.y = rect.h / src_rect.h,
	};
	PvPoint *resize = &(data->raster_appearances[PvElementRasterAppearanceIndex_Resize]->resize.resize);
	*resize = pv_point_mul(*resize, scale_);

	return true;
}

static void _func_raster_apply_appearances(
		PvElement *element,
		PvAppearance **appearances)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);
	PvElementRasterData *data = element->data;
	assert(data);

	size_t num = pv_general_get_parray_num((void **)appearances);
	for(int i = 0; i < (int)num; i++){
		switch(appearances[i]->kind){
			case PvAppearanceKind_None:
				// End of fppearances
				return;
				break;
			case PvAppearanceKind_Translate:
				{
					PvPoint *move = &(data->raster_appearances
							[PvElementRasterAppearanceIndex_Translate]->translate.move);
					*move = pv_point_add(*move, appearances[i]->translate.move);
				}
				break;
			case PvAppearanceKind_Resize:
				{
					PvPoint *resize = &(data->raster_appearances
							[PvElementRasterAppearanceIndex_Resize]->resize.resize);
					PvPoint img_size = get_pixbuf_size_(data->pixbuf);
					PvPoint src_size = pv_point_mul(img_size, *resize);

					*resize = pv_point_mul(*resize, appearances[i]->resize.resize);

					PvPoint dst_size = pv_point_mul(img_size, *resize);

					PvPoint *move = &(data->raster_appearances
							[PvElementRasterAppearanceIndex_Translate]->translate.move);
					PvPoint diff = pv_point_div_value(pv_point_sub(src_size, dst_size), 2);
					*move = pv_point_add(*move, diff);
				}
				break;
			case PvAppearanceKind_Rotate:
				{
					data->raster_appearances[PvElementRasterAppearanceIndex_Rotate]
						->rotate.degree += appearances[i]->rotate.degree;
				}
				break;
			default:
				pv_bug("Not implement. %d %d", i, appearances[i]->kind);
				return;
		}
	}

	return;
}

/* ****************
 * ElementInfo配列の定義
 **************** */

const PvElementInfo _pv_element_infos[] = {
	{PvElementKind_Root, "Root",
		.func_new_data				= _func_group_new_data,
		.func_free_data				= _func_group_free_data,
		.func_copy_new_data			= _func_group_copy_new_data,
		.func_write_svg				= _func_group_write_svg,
		.func_draw				= _func_group_draw,
		.func_draw_focusing			= _func_group_draw,
		.func_is_touch_element			= _func_group_is_touch_element,
		.func_is_overlap_rect			= _func_nop_is_overlap_rect,
		.func_remove_delete_anchor_point	= _func_nop_remove_delete_anchor_point,
		.func_is_diff_one			= _func_group_is_diff_one,
		.func_move_element			= _func_group_move_element,
		.func_get_num_anchor_point		= _func_zero_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_nop_is_exist_anchor_point,
		.func_new_anchor_points			= _func_null_new_anchor_points,
		.func_get_anchor_point			= _func_notimpl_get_anchor_point,
		.func_set_anchor_point_point		= _func_notimpl_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_notimpl_move_anchor_point,
		.func_get_rect_by_anchor_points		= _func_notimpl_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_notimpl_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_notimpl_get_rect_by_draw,
		.func_apply_appearances			= _func_nop_apply_appearances,
	},
	{PvElementKind_Layer, "Layer",
		.func_new_data				= _func_group_new_data,
		.func_free_data				= _func_group_free_data,
		.func_copy_new_data			= _func_group_copy_new_data,
		.func_write_svg				= _func_group_write_svg,
		.func_draw				= _func_group_draw,
		.func_draw_focusing			= _func_group_draw,
		.func_is_touch_element			= _func_group_is_touch_element,
		.func_is_overlap_rect			= _func_nop_is_overlap_rect,
		.func_remove_delete_anchor_point	= _func_nop_remove_delete_anchor_point,
		.func_is_diff_one			= _func_group_is_diff_one,
		.func_move_element			= _func_group_move_element,
		.func_get_num_anchor_point		= _func_zero_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_nop_is_exist_anchor_point,
		.func_new_anchor_points			= _func_null_new_anchor_points,
		.func_get_anchor_point			= _func_notimpl_get_anchor_point,
		.func_set_anchor_point_point		= _func_notimpl_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_notimpl_move_anchor_point,
		.func_get_rect_by_anchor_points		= _func_notimpl_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_notimpl_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_notimpl_get_rect_by_draw,
		.func_apply_appearances			= _func_nop_apply_appearances,
	},
	{PvElementKind_Group, "Group",
		.func_new_data				= _func_group_new_data,
		.func_free_data				= _func_group_free_data,
		.func_copy_new_data			= _func_group_copy_new_data,
		.func_write_svg				= _func_group_write_svg,
		.func_draw				= _func_group_draw,
		.func_draw_focusing			= _func_group_draw,
		.func_is_touch_element			= _func_group_is_touch_element,
		.func_is_overlap_rect			= _func_nop_is_overlap_rect,
		.func_remove_delete_anchor_point	= _func_nop_remove_delete_anchor_point,
		.func_is_diff_one			= _func_group_is_diff_one,
		.func_move_element			= _func_group_move_element,
		.func_get_num_anchor_point		= _func_zero_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_nop_is_exist_anchor_point,
		.func_new_anchor_points			= _func_null_new_anchor_points,
		.func_get_anchor_point			= _func_notimpl_get_anchor_point,
		.func_set_anchor_point_point		= _func_notimpl_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_notimpl_move_anchor_point,
		.func_get_rect_by_anchor_points		= _func_notimpl_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_notimpl_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_notimpl_get_rect_by_draw,
		.func_apply_appearances			= _func_nop_apply_appearances,
	},
	{PvElementKind_Curve, "Curve",
		.func_new_data				= _func_curve_new_data,
		.func_free_data				= _func_curve_free_data,
		.func_copy_new_data			= _func_curve_copy_new_data,
		.func_write_svg				= _func_curve_write_svg,
		.func_draw				= _func_curve_draw,
		.func_draw_focusing			= _func_curve_draw_focusing,
		.func_is_touch_element			= _func_curve_is_touch_element,
		.func_is_overlap_rect			= _func_curve_is_overlap_rect,
		.func_remove_delete_anchor_point	= _func_curve_remove_delete_anchor_point,
		.func_is_diff_one			= _func_curve_is_diff_one,
		.func_move_element			= _func_curve_move_element,
		.func_get_num_anchor_point		= _func_curve_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_curve_is_exist_anchor_point,
		.func_new_anchor_points			= _func_curve_new_anchor_points,
		.func_get_anchor_point			= _func_curve_get_anchor_point,
		.func_set_anchor_point_point		= _func_curve_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_curve_move_anchor_point,
		.func_get_rect_by_anchor_points		= _func_curve_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_curve_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_curve_get_rect_by_draw,
		.func_apply_appearances			= _func_curve_apply_appearances,
	},
	{PvElementKind_Raster, "Raster",
		.func_new_data				= _func_raster_new_data,
		.func_free_data				= _func_raster_free_data,
		.func_copy_new_data			= _func_raster_copy_new_data,
		.func_write_svg				= _func_notimpl_write_svg,
		.func_draw				= _func_raster_draw,
		.func_draw_focusing			= _func_raster_draw_focusing,
		.func_is_touch_element			= _func_raster_is_touch_element,
		.func_is_overlap_rect			= _func_raster_is_overlap_rect,
		.func_remove_delete_anchor_point	= _func_nop_remove_delete_anchor_point,
		.func_is_diff_one			= _func_raster_is_diff_one,
		.func_move_element			= _func_raster_move_element,
		.func_get_num_anchor_point		= _func_zero_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_nop_is_exist_anchor_point,
		.func_new_anchor_points			= _func_null_new_anchor_points,
		.func_get_anchor_point			= _func_raster_get_anchor_point,
		.func_set_anchor_point_point		= _func_raster_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_raster_move_anchor_point,
		.func_get_rect_by_anchor_points		= _func_raster_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_raster_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_notimpl_get_rect_by_draw,
		.func_apply_appearances			= _func_raster_apply_appearances,
	},
};



/* ****************
 * ElementInfo関連関数の定義
 **************** */

const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind)
{
	size_t num = sizeof(_pv_element_infos) / sizeof(PvElementInfo);
	for(int i = 0; i < (int)num; i++){
		if(kind == _pv_element_infos[i].kind){
			return &_pv_element_infos[i];
		}
	}

	pv_bug("%d", kind);
	return NULL;
}

