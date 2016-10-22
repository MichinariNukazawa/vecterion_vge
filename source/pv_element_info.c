#include "pv_element_info.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"
#include "pv_element_info_cairo.h"
#include "pv_render_context.h"
#include "pv_color.h"


/* ****************
 * null functions. (ex. use method not implement)
 **************** */

/** @brief 無効なindexを引いた際に埋め込まれているダミー関数 */
static gpointer _pv_element_error_return_null_new()
{
	pv_error("");
	return NULL;
}

static bool _pv_element_error_return_null_delete(void *_data)
{
	pv_error("");
	return false;
}

static gpointer _pv_element_error_return_null_copy_new(void *_data)
{
	pv_error("");
	return NULL;
}

/** @brief write_svg未実装箇所に挿入する */
static int _pv_element_notimplement_svg_write(
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

static bool _pv_element_notimplement_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	pv_error("");
	return true;
}

static bool _pv_element_notimplement_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		int offset,
		double gx,
		double gy)
{
	pv_error("");
	*is_touch = false;
	return true;
}

static bool _pv_element_notimplement_is_diff_one(
		bool *is_diff,
		const PvElement *element0,
		const PvElement *element1)
{
	pv_error("");
	*is_diff = true;
	return true;
}

static bool _pv_element_notimplement_move_element(
		const PvElement *element,
		double gx,
		double gy)
{
	pv_error("");
	return false;
}

static PvPoint _pv_element_notimplement_get_point_by_anchor_points(
		const PvElement *element)
{
	return PvPoint_Default;
}

static void _pv_element_notimplement_set_point_by_anchor_points(
		PvElement *element,
		const PvPoint point)
{
	return;
}

static PvAnchorPoint _pv_element_notimplement_get_anchor_point(
		const PvElement *element,
		const int index)
{
	PvAnchorPoint ap = PvAnchorPoint_Default;

	return ap;
}

static bool _pv_element_notimplement_set_anchor_point_point(
		const PvElement *element,
		const int index,
		const PvPoint point)
{
	return true;
}

static bool _pv_element_notimplement_move_anchor_point(
		const PvElement *element,
		const int index,
		const PvPoint move)
{
	return true;
}

static PvRect _pv_element_notimplement_get_rect_anchor_points(
		const PvElement *element)
{
	return PvRect_Default;
}



/* ****************
 * General
 **************** */

/** @brief 
 * arg1 NULL -> return NULL and not error(is_error == false)
 */
char *pv_general_new_str(const char *src, bool *is_error)
{
	*is_error = true;

	if(NULL == src){
		*is_error = false;
		return NULL;
	}

	if(20 < strlen(src)){
		pv_debug("len:%lu '%s'", strlen(src), src);
	}

	char *dst = (char *)malloc(sizeof(char) * (strlen(src) + 1));
	if(NULL == dst){
		pv_critical("");
		exit(-1);
	}

	strcpy(dst, src);

	*is_error = false;
	return dst;
}

// ** PvElementKindごとのdataのnew関数群

/* ****************
 * Group(Root,Layer,Group)
 **************** */

static gpointer _pv_element_group_data_new()
{
	PvElementGroupData *data = (PvElementGroupData *)malloc(sizeof(PvElementGroupData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->name = NULL;

	return (gpointer)data;
}

static bool _pv_element_group_data_delete(void *_data)
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

static gpointer _pv_element_group_data_copy_new(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementGroupData *data = (PvElementGroupData *)_data;

	PvElementGroupData *new_data = (PvElementGroupData *)malloc(sizeof(PvElementGroupData));
	if(NULL == new_data){
		pv_critical("");
		exit(-1);
	}

	bool is_error = true;
	new_data->name = pv_general_new_str(data->name, &is_error);
	if(is_error){
		pv_critical("");
		exit(-1);
	}

	return (gpointer)new_data;
}

static int _pv_element_group_write_svg(
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

static bool _pv_element_group_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	return true;
}

static bool _pv_element_group_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		int offset,
		double gx,
		double gy)
{
	*is_touch = false;
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

static bool _pv_element_group_is_diff_one(
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

static bool _pv_element_group_move_element(
		const PvElement *element,
		double gx,
		double gy)
{
	return true;
}

/* ****************
 * Bezier
 **************** */

int _pv_element_bezier_get_num_anchor_point(const PvElement *element);

static gpointer _pv_element_bezier_data_new()
{
	PvElementBezierData *data = (PvElementBezierData *)malloc(sizeof(PvElementBezierData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->is_close = false;
	data->anchor_points_num = 0;
	data->anchor_points = NULL;

	return (gpointer)data;
}

static bool _pv_element_bezier_data_delete(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementBezierData *data = (PvElementBezierData *)_data;

	if(NULL != data->anchor_points){
		free(data->anchor_points);
	}

	free(data);

	return true;
}

static gpointer _pv_element_bezier_data_copy_new(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementBezierData *data = (PvElementBezierData *)_data;

	PvElementBezierData *new_data = (PvElementBezierData *)malloc(sizeof(PvElementBezierData));
	if(NULL == new_data){
		pv_critical("");
		exit(-1);
	}

	*new_data = *data;
	new_data->anchor_points_num = 0;
	new_data->anchor_points = NULL;
	if(0 < data->anchor_points_num && NULL != data->anchor_points){
		size_t size = data->anchor_points_num * sizeof(PvAnchorPoint);
		new_data->anchor_points = malloc(size);
		if(NULL == new_data->anchor_points){
			pv_critical("");
			exit(-1);
		}
		memcpy(new_data->anchor_points, data->anchor_points, size);
		new_data->anchor_points_num = data->anchor_points_num;
	}

	return (gpointer)new_data;
}

static char *_pv_element_bezier_new_str_from_anchor(const PvAnchorPoint ap_current, const PvAnchorPoint ap_prev)
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

static int _pv_element_bezier_write_svg(
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

	PvElementBezierData *data = (PvElementBezierData *)element->data;

	char *str_current = NULL;
	for(int i = 0; i < data->anchor_points_num; i++){
		const PvAnchorPoint ap = data->anchor_points[i];

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
			const PvAnchorPoint ap_prev = data->anchor_points[i - 1];
			str_point = _pv_element_bezier_new_str_from_anchor(ap, ap_prev);
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

	if(data->is_close && 0 < data->anchor_points_num){
		const PvAnchorPoint ap_first = data->anchor_points[0];
		const PvAnchorPoint ap_last = data->anchor_points[data->anchor_points_num - 1];
		char *str_end = _pv_element_bezier_new_str_from_anchor(ap_first, ap_last);
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

static bool _pv_element_bezier_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	const PvRenderContext render_context = render_option.render_context;

	if(!_pv_element_bezier_command_path(
				cr,
				render_context,
				element))
	{
		pv_error("");
		return false;
	}

	double c_width = element->stroke.width * render_context.scale;
	cairo_set_line_width(cr, c_width);

	//! fill
	PvCairoRgbaColor cc_f = pv_color_get_cairo_rgba(
			element->color_pair.colors[PvColorPairGround_BackGround]);
	cairo_set_source_rgba (cr, cc_f.r, cc_f.g, cc_f.b, cc_f.a);
	cairo_fill_preserve(cr);
	//! stroke
	const PvStrokeLinecapInfo *linecap_info = get_stroke_linecap_info_from_id(element->stroke.linecap);
	cairo_set_line_cap (cr, linecap_info->cairo_value);
	const PvStrokeLinejoinInfo *linejoin_info = get_stroke_linejoin_info_from_id(element->stroke.linejoin);
	cairo_set_line_join (cr, linejoin_info->cairo_value);
	PvCairoRgbaColor cc_s = pv_color_get_cairo_rgba(
			element->color_pair.colors[PvColorPairGround_ForGround]);
	cairo_set_source_rgba (cr, cc_s.r, cc_s.g, cc_s.b, cc_s.a);
	cairo_stroke_preserve(cr);

	//! get extent area
	if(render_context.is_extent_view && !render_context.is_focus){
		PvRect crect_extent = _pv_renderer_get_rect_extent_from_cr(cr);
		cairo_new_path(cr);
		_pv_renderer_draw_extent_from_crect(cr, crect_extent);
	}

	cairo_new_path(cr);

	return true;
}

typedef enum{
	PvElementPointKind_Normal,
	PvElementPointKind_Selected,
	PvElementPointKind_AnchorHandle,
}PvElementPointKind;

static void _pv_element_bezier_draw_point(cairo_t *cr, PvPoint gp, PvElementPointKind kind)
{
	cairo_arc (cr, gp.x, gp.y, 2.0, 0., 2 * M_PI);

	cairo_set_source_rgba (cr, 1, 1, 1, 1.0); // white
	if(PvElementPointKind_Selected == kind){
		_pv_render_workingcolor_cairo_set_source_rgba(cr);
	}
	cairo_fill_preserve (cr);

	_pv_render_workingcolor_cairo_set_source_rgba(cr);
	cairo_set_line_width(cr, 0.75);
	cairo_stroke (cr);
}

static void _pv_element_bezier_draw_anchor_handle(
		cairo_t *cr, PvAnchorPoint ap,
		int ofs_index, PvRenderContext render_context)
{
	PvPoint gp_current = ap.points[PvAnchorPointIndex_Point];
	PvPoint gp_prev = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_HandlePrev);
	PvPoint gp_next = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_HandleNext);
	gp_current.x *= render_context.scale;
	gp_current.y *= render_context.scale;
	gp_prev.x *= render_context.scale;
	gp_prev.y *= render_context.scale;
	gp_next.x *= render_context.scale;
	gp_next.y *= render_context.scale;

	if(0 == ofs_index){
		// ** current anchor_point
		cairo_set_line_width(cr, 1.0);
		_pv_render_workingcolor_cairo_set_source_rgba(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_prev.x, gp_prev.y);
		cairo_stroke(cr);

		cairo_set_line_width(cr, 1.0);
		_pv_render_workingcolor_cairo_set_source_rgba(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_next.x, gp_next.y);
		cairo_stroke(cr);

		_pv_element_bezier_draw_point(cr, gp_prev, PvElementPointKind_AnchorHandle);
		_pv_element_bezier_draw_point(cr, gp_next, PvElementPointKind_AnchorHandle);
	} else if(-1 == ofs_index) {
		// ** prev anchor_point
		cairo_set_line_width(cr, 1.0);
		_pv_render_workingcolor_cairo_set_source_rgba(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_next.x, gp_next.y);
		cairo_stroke(cr);

		_pv_element_bezier_draw_point(cr, gp_next, PvElementPointKind_AnchorHandle);
	}else if(1 == ofs_index){
		// ** next anchor_point
		cairo_set_line_width(cr, 1.0);
		_pv_render_workingcolor_cairo_set_source_rgba(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_prev.x, gp_prev.y);
		cairo_stroke(cr);

		_pv_element_bezier_draw_point(cr, gp_prev, PvElementPointKind_AnchorHandle);
	}else{
		pv_bug("%d", ofs_index);
	}
}

static bool _pv_element_bezier_draw_focusing(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	const PvRenderContext render_context = render_option.render_context;
	const PvFocus *focus = render_option.focus;

	const PvElementBezierData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}

	// ** stroke line
	if(!_pv_element_bezier_command_path(
				cr,
				render_context,
				element))
	{
		pv_error("");
		return false;
	}
	double LINE_WIDTH_FOCUS = 1.0;
	cairo_set_line_width(cr, LINE_WIDTH_FOCUS);
	_pv_render_workingcolor_cairo_set_source_rgba(cr);
	cairo_stroke(cr);

	// ** anchor points
	int num = _pv_element_bezier_get_num_anchor_point(element);
	for(int i = 0; i < num; i++){
		int ofs_index = (i - focus->index);
		if(-1 != focus->index){
			if(abs(ofs_index) <= 1){
				// * anchor handle. draw to focus and +-1
				_pv_element_bezier_draw_anchor_handle(
						cr, data->anchor_points[i],
						ofs_index, render_context);
			}
			// *anchor handle. to last AnchorPoint
			if(0 == focus->index && i == (num - 1) && data->is_close){
				_pv_element_bezier_draw_anchor_handle(
						cr, data->anchor_points[i],
						-1, render_context);
			}
		}
		PvPoint gp = data->anchor_points[i].points[PvAnchorPointIndex_Point];
		gp.x *= render_context.scale;
		gp.y *= render_context.scale;
		PvElementPointKind kind = ((i == focus->index)?
				PvElementPointKind_Selected : PvElementPointKind_Normal);
		_pv_element_bezier_draw_point(cr, gp, kind);
	}

	return true;
}

static bool _pv_element_bezier_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		int offset,
		double gx,
		double gy)
{
	*is_touch = false;

	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1,1);
	cairo_t *cr = cairo_create (surface);
	if(NULL == cr){
		pv_error("");
		return false;
	}

	PvRenderContext render_context = PvRenderContext_Default;
	if(!_pv_element_bezier_command_path(
				cr,
				render_context,
				element))
	{
		pv_error("");
		return false;
	}

	double c_width = (element->stroke.width * render_context.scale) + offset;
	cairo_set_line_width(cr, c_width);

	// PvRect crect_extent = _pv_renderer_get_rect_extent_from_cr(cr);
	//! @fixme bug fill area not detection.(down below side in fill.)
	*is_touch = cairo_in_stroke(cr, gx, gy) || cairo_in_fill(cr, gx, gy);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return true;
}

static bool _pv_element_bezier_is_diff_one(
		bool *is_diff,
		const PvElement *element0,
		const PvElement *element1)
{
	const PvElementBezierData *data0 = element0->data;
	const PvElementBezierData *data1 = element1->data;
	if(NULL == data0 || NULL == data1){
		pv_bug("%p,%p", data0, data1);
		*is_diff = true;
		return false;
	}

	if(data0->is_close != data1->is_close){
		*is_diff = true;
		return true;
	}

	if(data0->anchor_points_num != data1->anchor_points_num){
		*is_diff = true;
		return true;
	}

	for(int i = 0; i < data0->anchor_points_num; i++){
		const PvAnchorPoint *ap0 = &(data0->anchor_points[i]);
		const PvAnchorPoint *ap1 = &(data1->anchor_points[i]);
		if(!(true
					&& ap0->points[0].x == ap1->points[0].x
					&& ap0->points[0].y == ap1->points[0].y
					&& ap0->points[1].x == ap1->points[1].x
					&& ap0->points[1].y == ap1->points[1].y
					&& ap0->points[2].x == ap1->points[2].x
					&& ap0->points[2].y == ap1->points[2].y))
		{
			*is_diff = true;
			return true;
		}
	}

	*is_diff = false;
	return true;
}

/*! @fixme get point left head. */
static PvPoint _pv_element_bezier_get_point_by_anchor_points(
		const PvElement *element)
{
	assert(element);
	assert(element->data);
	const PvElementBezierData *data = element->data;

	PvPoint point = PvPoint_Default;
	int num = data->anchor_points_num;
	for(int i = 0; i < num; i++){
		PvPoint a_point = pv_anchor_point_get_handle(data->anchor_points[i], PvAnchorPointIndex_Point);
		if(0 == i){
			point = a_point;
		}else{
			point.x = (point.x < a_point.x)? point.x : a_point.x;
			point.y = (point.y < a_point.y)? point.y : a_point.y;
		}
	}

	return point;
}

static void _pv_element_bezier_set_point_by_anchor_points(
		PvElement *element,
		const PvPoint point)
{
	assert(element);
	assert(element->data);
	PvElementBezierData *data = element->data;


	PvPoint point_prev = _pv_element_bezier_get_point_by_anchor_points(element);
	PvPoint move = pv_point_sub(point, point_prev);

	int num = data->anchor_points_num;
	for(int i = 0; i < num; i++){
		pv_anchor_point_move_point(&(data->anchor_points[i]), move);
	}

	return;
}

static bool _pv_element_bezier_move_element(
		const PvElement *element,
		double gx,
		double gy)
{
	if(NULL == element || PvElementKind_Bezier != element->kind){
		pv_error("%p", element);
		return false;
	}
	PvElementBezierData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}

	for(int i = 0; i < data->anchor_points_num; i++){
		data->anchor_points[i].points[PvAnchorPointIndex_Point].x += gx;
		data->anchor_points[i].points[PvAnchorPointIndex_Point].y += gy;
	}

	return true;
}

int _pv_element_zero_get_num_anchor_point(
		const PvElement *element)
{
	return 0;
}

int _pv_element_bezier_get_num_anchor_point(
		const PvElement *element)
{
	if(NULL == element || PvElementKind_Bezier != element->kind){
		pv_error("%p", element);
		return false;
	}
	PvElementBezierData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}

	return data->anchor_points_num;
}

PvAnchorPoint *_pv_element_null_new_anchor_points(
		const PvElement *element)
{
	return NULL;
}

PvAnchorPoint *_pv_element_bezier_new_anchor_points(
		const PvElement *element)
{
	assert(element);
	assert(PvElementKind_Bezier == element->kind);

	PvElementBezierData *data = element->data;
	assert(data);
	assert(data->anchor_points);

	PvAnchorPoint *new_anchor_points = NULL;
	if(0 < data->anchor_points_num && NULL != data->anchor_points){
		size_t size = data->anchor_points_num * sizeof(PvAnchorPoint);
		new_anchor_points = malloc(size);
		memcpy(new_anchor_points, data->anchor_points, size);
	}

	return new_anchor_points;
}

static PvAnchorPoint _pv_element_bezier_get_anchor_point(
		const PvElement *element,
		const int index)
{
	assert(element);
	assert(PvElementKind_Bezier == element->kind);

	PvElementBezierData *data = element->data;
	assert(data);
	assert(data->anchor_points);
	assert(index < data->anchor_points_num);

	return data->anchor_points[index];
}

static bool _pv_element_bezier_set_anchor_point_point(
		const PvElement *element,
		const int index,
		const PvPoint point)
{
	assert(element);
	assert(PvElementKind_Bezier == element->kind);

	PvElementBezierData *data = element->data;
	assert(data);
	assert(data->anchor_points);
	assert(index < data->anchor_points_num);

	pv_anchor_point_set_point(
			&(data->anchor_points[index]),
			point);

	return true;
}

static bool _pv_element_bezier_move_anchor_point(
		const PvElement *element,
		const int index,
		const PvPoint move)
{
	assert(element);
	assert(PvElementKind_Bezier == element->kind);

	PvElementBezierData *data = element->data;
	assert(data);

	int num = _pv_element_bezier_get_num_anchor_point(element);
	if(num <= index){
		pv_bug("%d %d", index, num);
		return false;
	}

	data->anchor_points[index].points[PvAnchorPointIndex_Point].x += move.x;
	data->anchor_points[index].points[PvAnchorPointIndex_Point].y += move.y;

	return true;
}

static PvRect _pv_element_bezier_get_rect_anchor_points(
		const PvElement *element)
{
	const PvElementBezierData *data = (PvElementBezierData *)element->data;

	PvPoint min = (PvPoint){0, 0};
	PvPoint max = (PvPoint){0, 0};
	for(int i = 0; i < data->anchor_points_num; i++){
		PvPoint p = data->anchor_points[i].points[PvAnchorPointIndex_Point];
		if(0 == i){
			min.x = p.x;
			min.y = p.y;
			max.x = p.x;
			max.y = p.y;
		}else{
			min.x = (min.x < p.x)? min.x : p.x;
			min.y = (min.y < p.y)? min.y : p.y;
			max.x = (max.x > p.x)? max.x : p.x;
			max.y = (max.y > p.y)? max.y : p.y;
		}
	}

	PvRect rect = (PvRect){min.x, min.y, (max.x - min.x), (max.y - min.y)};

	return rect;
}



/* ****************
 * Raster
 **************** */

static gpointer _pv_element_raster_data_new()
{
	PvElementRasterData *data = (PvElementRasterData *)malloc(sizeof(PvElementRasterData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->matrix = PvMatrix_Default;
	data->path = NULL;
	data->pixbuf = NULL;

	return (gpointer)data;
}

static bool _pv_element_raster_data_delete(void *_data)
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

	free(data);

	return true;
}

static gpointer _pv_element_raster_data_copy_new(void *_data)
{
	if(NULL == _data){
		pv_error("");
		return false;
	}

	PvElementRasterData *data = (PvElementRasterData *)_data;

	PvElementRasterData *new_data 
		= (PvElementRasterData *)malloc(sizeof(PvElementRasterData));
	if(NULL == new_data){
		pv_critical("");
		exit(-1);
	}
	new_data = data;

	bool is_error = true;
	new_data->path = pv_general_new_str(data->path, &is_error);
	if(is_error){
		pv_critical("");
		exit(-1);
	}

	g_object_ref(G_OBJECT(data->pixbuf));
	new_data->pixbuf = data->pixbuf;

	return (gpointer)new_data;
}

static bool _pv_element_raster_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	const PvRenderContext render_context = render_option.render_context;

	PvElementRasterData *data = element->data;
	GdkPixbuf *pixbuf = data->pixbuf;
	double w = gdk_pixbuf_get_width(pixbuf);
	double h = gdk_pixbuf_get_height(pixbuf);
	w *= render_context.scale;
	h *= render_context.scale;
	GdkPixbuf *pb = gdk_pixbuf_scale_simple(
			pixbuf,
			(int)w, (int)h,
			GDK_INTERP_HYPER);
	if(NULL == pb){
		pv_error("");
		return false;
	}else{
		double x = data->matrix.x;
		double y = data->matrix.y;
		x *= render_context.scale;
		y *= render_context.scale;
		gdk_cairo_set_source_pixbuf (cr, pb, x, y);
		cairo_paint (cr);
		g_object_unref(G_OBJECT(pb));
	}

	return true;
}

static bool _pv_element_raster_draw_focusing(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	const PvRenderContext render_context = render_option.render_context;

	const PvElementRasterData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}
	GdkPixbuf *pixbuf = data->pixbuf;
	double w = gdk_pixbuf_get_width(pixbuf);
	double h = gdk_pixbuf_get_height(pixbuf);
	w *= render_context.scale;
	h *= render_context.scale;
	double x = data->matrix.x;
	double y = data->matrix.y;
	x *= render_context.scale;
	y *= render_context.scale;

	cairo_rectangle(cr, x, y, w, h);
	cairo_set_line_width(cr, 1.0);
	_pv_render_workingcolor_cairo_set_source_rgba(cr);
	cairo_stroke(cr);

	return true;
}

static bool _is_inside_rect(
		double x_min, double x_max,
		double y_min, double y_max,
		double x, double y)
{
	return (x_min <= x && x <= x_max && y_min <= y && y <= y_max);
}

static bool _pv_element_raster_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		int offset,
		double gx,
		double gy)
{
	*is_touch = false;

	PvElementRasterData *data = element->data;

	GdkPixbuf *pixbuf = data->pixbuf;
	double w = gdk_pixbuf_get_width(pixbuf);
	double h = gdk_pixbuf_get_height(pixbuf);
	*is_touch = _is_inside_rect(
			data->matrix.x,
			data->matrix.x + w,
			data->matrix.y,
			data->matrix.y + h,
			gx,
			gy);

	return true;
}

static bool _pv_matrix_is_diff(const PvMatrix matrix1, const PvMatrix matrix2)
{
	return !(matrix1.x == matrix2.x && matrix1.y == matrix2.y);
}

static bool _pv_element_raster_is_diff_one(
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

	if(_pv_matrix_is_diff(data0->matrix, data1->matrix)){
		*is_diff = true;
		return false;
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

static PvPoint _pv_element_raster_get_point_by_anchor_points(
		const PvElement *element)
{
	assert(element);
	assert(element->data);
	const PvElementRasterData *data = element->data;

	PvPoint ret = {
		.x = data->matrix.x,
		.y = data->matrix.y,
	};

	return ret;
}

static void _pv_element_raster_set_point_by_anchor_points(
		PvElement *element,
		const PvPoint point)
{
	assert(element);
	assert(element->data);
	PvElementRasterData *data = element->data;

	data->matrix.x = point.x;
	data->matrix.y = point.y;
}

static bool _pv_element_raster_move_element(
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

	data->matrix.x += gx;
	data->matrix.y += gy;

	return true;
}

static PvAnchorPoint _pv_element_raster_get_anchor_point(
		const PvElement *element,
		const int index)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	PvAnchorPoint ap = PvAnchorPoint_Default;
	PvPoint point = {
		.x = data->matrix.x,
		.y = data->matrix.y,
	};
	pv_anchor_point_set_handle(&ap, PvAnchorPointIndex_Point, point);

	return ap;
}

static bool _pv_element_raster_set_anchor_point_point(
		const PvElement *element,
		const int index,
		const PvPoint point)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	data->matrix.x = point.x;
	data->matrix.y = point.y;

	return true;
}

static bool _pv_element_raster_move_anchor_point(
		const PvElement *element,
		const int index,
		const PvPoint move)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	switch(index){
		case 0:
			{
				data->matrix.x += move.x;
				data->matrix.y += move.y;
			}
			break;
		default:
			{
				pv_debug("Not implement.");
				return true;
			}
			break;
	}

	return true;
}

static PvRect _pv_element_raster_get_rect_anchor_points(
		const PvElement *element)
{
	assert(element);
	assert(PvElementKind_Raster == element->kind);

	PvElementRasterData *data = element->data;
	assert(data);

	PvRect rect = PvRect_Default;
	rect.x = data->matrix.x;
	rect.y = data->matrix.y;
	rect.w = gdk_pixbuf_get_width(data->pixbuf);
	rect.h = gdk_pixbuf_get_height(data->pixbuf);

	return rect;
}

/* ****************
 * ElementInfo配列の定義
 **************** */

const PvElementInfo _pv_element_info[] = {
	{PvElementKind_NotDefined, "NotDefined",
		.func_new_data			= _pv_element_error_return_null_new,
		.func_delete_data		= _pv_element_error_return_null_delete,
		.func_copy_new_data		= _pv_element_error_return_null_copy_new,
		.func_write_svg			= _pv_element_notimplement_svg_write,
		.func_draw			= _pv_element_notimplement_draw,
		.func_draw_focusing		= _pv_element_notimplement_draw,
		.func_is_touch_element		= _pv_element_notimplement_is_touch_element,
		.func_is_diff_one		= _pv_element_notimplement_is_diff_one,
		.func_get_point_by_anchor_points	= _pv_element_notimplement_get_point_by_anchor_points,
		.func_set_point_by_anchor_points	= _pv_element_notimplement_set_point_by_anchor_points,
		.func_move_element		= _pv_element_notimplement_move_element,
		.func_get_num_anchor_point	= _pv_element_zero_get_num_anchor_point,
		.func_new_anchor_points		= _pv_element_null_new_anchor_points,
		.func_get_anchor_point		= _pv_element_notimplement_get_anchor_point,
		.func_set_anchor_point_point	= _pv_element_notimplement_set_anchor_point_point,
		.func_move_anchor_point_point	= _pv_element_notimplement_move_anchor_point,
		.func_get_rect_by_anchor_points	= _pv_element_notimplement_get_rect_anchor_points,
	},
	{PvElementKind_Root, "Root",
		.func_new_data			= _pv_element_group_data_new,
		.func_delete_data		= _pv_element_group_data_delete,
		.func_copy_new_data		= _pv_element_group_data_copy_new,
		.func_write_svg			= _pv_element_group_write_svg,
		.func_draw			= _pv_element_group_draw,
		.func_draw_focusing		= _pv_element_group_draw,
		.func_is_touch_element		= _pv_element_group_is_touch_element,
		.func_is_diff_one		= _pv_element_group_is_diff_one,
		.func_get_point_by_anchor_points	= _pv_element_notimplement_get_point_by_anchor_points,
		.func_set_point_by_anchor_points	= _pv_element_notimplement_set_point_by_anchor_points,
		.func_move_element		= _pv_element_group_move_element,
		.func_get_num_anchor_point	= _pv_element_zero_get_num_anchor_point,
		.func_new_anchor_points		= _pv_element_null_new_anchor_points,
		.func_get_anchor_point		= _pv_element_notimplement_get_anchor_point,
		.func_set_anchor_point_point	= _pv_element_notimplement_set_anchor_point_point,
		.func_move_anchor_point_point	= _pv_element_notimplement_move_anchor_point,
		.func_get_rect_by_anchor_points	= _pv_element_notimplement_get_rect_anchor_points,
	},
	{PvElementKind_Layer, "Layer",
		.func_new_data			= _pv_element_group_data_new,
		.func_delete_data		= _pv_element_group_data_delete,
		.func_copy_new_data		= _pv_element_group_data_copy_new,
		.func_write_svg			= _pv_element_group_write_svg,
		.func_draw			= _pv_element_group_draw,
		.func_draw_focusing		= _pv_element_group_draw,
		.func_is_touch_element		= _pv_element_group_is_touch_element,
		.func_is_diff_one		= _pv_element_group_is_diff_one,
		.func_get_point_by_anchor_points	= _pv_element_notimplement_get_point_by_anchor_points,
		.func_set_point_by_anchor_points	= _pv_element_notimplement_set_point_by_anchor_points,
		.func_move_element		= _pv_element_group_move_element,
		.func_get_num_anchor_point	= _pv_element_zero_get_num_anchor_point,
		.func_new_anchor_points		= _pv_element_null_new_anchor_points,
		.func_get_anchor_point		= _pv_element_notimplement_get_anchor_point,
		.func_set_anchor_point_point	= _pv_element_notimplement_set_anchor_point_point,
		.func_move_anchor_point_point	= _pv_element_notimplement_move_anchor_point,
		.func_get_rect_by_anchor_points	= _pv_element_notimplement_get_rect_anchor_points,
	},
	{PvElementKind_Group, "Group",
		.func_new_data			= _pv_element_group_data_new,
		.func_delete_data		= _pv_element_group_data_delete,
		.func_copy_new_data		= _pv_element_group_data_copy_new,
		.func_write_svg			= _pv_element_group_write_svg,
		.func_draw			= _pv_element_group_draw,
		.func_draw_focusing		= _pv_element_group_draw,
		.func_is_touch_element		= _pv_element_group_is_touch_element,
		.func_is_diff_one		= _pv_element_group_is_diff_one,
		.func_get_point_by_anchor_points	= _pv_element_notimplement_get_point_by_anchor_points,
		.func_set_point_by_anchor_points	= _pv_element_notimplement_set_point_by_anchor_points,
		.func_move_element		= _pv_element_group_move_element,
		.func_get_num_anchor_point	= _pv_element_zero_get_num_anchor_point,
		.func_new_anchor_points		= _pv_element_null_new_anchor_points,
		.func_get_anchor_point		= _pv_element_notimplement_get_anchor_point,
		.func_set_anchor_point_point	= _pv_element_notimplement_set_anchor_point_point,
		.func_move_anchor_point_point	= _pv_element_notimplement_move_anchor_point,
		.func_get_rect_by_anchor_points	= _pv_element_notimplement_get_rect_anchor_points,
	},
	{PvElementKind_Bezier, "Bezier",
		.func_new_data			= _pv_element_bezier_data_new,
		.func_delete_data		= _pv_element_bezier_data_delete,
		.func_copy_new_data		= _pv_element_bezier_data_copy_new,
		.func_write_svg			= _pv_element_bezier_write_svg,
		.func_draw			= _pv_element_bezier_draw,
		.func_draw_focusing		= _pv_element_bezier_draw_focusing,
		.func_is_touch_element		= _pv_element_bezier_is_touch_element,
		.func_is_diff_one		= _pv_element_bezier_is_diff_one,
		.func_get_point_by_anchor_points	= _pv_element_bezier_get_point_by_anchor_points,
		.func_set_point_by_anchor_points	= _pv_element_bezier_set_point_by_anchor_points,
		.func_move_element		= _pv_element_bezier_move_element,
		.func_get_num_anchor_point	= _pv_element_bezier_get_num_anchor_point,
		.func_new_anchor_points		= _pv_element_bezier_new_anchor_points,
		.func_get_anchor_point		= _pv_element_bezier_get_anchor_point,
		.func_set_anchor_point_point	= _pv_element_bezier_set_anchor_point_point,
		.func_move_anchor_point_point	= _pv_element_bezier_move_anchor_point,
		.func_get_rect_by_anchor_points	= _pv_element_bezier_get_rect_anchor_points,
	},
	{PvElementKind_Raster, "Raster",
		.func_new_data			= _pv_element_raster_data_new,
		.func_delete_data		= _pv_element_raster_data_delete,
		.func_copy_new_data		= _pv_element_raster_data_copy_new,
		.func_write_svg			= _pv_element_notimplement_svg_write,
		.func_draw			= _pv_element_raster_draw,
		.func_draw_focusing		= _pv_element_raster_draw_focusing,
		.func_is_touch_element		= _pv_element_raster_is_touch_element,
		.func_is_diff_one		= _pv_element_raster_is_diff_one,
		.func_get_point_by_anchor_points	= _pv_element_raster_get_point_by_anchor_points,
		.func_set_point_by_anchor_points	= _pv_element_raster_set_point_by_anchor_points,
		.func_move_element		= _pv_element_raster_move_element,
		.func_get_num_anchor_point	= _pv_element_zero_get_num_anchor_point,
		.func_new_anchor_points		= _pv_element_null_new_anchor_points,
		.func_get_anchor_point		= _pv_element_raster_get_anchor_point,
		.func_set_anchor_point_point	= _pv_element_raster_set_anchor_point_point,
		.func_move_anchor_point_point	= _pv_element_raster_move_anchor_point,
		.func_get_rect_by_anchor_points	= _pv_element_raster_get_rect_anchor_points,
	},
	/* 番兵 */
	{PvElementKind_EndOfKind, "EndOfKind",
		.func_new_data			= _pv_element_error_return_null_new,
		.func_delete_data		= _pv_element_error_return_null_delete,
		.func_copy_new_data		= _pv_element_error_return_null_copy_new,
		.func_write_svg			= _pv_element_notimplement_svg_write,
		.func_draw			= _pv_element_notimplement_draw,
		.func_draw_focusing		= _pv_element_notimplement_draw,
		.func_is_touch_element		= _pv_element_notimplement_is_touch_element,
		.func_is_diff_one		= _pv_element_notimplement_is_diff_one,
		.func_get_point_by_anchor_points	= _pv_element_notimplement_get_point_by_anchor_points,
		.func_set_point_by_anchor_points	= _pv_element_notimplement_set_point_by_anchor_points,
		.func_move_element		= _pv_element_notimplement_move_element,
		.func_get_num_anchor_point	= _pv_element_zero_get_num_anchor_point,
		.func_new_anchor_points		= _pv_element_null_new_anchor_points,
		.func_get_anchor_point		= _pv_element_notimplement_get_anchor_point,
		.func_set_anchor_point_point	= _pv_element_notimplement_set_anchor_point_point,
		.func_move_anchor_point_point	= _pv_element_notimplement_move_anchor_point,
		.func_get_rect_by_anchor_points	= _pv_element_notimplement_get_rect_anchor_points,
	},
};



/* ****************
 * ElementInfo関連関数の定義
 **************** */

const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind)
{

	if(PvElementKind_NotDefined == kind || PvElementKind_EndOfKind == kind){
		pv_error("%d", kind);
		return NULL;
	}

	int num = sizeof(_pv_element_info) / sizeof(PvElementInfo);
	for(int i = 0; i < num; i++){
		if(kind == _pv_element_info[i].kind){
			return &_pv_element_info[i];
		}
	}

	pv_bug("%d", kind);
	return NULL;
}

