#include "pv_element_info.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pv_error.h"
#include "pv_render_context.h"
#include "pv_color.h"
#include "pv_cairo.h"
#include "pv_rotate.h"
#include "pv_basic_shape_info.h"



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
// ******** ********


static const char *_func_default_get_kind_name(
		const PvElement *element)
{
	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assertf(info, "%d", element->kind);

	return info->name;
}

static bool _func_nop_write_svg_after(
		InfoTargetSvg *target,
		const PvElement *element,
		PvElementWriteSvgContext *element_write_svg_context,
		const ConfWriteSvg *conf)
{
	return true;
}

static PvElementDrawRecursive _func_nop_draw_after(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
		const PvRenderOption render_option,
		const PvElement *element)
{
	return PvElementDrawRecursive_Continues;
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
		pv_debug("len:%zu '%s'", strlen(src), src);
	}

	char *dst = (char *)malloc(sizeof(char) * (strlen(src) + 1));
	pv_assertf(dst, "%s", src);

	strcpy(dst, src);

	return dst;
}

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
	data->kind = PvElementGroupKind_Normal;
	data->cairo_fill_rule = CAIRO_FILL_RULE_EVEN_ODD;

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

static bool _func_group_write_svg(
		InfoTargetSvg *target,
		const PvElement *element,
		PvElementWriteSvgContext *element_write_svg_context,
		const ConfWriteSvg *conf)
{
	pv_assert(target);
	pv_assert(target->xml_parent_node);
	pv_assert(element);
	pv_assert(conf);

	if(PvElementKind_Root == element->kind){
		return true;
	}

	PvElementGroupData *data = (PvElementGroupData *)element->data;
	switch(data->kind){
		case PvElementGroupKind_MaskCurveSimple:
			element_write_svg_context->element_group_kind = PvElementGroupKind_MaskCurveSimple;
			return true;
			break;
		case PvElementGroupKind_Normal:
			// NOP
		default:
			break;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assertf(info, "%d", element->kind);

	xmlNodePtr node = NULL;
	node = xmlNewNode(NULL, BAD_CAST "g");
	pv_assert(node);

	// ** To Inkscape
	if(PvElementKind_Layer == element->kind){
		xmlNewProp(node, BAD_CAST "inkscape:groupmode", BAD_CAST "layer");
	}
	xmlNewProp(node, BAD_CAST "inkscape:label", BAD_CAST info->name);

	xmlAddChild(target->xml_parent_node, node);
	target->xml_parent_node = node;
	target->xml_new_node = node;

	return true;
}

static bool _func_group_write_svg_after(
		InfoTargetSvg *target,
		const PvElement *element,
		PvElementWriteSvgContext *element_write_svg_context,
		const ConfWriteSvg *conf)
{
	PvElementGroupData *data = (PvElementGroupData *)element->data;
	switch(data->kind){
		case PvElementGroupKind_MaskCurveSimple:
			element_write_svg_context->element_group_kind = PvElementGroupKind_MaskCurveSimple;
			break;
		default:
			break;
	}
	return true;
}

static void _func_curve_draw_inline(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
		const PvElement *element);
static void _curve_command_path(
		cairo_t *cr,
		const PvElement *element);

static PvElementDrawRecursive _func_group_draw(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
		const PvRenderOption render_option,
		const PvElement *element)
{
	PvElementGroupData *data = (PvElementGroupData *)element->data;

	if(PvElementGroupKind_MaskCurveSimple == data->kind){
		element_render_context->element_group_kind = data->kind;
		element_render_context->nest_mask_curve_simple++;

		cairo_set_fill_rule(cr, data->cairo_fill_rule);
	}

	return PvElementDrawRecursive_Continues;
}

static PvElementDrawRecursive _func_group_draw_after(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
		const PvRenderOption render_option,
		const PvElement *element)
{
	PvElementGroupData *data = (PvElementGroupData *)element->data;

	if(PvElementGroupKind_MaskCurveSimple == data->kind){
		element_render_context->nest_mask_curve_simple--;
		//! @todo pop parent group fill_rule?
		if(0 == element_render_context->nest_mask_curve_simple){
			// exit masking
			element_render_context->element_group_kind = PvElementGroupKind_Normal;

			cairo_set_fill_rule(cr, CAIRO_FILL_RULE_WINDING); // cairo default
		}
	}
	return PvElementDrawRecursive_Continues;
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
		PvElement *element,
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
		PvElement *element,
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
	size_t num = pv_anchor_path_get_anchor_point_num(simplify->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvPoint point = {0, 0};
		PvPoint scale = {render_context.scale, render_context.scale};
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(simplify->anchor_path, i, PvAnchorPathIndexTurn_Disable);
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
	cairo_set_source_rgba (cr, 0.9, 0.2, 0.4, 0.4);
	cairo_fill(cr);
}

static void _curve_command_path(
		cairo_t *cr,
		const PvElement *element)
{
	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	if(0 == num){
		return;
	}

	// ** path stroking
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		PvPoint point = pv_anchor_point_get_point(ap);
		if(0 == i){
			if(1 == num){
				cairo_rectangle (cr, point.x, point.y, 2, 2);
				cairo_fill (cr);
			}else{
				cairo_move_to(cr, point.x, point.y);
			}
		}else{
			const PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, (i - 1), PvAnchorPathIndexTurn_Disable);

			PvPoint first_point = pv_anchor_point_get_handle(ap_prev, PvAnchorPointIndex_HandleNext);
			PvPoint second_point = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_HandlePrev);

			cairo_curve_to(
					cr,
					first_point.x, first_point.y,
					second_point.x,  second_point.y,
					point.x, point.y);
		}

	}
	if(pv_anchor_path_get_is_close(element->anchor_path)){
		PvPoint point = pv_anchor_point_get_point(pv_anchor_path_get_anchor_point_from_index(element->anchor_path, 0, PvAnchorPathIndexTurn_Disable));

		const PvAnchorPoint *ap_last = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, ((int)num - 1), PvAnchorPathIndexTurn_Disable);
		PvPoint first_point = pv_anchor_point_get_handle(ap_last, PvAnchorPointIndex_HandleNext);

		const PvAnchorPoint *ap_first = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, 0, PvAnchorPathIndexTurn_Disable);
		PvPoint second_point = pv_anchor_point_get_handle(ap_first, PvAnchorPointIndex_HandlePrev);

		cairo_curve_to(
				cr,
				first_point.x, first_point.y,
				second_point.x,  second_point.y,
				point.x, point.y);
		cairo_close_path (cr);
	}
}

void storke_and_fill_(cairo_t *cr, const PvElement *element)
{
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

static void _func_curve_draw_inline(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
		const PvElement *element)
{
	_curve_command_path(cr, element);
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

static void node_add_color_props_(xmlNodePtr node, const PvElement *element)
{
	char *str_fg_color = pv_color_new_str_svg_rgba_simple(
			element->color_pair.colors[PvColorPairGround_ForGround]);
	pv_assert(str_fg_color);
	char *str_bg_color = pv_color_new_str_svg_rgba_simple(
			element->color_pair.colors[PvColorPairGround_BackGround]);
	pv_assert(str_bg_color);

	xmlNewProp(node, BAD_CAST "fill", BAD_CAST str_bg_color);
	xmlNewProp(node, BAD_CAST "stroke", BAD_CAST str_fg_color);

	g_free(str_bg_color);
	g_free(str_fg_color);
}

static bool node_add_stroke_props_(xmlNodePtr node, PvStroke stroke)
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

	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
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

	return (gpointer)data;
}

static bool _func_curve_free_data(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementCurveData *data = (PvElementCurveData *)_data;

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

	return (gpointer)new_data;
}

static bool _func_curve_write_svg(
		InfoTargetSvg *target,
		const PvElement *element,
		PvElementWriteSvgContext *element_write_svg_context,
		const ConfWriteSvg *conf)
{
	pv_assert(target);
	pv_assert(target->xml_parent_node);
	pv_assert(element);
	pv_assert(conf);

	char *str_current = NULL;
	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap_ = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
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
			const PvAnchorPoint *ap_prev_ = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, (i - 1), PvAnchorPathIndexTurn_Disable);
			pv_assert(ap_prev_);
			const PvAnchorPoint ap_prev = *ap_prev_;
			str_point = _curve_new_str_from_anchor(ap, ap_prev);
		}
		pv_assert(str_point);

		char *str_prev = str_current;
		str_current = g_strdup_printf("%s %s",
				((NULL == str_prev)? "":str_prev),
				str_point
				);
		pv_assert(str_current);

		g_free(str_point);
		g_free(str_prev);
	}

	if(pv_anchor_path_get_is_close(element->anchor_path) && 0 < num){
		const PvAnchorPoint *ap_first = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, 0, PvAnchorPathIndexTurn_Disable);
		const PvAnchorPoint *ap_last = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, (num - 1), PvAnchorPathIndexTurn_Disable);
		char *str_end = _curve_new_str_from_anchor(*ap_first, *ap_last);
		char *str_prev = str_current;
		str_current = g_strjoin(" ", str_current, str_end, "Z", NULL);
		g_free(str_prev);
		pv_assert(str_current);
	}

	bool is_append = false;
	switch(element_write_svg_context->element_group_kind){
		case PvElementGroupKind_MaskCurveSimple:
			{
				if(element != element->parent->childs[0]){
					is_append = true;
				}
			}
			break;
		default:
			break;
	}

	if(is_append){
		xmlNodePtr node = target->xml_new_node;
		xmlChar *prop = xmlGetProp(node, BAD_CAST "d");
		if(NULL == prop){
			pv_warning("");
			return false;
		}
		char *str_prev = str_current;
		str_current = g_strdup_printf("%s %s",
				prop, str_prev);
		pv_assert(str_current);
		xmlSetProp(node, BAD_CAST "d", (xmlChar *)str_current);
	}else{
		xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "path");
		pv_assert(node);

		switch(element_write_svg_context->element_group_kind){
			case PvElementGroupKind_MaskCurveSimple:
				{
					PvElementGroupData *group_data = element->parent->data;
					if(CAIRO_FILL_RULE_EVEN_ODD == group_data->cairo_fill_rule){
						xmlNewProp(node, BAD_CAST "fill-rule", BAD_CAST "evenodd");
					}
				}
				break;
			default:
				break;
		}

		xmlNewProp(node, BAD_CAST "d", BAD_CAST str_current);
		node_add_color_props_(node, element);
		node_add_stroke_props_(node, element->stroke);

		xmlAddChild(target->xml_parent_node, node);
		//target->xml_parent_node = node;
		target->xml_new_node = node;
	}

	g_free(str_current);

	return true;
}

static PvElementDrawRecursive _func_curve_draw(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
		const PvRenderOption render_option,
		const PvElement *element)
{
	PvElement *simplify = _curve_simplify_new(
			element,
			render_option.render_context,
			PvSimplifyOption_All);
	pv_assert(simplify);

	_func_curve_draw_inline(cr, element_render_context, simplify);

	switch(element_render_context->element_group_kind){
		case PvElementGroupKind_Normal:
			{
				storke_and_fill_(cr, simplify);
				cairo_new_path(cr);
			}
			break;
		case PvElementGroupKind_MaskCurveSimple:
			{
				size_t num = pv_general_get_parray_num((void **)element->parent->childs);
				pv_assert(0 < num);
				if(element == element->parent->childs[(int)num - 1]){
					storke_and_fill_(cr, simplify);
					cairo_new_path(cr);
				}else{
					// NOP
				}
			}
			break;
		default:
			pv_abortf("%d", element_render_context->element_group_kind);
	}

	_curve_simplify_free(simplify);

	return PvElementDrawRecursive_End;
}

static PvElementDrawRecursive _func_curve_draw_focusing(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
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

	// ** stroke line
	_curve_command_path(cr, simplify);
	double LINE_WIDTH_FOCUS = 1.0;
	cairo_set_line_width(cr, LINE_WIDTH_FOCUS);
	pv_cairo_set_source_rgba_workingcolor(cr);
	cairo_stroke(cr);

	// ** anchor points
	int focus_index = pv_anchor_path_get_index_from_anchor_point(
			element->anchor_path, pv_focus_get_first_anchor_point(focus));
	size_t num = _func_curve_get_num_anchor_point(simplify);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
				element->anchor_path,
				i,
				PvAnchorPathIndexTurn_Disable);
		const PvAnchorPoint *ap_simplify = pv_anchor_path_get_anchor_point_from_index(
				simplify->anchor_path,
				i,
				PvAnchorPathIndexTurn_Disable);
		if(-1 != focus_index){
			int ofs_index = (i - focus_index);
			if(abs(ofs_index) <= 1){
				// * anchor handle. draw to focus and +-1
				_curve_draw_anchor_handle(cr, *ap_simplify, ofs_index);
			}
			if(0 == focus_index && i == ((int)num - 1) && pv_anchor_path_get_is_close(simplify->anchor_path)){
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

	// ** extent_view
	if(render_context.is_extent_view){
		_draw_extent_from_rect(cr, _func_curve_get_rect_by_anchor_points(simplify));
	}

	_curve_simplify_free(simplify);

	return PvElementDrawRecursive_End;
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

static bool _func_general_is_overlap_rect(
		bool *is_overlap,
		const PvElement *element,
		int offset,
		PvRect rect)
{
	*is_overlap = false;

	rect = pv_rect_abs_size(rect);

	PvRect rect_offseted = pv_rect_add_corners(rect, offset);

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assertf(info, "%d", element->kind);

	size_t num = info->func_get_num_anchor_point(element);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *anchor_point = info->func_get_anchor_point(element, i);
		pv_assertf(anchor_point, "%d", i);
		PvPoint a_point = pv_anchor_point_get_handle(anchor_point, PvAnchorPointIndex_Point);
		if(pv_rect_is_inside(rect_offseted, a_point)){
			*is_overlap = true;
			return true;
		}
	}

	return true;
}

static bool _func_curve_is_overlap_rect(
		bool *is_overlap,
		const PvElement *element,
		int offset,
		PvRect rect)
{
	return _func_general_is_overlap_rect(
			is_overlap,
			element,
			offset,
			rect
			);
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

	if(pv_anchor_path_get_is_close(element->anchor_path)){
		int index = pv_anchor_path_get_index_from_anchor_point(element->anchor_path, anchor_point);
		pv_assert(0 <= index);

		bool ret;
		ret = pv_anchor_path_change_head_index(element->anchor_path, index);
		pv_assert(ret);
		pv_anchor_path_set_is_close(element->anchor_path, false);
		pv_anchor_path_remove_delete_anchor_point(element->anchor_path, anchor_point);
		pv_assert(ret);
	}else{
		int index = pv_anchor_path_get_index_from_anchor_point(element->anchor_path, anchor_point);
		pv_assert(0 <= index);

		PvAnchorPath *anchor_path = pv_anchor_path_split_new_from_index_remove_delete(
				element->anchor_path,
				index);
		if(NULL != anchor_path){
			PvElement *element_2 = pv_element_curve_new_set_anchor_path(anchor_path);
			pv_assert(element_2);
			pv_element_copy_property(element_2, element);
			pv_element_append_child(element->parent, element, element_2);
			*p_foot_element = element_2;
		}
	}

	if(0 == pv_anchor_path_get_anchor_point_num(element->anchor_path)){
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

	*is_diff = false;
	return true;
}

static bool _func_curve_move_element(
		PvElement *element,
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

	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
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

	return pv_anchor_path_get_anchor_point_num(element->anchor_path);
}

static bool _func_curve_is_exist_anchor_point(
		const PvElement *element,
		const PvAnchorPoint *ap)
{
	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap_ = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		if(ap_ == ap){
			return true;
		}
	}

	return false;
}

static PvAnchorPoint *_func_curve_get_anchor_point(
		const PvElement *element,
		const int index)
{
	pv_assert(element);
	pv_assertf(PvElementKind_Curve == element->kind, "%d", element->kind);

	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, index, PvAnchorPathIndexTurn_Disable);
	return ap;
}

static bool _func_curve_set_anchor_point_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint point)
{
	pv_assert(element);
	pv_assertf(PvElementKind_Curve == element->kind, "%d", element->kind);

	PvElementCurveData *data = element->data;
	pv_assert(data);

	if(!pv_anchor_path_is_exist_anchor_point(element->anchor_path, ap)){
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
	pv_assert(element);
	pv_assertf(PvElementKind_Curve == element->kind, "%d", element->kind);

	PvElementCurveData *data = element->data;
	pv_assert(data);

	if(!pv_anchor_path_is_exist_anchor_point(element->anchor_path, ap)){
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
	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	pv_assert(0 < num);

	PvPoint min = (PvPoint){0, 0};
	PvPoint max = (PvPoint){0, 0};
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
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
	pv_assert(element);
	pv_assert(element->data);

	PvPoint point = PvPoint_Default;
	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
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
	pv_assert(element);
	pv_assert(element->data);


	PvPoint point_prev = _curve_get_point_by_anchor_points(element);
	PvPoint move = pv_point_sub(point, point_prev);

	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);

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
	if(rect_src.w < PV_DELTA_OF_RESIZE){
		pv_warning("");
		scale.x = 1;
	}
	if(rect_src.h < PV_DELTA_OF_RESIZE){
		pv_warning("");
		scale.y = 1;
	}


	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
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

	_curve_command_path(cr, element);
	storke_and_fill_(cr, element);

	PvRect rect = _get_rect_extent_from_cr(cr);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return rect;
}

static void _func_curve_apply_appearances(
		PvElement *element,
		PvAppearance **appearances)
{
	pv_assert(element);
	PvElementCurveData *data = element->data;
	pv_assert(data);

	size_t num = pv_general_get_parray_num((void **)appearances);
	for(int i = 0; i < (int)num; i++){
		switch(appearances[i]->kind){
			case PvAppearanceKind_None:
				// End of appearances
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
 * BasicShape
 **************** */

static const char *_func_basic_shape_get_kind_name(
		const PvElement *element)
{
	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assertf(info, "%d", element->kind);
	const PvElementBasicShapeData *data = element->data;
	pv_assertf(data, "%d", element->kind);
	const PvBasicShapeInfo *basic_shape_info = pv_basic_shape_info_get_from_kind(data->kind);
	pv_assertf(info, "%d %d", element->kind, data->kind);

	return basic_shape_info->func_get_kind_name(data);
}

static gpointer _func_basic_shape_new_data()
{
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)malloc(sizeof(PvElementBasicShapeData));
	pv_assert(element_data);

	element_data->kind = PvBasicShapeKind_Raster;

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(element_data->kind);
	pv_assertf(info, "%d", element_data->kind);

	element_data->data = info->func_new_data();
	pv_assert(element_data->data);

	element_data->basic_shape_appearances = pv_appearance_parray_new_from_num(Num_PvElementBasicShapeAppearance);
	pv_assert(element_data->basic_shape_appearances);
	element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Translate]->kind = PvAppearanceKind_Translate;
	element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Resize]->kind = PvAppearanceKind_Resize;
	element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Resize]->resize.resize = (PvPoint){0, 0};
	element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Rotate]->kind = PvAppearanceKind_Rotate;

	return (gpointer)element_data;
}

static bool _func_basic_shape_free_data(void *_data)
{
	pv_assert(_data);
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)_data;
	pv_assert(element_data);

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(element_data->kind);
	pv_assertf(info, "%d", element_data->kind);
	info->func_free_data(element_data->data);

	pv_appearance_parray_free(element_data->basic_shape_appearances);

	free(element_data);

	return true;
}

static gpointer _func_basic_shape_copy_new_data(void *_data)
{
	pv_assert(_data);
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)_data;
	pv_assert(element_data);

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(element_data->kind);
	pv_assertf(info, "%d", element_data->kind);

	PvElementBasicShapeData *new_element_data = (PvElementBasicShapeData *)malloc(sizeof(PvElementBasicShapeData));
	pv_assert(new_element_data);

	*new_element_data = *element_data;

	new_element_data->data = info->func_copy_new_data(element_data->data);
	pv_assert(new_element_data->data);

	if(NULL != element_data->basic_shape_appearances){
		new_element_data->basic_shape_appearances = pv_appearance_parray_copy_new(element_data->basic_shape_appearances);
		pv_assert(new_element_data->basic_shape_appearances);
	}

	return (gpointer)new_element_data;
}

static bool _func_basic_shape_write_svg(
		InfoTargetSvg *target,
		const PvElement *element,
		PvElementWriteSvgContext *element_write_svg_context,
		const ConfWriteSvg *conf)
{
	pv_assert(target);
	pv_assert(target->xml_parent_node);

	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)element->data;
	pv_assert(element_data);

	pv_assert(conf);

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(element_data->kind);
	pv_assertf(info, "%d", element_data->kind);

	xmlNodePtr node = NULL;
	if(!info->func_write_svg(&node, target, element, conf)){
		pv_error("%d", element_data->kind);
		return false;
	}

	node_add_color_props_(node, element);
	node_add_stroke_props_(node, element->stroke);

	return true;
}

static void _func_basic_shape_apply_appearances(
		PvElement *element,
		PvAppearance **appearances);

static PvElement *_basic_shape_simplify_new(const PvElement *element, PvRenderContext render_context)
{
	pv_assert(element);

	PvElement *simplify = pv_element_copy_recursive(element);
	pv_assert(simplify);
	PvElementBasicShapeData *simplify_data = simplify->data;
	pv_assert(simplify_data);

	// appearance
	_func_basic_shape_apply_appearances(simplify, element->etaion_work_appearances);
	simplify->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

	// scale
	simplify->stroke.width *= render_context.scale;
	PvPoint *move = &(simplify_data->basic_shape_appearances
			[PvElementBasicShapeAppearanceIndex_Translate]->translate.move);
	*move = pv_point_mul_value(*move, render_context.scale);
	PvPoint *resize = &(simplify_data->basic_shape_appearances
			[PvElementBasicShapeAppearanceIndex_Resize]->resize.resize);
	*resize = pv_point_mul_value(*resize, render_context.scale);

	return simplify;
}

void _basic_shape_simplify_free(PvElement *self)
{
	pv_element_remove_free_recursive(self);
}

typedef enum{
	BasicShapeDrawKind_Draw,
	BasicShapeDrawKind_DrawFocusing,
	BasicShapeDrawKind_IsTouch,
}BasicShapeDrawKind;

static void _basic_shape_draw_inline(
		cairo_t *cr,
		const PvRenderContext render_context,
		const PvElement *element,
		double line_width,
		BasicShapeDrawKind basic_shapeDrawKind
		)
{
	pv_assert(element);
	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	PvElement *simplify = _basic_shape_simplify_new(element, render_context);
	pv_assert(simplify);
	PvElementBasicShapeData *simplify_data = simplify->data;
	pv_assert(simplify_data);

	PvPoint position = simplify_data->basic_shape_appearances
		[PvElementBasicShapeAppearanceIndex_Translate]->translate.move;
	PvPoint resize = simplify_data->basic_shape_appearances
		[PvElementBasicShapeAppearanceIndex_Resize]->resize.resize;
	double degree = simplify_data->basic_shape_appearances
		[PvElementBasicShapeAppearanceIndex_Rotate]->rotate.degree;

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(simplify_data->kind);
	pv_assertf(info, "%d", simplify_data->kind);

	PvPoint size = info->func_get_size(simplify_data->data);
	size = pv_point_mul(size, resize);

	// effectively invisible to not draw
	const int PX_BASIC_SHAPE_MINIMUM = 0.00001;
	if(fabs(size.x)  < PX_BASIC_SHAPE_MINIMUM || fabs(size.y) < PX_BASIC_SHAPE_MINIMUM){
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
	switch(basic_shapeDrawKind){
		case BasicShapeDrawKind_Draw:
			{
				cairo_translate(cr, position.x, position.y);
				info->func_draw(cr, simplify, resize);
			}
			break;
		case BasicShapeDrawKind_DrawFocusing:
			{
				cairo_rectangle(cr, position.x, position.y, size.x, size.y);
				cairo_set_line_width(cr, line_width);
				pv_cairo_set_source_rgba_workingcolor(cr);
				cairo_stroke(cr);
			}
			break;
		case BasicShapeDrawKind_IsTouch:
			{
				cairo_rectangle(cr, position.x, position.y, size.x, size.y);
				cairo_set_line_width(cr, line_width);
				pv_cairo_set_source_rgba_workingcolor(cr);
				cairo_stroke_preserve(cr);
				cairo_fill_preserve(cr);
			}
			break;
		default:
			pv_abortf("%d", basic_shapeDrawKind);
			break;
	}
	cairo_restore(cr); // end rorate

finally:
	_basic_shape_simplify_free(simplify);

	return;
}

static PvElementDrawRecursive _func_basic_shape_draw(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
		const PvRenderOption render_option,
		const PvElement *element)
{
	_basic_shape_draw_inline(cr, render_option.render_context, element, 0, BasicShapeDrawKind_Draw);

	return PvElementDrawRecursive_End;
}

static PvElementDrawRecursive _func_basic_shape_draw_focusing(
		cairo_t *cr,
		PvElementRenderContext *element_render_context,
		const PvRenderOption render_option,
		const PvElement *element)
{
	_basic_shape_draw_inline(cr, render_option.render_context, element, 1.0, BasicShapeDrawKind_DrawFocusing);

	return PvElementDrawRecursive_End;
}

static bool _func_basic_shape_is_touch_element(
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
	_basic_shape_draw_inline(cr, render_context, element, c_width, BasicShapeDrawKind_IsTouch);

	//! @fixme bug fill area not detection.(down below side in fill.)
	*is_touch = cairo_in_stroke(cr, gx, gy) || cairo_in_fill(cr, gx, gy);

	cairo_surface_destroy (surface);

	return true;
}

static bool _func_basic_shape_is_overlap_rect(
		bool *is_overlap,
		const PvElement *element,
		int offset,
		PvRect rect)
{
	return _func_general_is_overlap_rect(
			is_overlap,
			element,
			offset,
			rect
			);
}

static void basic_shape_apply_anchor_path_(PvElement *element)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	PvPoint position = element_data->basic_shape_appearances
		[PvElementBasicShapeAppearanceIndex_Translate]->translate.move;
	PvPoint resize = element_data->basic_shape_appearances
		[PvElementBasicShapeAppearanceIndex_Resize]->resize.resize;
	double degree = element_data->basic_shape_appearances
		[PvElementBasicShapeAppearanceIndex_Rotate]->rotate.degree;

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(element_data->kind);
	pv_assertf(info, "%d", element_data->kind);

	PvPoint size = info->func_get_size(element_data->data);
	size = pv_point_mul(size, resize);

	PvRect rect = (PvRect){
		.x = position.x,
		.y = position.y,
		.w = size.x,
		.h = size.y,
	};

	pv_anchor_path_set_is_close(element->anchor_path, true);

	PvPoint center = pv_rect_get_center(rect);
	for(int i = 0; i < 4; i++){
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
				element->anchor_path, i, PvAnchorPathIndexTurn_Disable);
		pv_assertf(ap, "%d", i);

		PvPoint point = pv_rect_get_edge_point(rect, i);
		point = pv_rotate_point(point, degree, center);

		pv_anchor_point_set_handle_zero(ap, PvAnchorPointIndex_Point);
		pv_anchor_point_set_point(ap, point);
	}
}

static bool convert_basic_shape_to_curve_(PvElement *element)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	basic_shape_apply_anchor_path_(element);

	_func_basic_shape_free_data(element->data);
	element->data = _func_curve_new_data();
	pv_assert(element->data);

	element->kind = PvElementKind_Curve;

	return true;
}

bool _func_basic_shape_remove_delete_anchor_point(
		PvElement *element,
		const PvAnchorPoint *anchor_point,
		PvElement **p_foot_element,
		bool *is_deleted_element)
{
	*p_foot_element = NULL;
	*is_deleted_element = false;

	basic_shape_apply_anchor_path_(element);

	if(PvBasicShapeKind_Raster == pv_element_get_basic_shape_kind(element)){
		//! @todo
		return true;
	}

	if(!convert_basic_shape_to_curve_(element)){
		return false;
	}

	return _func_curve_remove_delete_anchor_point(
			element,
			anchor_point,
			p_foot_element,
			is_deleted_element);
}

static bool _func_basic_shape_is_diff_one(
		bool *is_diff,
		const PvElement *element0,
		const PvElement *element1)
{
	const PvElementBasicShapeData *element0_data = element0->data;
	const PvElementBasicShapeData *element1_data = element1->data;
	pv_assert(element0_data);
	pv_assert(element1_data);

	if(pv_appearance_parray_is_diff(element0_data->basic_shape_appearances, element1_data->basic_shape_appearances)){
		*is_diff = true;
		return true;
	}

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(element0_data->kind);
	pv_assertf(info, "%d", element0_data->kind);

	if(info->func_is_diff_one(element0_data->data, element1_data->data)){
		*is_diff = true;
		return true;
	}

	*is_diff = false;
	return true;
}

static bool _func_basic_shape_move_element(
		PvElement *element,
		double gx,
		double gy)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	PvPoint position = element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Translate]->translate.move;
	position.x += gx;
	position.y += gy;
	element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Translate]->translate.move = position;

	basic_shape_apply_anchor_path_(element);

	return true;
}

int _func_basic_shape_get_num_anchor_point(
		const PvElement *element)
{
	if(NULL == element || PvElementKind_BasicShape != element->kind){
		pv_error("%p", element);
		return -1;
	}

	return pv_anchor_path_get_anchor_point_num(element->anchor_path);
}

static bool _func_basic_shape_is_exist_anchor_point(
		const PvElement *element,
		const PvAnchorPoint *ap)
{
	size_t num = pv_anchor_path_get_anchor_point_num(element->anchor_path);
	for(int i = 0; i < (int)num; i++){
		PvAnchorPoint *ap_ = pv_anchor_path_get_anchor_point_from_index(
				element->anchor_path,
				i,
				PvAnchorPathIndexTurn_Disable);
		if(ap_ == ap){
			return true;
		}
	}

	return false;
}

static PvAnchorPoint *_func_basic_shape_get_anchor_point(
		const PvElement *element,
		const int index)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	PvPoint position = element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Translate]->translate.move;

	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			element->anchor_path,
			index,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);

	PvPoint point = {
		.x = position.x,
		.y = position.y,
	};
	pv_anchor_point_set_handle(ap, PvAnchorPointIndex_Point, point);

	return ap;
}

static bool _func_basic_shape_set_anchor_point_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint point)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	basic_shape_apply_anchor_path_(element);

	PvPoint prev = pv_anchor_point_get_point(ap);
	if(PV_DELTA > pv_point_distance(prev, point)){
		pv_debug("not change.");
		return true;
	}

	if(PvBasicShapeKind_Raster == pv_element_get_basic_shape_kind(element)){
		//! @todo
		return true;
	}

	if(!convert_basic_shape_to_curve_(element)){
		return false;
	}

	return _func_curve_set_anchor_point_point(
			element,
			ap,
			point);
}

static bool _func_basic_shape_move_anchor_point(
		PvElement *element,
		PvAnchorPoint *ap,
		const PvPoint move)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	basic_shape_apply_anchor_path_(element);

	if(PvBasicShapeKind_Raster == pv_element_get_basic_shape_kind(element)){
		//! @todo
		return true;
	}

	if(!convert_basic_shape_to_curve_(element)){
		return false;
	}

	return _func_curve_move_anchor_point(
			element,
			ap,
			move);
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

static PvRect _func_basic_shape_get_rect_by_anchor_points(
		const PvElement *element)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	PvPoint position = element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Translate]->translate.move;
	PvPoint resize = element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Resize]->resize.resize;

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(element_data->kind);
	pv_assertf(info, "%d", element_data->kind);
	PvPoint size = info->func_get_size(element_data->data);

	PvRect rect = PvRect_Default;
	rect.x = position.x;
	rect.y = position.y;
	rect.w = size.x * resize.x;
	rect.h = size.y * resize.y;

	PvPoint center = pv_rect_get_center(rect);
	PvRect expand = PvRect_Default;
	for(int i = 0; i < 4; i++){
		PvPoint p = pv_rect_get_edge_point(rect, i);
		p = pv_rotate_point(
				p,
				element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Rotate]->rotate.degree,
				center);
		expand = pv_rect_expand_from_point_(expand, p, i == 0);
	}

	return expand;
}

/*
static PvRect _func_basic_shape_get_rect_by_anchor_points(
		const PvElement *element)
{
	return _func_curve_get_rect_by_anchor_points(element);
}
*/

static bool _func_basic_shape_set_rect_by_anchor_points(
		PvElement *element,
		PvRect rect)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	PvPoint *move = &(element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Translate]->translate.move);
	*move = (PvPoint){.x = rect.x, .y = rect.y};

	{
		PvRect src_rect = _func_basic_shape_get_rect_by_anchor_points(element);

		PvPoint *resize = &(element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Resize]->resize.resize);
		if(PV_DELTA < src_rect.w){
			double scale = rect.w / src_rect.w;
			(*resize).x *= scale;
		}else{
			(*resize).x = rect.w;
		}
		if(PV_DELTA < src_rect.h){
			double scale = rect.h / src_rect.h;
			(*resize).y *= scale;
		}else{
			(*resize).y = rect.h;
		}
	}

	basic_shape_apply_anchor_path_(element);

	return true;
}

static PvRect _func_basic_shape_get_rect_by_draw(
		const PvElement *element)
{
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1,1);
	pv_assert(surface);
	cairo_t *cr = cairo_create (surface);
	pv_assert(cr);

	_curve_command_path(cr, element);
	storke_and_fill_(cr, element);

	PvElementRenderContext erc = PvElementRenderContext_Default;
	_func_basic_shape_draw(
			cr,
			&erc,
			PvRenderOption_Default(),
			element
			);

	PvRect rect = _get_rect_extent_from_cr(cr);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return rect;
}

static void _func_basic_shape_apply_appearances(
		PvElement *element,
		PvAppearance **appearances)
{
	pv_assert(element);
	pv_assertf(PvElementKind_BasicShape == element->kind, "%d", element->kind);

	PvElementBasicShapeData *element_data = element->data;
	pv_assert(element_data);

	const PvBasicShapeInfo *info = pv_basic_shape_info_get_from_kind(element_data->kind);
	pv_assertf(info, "%d", element_data->kind);

	size_t num = pv_general_get_parray_num((void **)appearances);
	for(int i = 0; i < (int)num; i++){
		switch(appearances[i]->kind){
			case PvAppearanceKind_None:
				// End of appearances
				return;
				break;
			case PvAppearanceKind_Translate:
				{
					PvPoint *move = &(element_data->basic_shape_appearances
							[PvElementBasicShapeAppearanceIndex_Translate]->translate.move);
					*move = pv_point_add(*move, appearances[i]->translate.move);
				}
				break;
			case PvAppearanceKind_Resize:
				{
					PvPoint *resize = &(element_data->basic_shape_appearances
							[PvElementBasicShapeAppearanceIndex_Resize]->resize.resize);
					PvPoint img_size = info->func_get_size(element_data->data);
					PvPoint src_size = pv_point_mul(img_size, *resize);

					*resize = pv_point_mul(*resize, appearances[i]->resize.resize);

					PvPoint dst_size = pv_point_mul(img_size, *resize);

					PvPoint *move = &(element_data->basic_shape_appearances
							[PvElementBasicShapeAppearanceIndex_Translate]->translate.move);
					PvPoint diff = pv_point_div_value(pv_point_sub(src_size, dst_size), 2);
					*move = pv_point_add(*move, diff);
				}
				break;
			case PvAppearanceKind_Rotate:
				{
					element_data->basic_shape_appearances[PvElementBasicShapeAppearanceIndex_Rotate]
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
		.func_get_kind_name			= _func_default_get_kind_name,
		.func_new_data				= _func_group_new_data,
		.func_free_data				= _func_group_free_data,
		.func_copy_new_data			= _func_group_copy_new_data,
		.func_write_svg				= _func_group_write_svg,
		.func_write_svg_after			= _func_group_write_svg_after,
		.func_draw				= _func_group_draw,
		.func_draw_focusing			= _func_group_draw,
		.func_draw_after			= _func_nop_draw_after,
		.func_is_touch_element			= _func_group_is_touch_element,
		.func_is_overlap_rect			= _func_nop_is_overlap_rect,
		.func_is_diff_one			= _func_group_is_diff_one,
		.func_move_element			= _func_group_move_element,
		.func_get_num_anchor_point		= _func_zero_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_nop_is_exist_anchor_point,
		.func_get_anchor_point			= _func_notimpl_get_anchor_point,
		.func_set_anchor_point_point		= _func_notimpl_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_notimpl_move_anchor_point,
		.func_remove_delete_anchor_point	= _func_nop_remove_delete_anchor_point,
		.func_get_rect_by_anchor_points		= _func_notimpl_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_notimpl_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_notimpl_get_rect_by_draw,
		.func_apply_appearances			= _func_nop_apply_appearances,
	},
	{PvElementKind_Layer, "Layer",
		.func_get_kind_name			= _func_default_get_kind_name,
		.func_new_data				= _func_group_new_data,
		.func_free_data				= _func_group_free_data,
		.func_copy_new_data			= _func_group_copy_new_data,
		.func_write_svg				= _func_group_write_svg,
		.func_write_svg_after			= _func_group_write_svg_after,
		.func_draw				= _func_group_draw,
		.func_draw_focusing			= _func_group_draw,
		.func_draw_after			= _func_nop_draw_after,
		.func_is_touch_element			= _func_group_is_touch_element,
		.func_is_overlap_rect			= _func_nop_is_overlap_rect,
		.func_is_diff_one			= _func_group_is_diff_one,
		.func_move_element			= _func_group_move_element,
		.func_get_num_anchor_point		= _func_zero_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_nop_is_exist_anchor_point,
		.func_get_anchor_point			= _func_notimpl_get_anchor_point,
		.func_set_anchor_point_point		= _func_notimpl_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_notimpl_move_anchor_point,
		.func_remove_delete_anchor_point	= _func_nop_remove_delete_anchor_point,
		.func_get_rect_by_anchor_points		= _func_notimpl_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_notimpl_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_notimpl_get_rect_by_draw,
		.func_apply_appearances			= _func_nop_apply_appearances,
	},
	{PvElementKind_Group, "Group",
		.func_get_kind_name			= _func_default_get_kind_name,
		.func_new_data				= _func_group_new_data,
		.func_free_data				= _func_group_free_data,
		.func_copy_new_data			= _func_group_copy_new_data,
		.func_write_svg				= _func_group_write_svg,
		.func_write_svg_after			= _func_group_write_svg_after,
		.func_draw				= _func_group_draw,
		.func_draw_focusing			= _func_group_draw,
		.func_draw_after			= _func_group_draw_after,
		.func_is_touch_element			= _func_group_is_touch_element,
		.func_is_overlap_rect			= _func_nop_is_overlap_rect,
		.func_is_diff_one			= _func_group_is_diff_one,
		.func_move_element			= _func_group_move_element,
		.func_get_num_anchor_point		= _func_zero_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_nop_is_exist_anchor_point,
		.func_get_anchor_point			= _func_notimpl_get_anchor_point,
		.func_set_anchor_point_point		= _func_notimpl_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_notimpl_move_anchor_point,
		.func_remove_delete_anchor_point	= _func_nop_remove_delete_anchor_point,
		.func_get_rect_by_anchor_points		= _func_notimpl_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_notimpl_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_notimpl_get_rect_by_draw,
		.func_apply_appearances			= _func_nop_apply_appearances,
	},
	{PvElementKind_Curve, "Curve",
		.func_get_kind_name			= _func_default_get_kind_name,
		.func_new_data				= _func_curve_new_data,
		.func_free_data				= _func_curve_free_data,
		.func_copy_new_data			= _func_curve_copy_new_data,
		.func_write_svg				= _func_curve_write_svg,
		.func_write_svg_after			= _func_nop_write_svg_after,
		.func_draw				= _func_curve_draw,
		.func_draw_focusing			= _func_curve_draw_focusing,
		.func_draw_after			= _func_nop_draw_after,
		.func_is_touch_element			= _func_curve_is_touch_element,
		.func_is_overlap_rect			= _func_curve_is_overlap_rect,
		.func_is_diff_one			= _func_curve_is_diff_one,
		.func_move_element			= _func_curve_move_element,
		.func_get_num_anchor_point		= _func_curve_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_curve_is_exist_anchor_point,
		.func_get_anchor_point			= _func_curve_get_anchor_point,
		.func_set_anchor_point_point		= _func_curve_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_curve_move_anchor_point,
		.func_remove_delete_anchor_point	= _func_curve_remove_delete_anchor_point,
		.func_get_rect_by_anchor_points		= _func_curve_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_curve_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_curve_get_rect_by_draw,
		.func_apply_appearances			= _func_curve_apply_appearances,
	},
	{PvElementKind_BasicShape, "BasicShape",
		.func_get_kind_name			= _func_basic_shape_get_kind_name,
		.func_new_data				= _func_basic_shape_new_data,
		.func_free_data				= _func_basic_shape_free_data,
		.func_copy_new_data			= _func_basic_shape_copy_new_data,
		.func_write_svg				= _func_basic_shape_write_svg,
		.func_write_svg_after			= _func_nop_write_svg_after,
		.func_draw				= _func_basic_shape_draw,
		.func_draw_focusing			= _func_basic_shape_draw_focusing,
		.func_draw_after			= _func_nop_draw_after,
		.func_is_touch_element			= _func_basic_shape_is_touch_element,
		.func_is_overlap_rect			= _func_basic_shape_is_overlap_rect,
		.func_is_diff_one			= _func_basic_shape_is_diff_one,
		.func_move_element			= _func_basic_shape_move_element,
		.func_get_num_anchor_point		= _func_basic_shape_get_num_anchor_point,
		.func_is_exist_anchor_point		= _func_basic_shape_is_exist_anchor_point,
		.func_get_anchor_point			= _func_basic_shape_get_anchor_point,
		.func_set_anchor_point_point		= _func_basic_shape_set_anchor_point_point,
		.func_move_anchor_point_point		= _func_basic_shape_move_anchor_point,
		.func_remove_delete_anchor_point	= _func_basic_shape_remove_delete_anchor_point,
		.func_get_rect_by_anchor_points		= _func_basic_shape_get_rect_by_anchor_points,
		.func_set_rect_by_anchor_points		= _func_basic_shape_set_rect_by_anchor_points,
		.func_get_rect_by_draw			= _func_basic_shape_get_rect_by_draw,
		.func_apply_appearances			= _func_basic_shape_apply_appearances,
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

