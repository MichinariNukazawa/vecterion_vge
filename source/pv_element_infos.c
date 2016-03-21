#include "pv_element_infos.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"
#include "pv_element_info_cairo.h"
#include "pv_render_context.h"


/* ****************
 * null functions. (ex. use method not implement)
 **************** */

/** @brief 無効なindexを引いた際に埋め込まれているダミー関数 */
gpointer _pv_element_error_return_null_new()
{
	pv_error("");
	return NULL;
}

bool _pv_element_error_return_null_delete(void *_data)
{
	pv_error("");
	return false;
}

gpointer _pv_element_error_return_null_copy_new(void *_data)
{
	pv_error("");
	return NULL;
}

/** @brief write_svg未実装箇所に挿入する */
int _pv_element_write_svg_notimplement(
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

bool _pv_element_notimplement_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	pv_error("");
	return true;
}

bool _pv_element_notimplement_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		double gx,
		double gy)
{
	pv_error("");
	*is_touch = false;
	return true;
}

bool _pv_element_notimplement_is_diff_one(
		bool *is_diff,
		const PvElement *element0,
		const PvElement *element1)
{
	pv_error("");
	*is_diff = true;
	return true;
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

gpointer _pv_element_group_data_new()
{
	PvElementGroupData *data = (PvElementGroupData *)malloc(sizeof(PvElementGroupData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->name = NULL;

	return (gpointer)data;
}

bool _pv_element_group_data_delete(void *_data)
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

gpointer _pv_element_group_data_copy_new(void *_data)
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

int _pv_element_group_write_svg(
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

bool _pv_element_group_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	return true;
}

bool _pv_element_group_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		double gx,
		double gy)
{
	*is_touch = false;
	return true;
}

bool pv_general_strcmp(char *str0, char *str1)
{
	if(NULL == str0 && NULL == str1){
		return true;
	}
	if(NULL == str0 || NULL == str1){
		return false;
	}

	return (0 == strcmp(str0, str1));
}

bool _pv_element_group_is_diff_one(
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

	*is_diff = !pv_general_strcmp(data0->name, data1->name);
	return true;
}

/* ****************
 * Bezier
 **************** */

gpointer _pv_element_bezier_data_new()
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

bool _pv_element_bezier_data_delete(void *_data)
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

gpointer _pv_element_bezier_data_copy_new(void *_data)
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

char *_pv_element_bezier_new_str_from_anchor(const PvAnchorPoint ap_current, const PvAnchorPoint ap_prev)
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

int _pv_element_bezier_write_svg(
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

	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "path");
	xmlNewProp(node, BAD_CAST "fill", BAD_CAST "none");
	xmlNewProp(node, BAD_CAST "stroke", BAD_CAST "black");
	xmlNewProp(node, BAD_CAST "stroke-width", BAD_CAST "1");
	xmlNewProp(node, BAD_CAST "d", BAD_CAST str_current);

	g_free(str_current);

	xmlAddChild(target->xml_parent_node, node);
	//target->xml_parent_node = node;
	target->xml_new_node = node;

	return 0;
}

bool _pv_element_bezier_command_path(
		cairo_t *cr,
		const PvRenderContext render_context,
		int goffset, // use detection
		const PvElement *element)
{
	const PvElementBezierData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}
	PvAnchorPoint * const anchor_points = data->anchor_points;

	if((data->anchor_points_num) <= 0){
		return true;
	}

	// ** draw property is element or focus element
	cairo_set_source_rgb (cr, 0.1, 0.1, 0.1);
	double offset = (goffset * (1.0 / render_context.scale));
	double width = 2.0 + offset;
	cairo_set_line_width(cr, width);

	// ** stroke
	double first_ap_x = 0, first_ap_y = 0, second_ap_x = 0, second_ap_y = 0;
	for(int i = 0; i < data->anchor_points_num; i++){
		double x = anchor_points[i].points[PvAnchorPointIndex_Point].x;
		double y = anchor_points[i].points[PvAnchorPointIndex_Point].y;

		if(0 == i){
			x *= render_context.scale;
			y *= render_context.scale;

			if(1 == data->anchor_points_num){
				cairo_rectangle (cr, x, y, 2, 2);
				cairo_fill (cr);
			}else{
				cairo_move_to(cr, x, y);
			}
		}else{
			const PvAnchorPoint *ap_prev = &anchor_points[i - 1];
			first_ap_x = ap_prev->points[PvAnchorPointIndex_HandleNext].x
				+ ap_prev->points[PvAnchorPointIndex_Point].x;
			first_ap_y = ap_prev->points[PvAnchorPointIndex_HandleNext].y
				+ ap_prev->points[PvAnchorPointIndex_Point].y;

			second_ap_x = anchor_points[i].points[PvAnchorPointIndex_HandlePrev].x;
			second_ap_y = anchor_points[i].points[PvAnchorPointIndex_HandlePrev].y;
			second_ap_x += x;
			second_ap_y += y;

			x *= render_context.scale;
			y *= render_context.scale;
			first_ap_x *= render_context.scale;
			first_ap_y *= render_context.scale;
			second_ap_x *= render_context.scale;
			second_ap_y *= render_context.scale;
			cairo_curve_to(cr, first_ap_x, first_ap_y, second_ap_x, second_ap_y, x, y);
		}

	}
	if(data->is_close){
		double x = anchor_points[0].points[PvAnchorPointIndex_Point].x;
		double y = anchor_points[0].points[PvAnchorPointIndex_Point].y;

		const PvAnchorPoint *ap_last = &anchor_points[data->anchor_points_num - 1];
		first_ap_x = ap_last->points[PvAnchorPointIndex_HandleNext].x
			+ ap_last->points[PvAnchorPointIndex_Point].x;
		first_ap_y = ap_last->points[PvAnchorPointIndex_HandleNext].y
			+ ap_last->points[PvAnchorPointIndex_Point].y;

		const PvAnchorPoint *ap_first = &anchor_points[0];
		second_ap_x = ap_first->points[PvAnchorPointIndex_HandlePrev].x
			+ ap_first->points[PvAnchorPointIndex_Point].x;
		second_ap_y = ap_first->points[PvAnchorPointIndex_HandlePrev].y
			+ ap_first->points[PvAnchorPointIndex_Point].y;

		first_ap_x *= render_context.scale;
		first_ap_y *= render_context.scale;
		second_ap_x *= render_context.scale;
		second_ap_y *= render_context.scale;
		x *= render_context.scale;
		y *= render_context.scale;
		cairo_curve_to(cr, first_ap_x, first_ap_y, second_ap_x, second_ap_y, x, y);
		cairo_close_path (cr);
	}

	return true;
}

bool _pv_element_bezier_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	const PvRenderContext render_context = render_option.render_context;

	if(!_pv_element_bezier_command_path(
		cr,
		render_context,
		0,
		element))
	{
		pv_error("");
		return false;
	}

	PvRect crect_extent = _pv_renderer_get_rect_extent_from_cr(cr);
	cairo_stroke(cr);
	if(render_context.is_extent_view && !render_context.is_focus){
		_pv_renderer_draw_extent_from_crect(cr, crect_extent);
	}

	if(render_context.is_focus){
		const PvElementBezierData *data = element->data;
		if(NULL == data){
			pv_error("");
			return false;
		}

		// ** anchor point
		cairo_set_line_width(cr, 1.0);
		_pv_render_workingcolor_cairo_set_source_rgb(cr);

		// ** prev anchor_point
		if(0 < (data->anchor_points_num)){
			int ix =  data->anchor_points_num - ((data->is_close) ? 1:2);
			const PvAnchorPoint ap = data->anchor_points[ix];
			PvPoint gp_point = ap.points[PvAnchorPointIndex_Point];
			PvPoint gp_next = pv_anchor_point_get_handle(ap, PvAnchorPointIndex_HandleNext);
			gp_point.x *= render_context.scale;
			gp_point.y *= render_context.scale;
			gp_next.x *= render_context.scale;
			gp_next.y *= render_context.scale;
			cairo_move_to(cr, gp_point.x, gp_point.y);
			cairo_line_to(cr, gp_next.x, gp_next.y);
			cairo_stroke(cr);
			cairo_arc (cr, gp_next.x, gp_next.y, 2.0, 0., 2 * M_PI);
			cairo_fill (cr);
		}
		// ** current anchor_point
		int ix = ((data->is_close)? 0 : data->anchor_points_num - 1);
		PvPoint gp_current = data->anchor_points[ix].points[PvAnchorPointIndex_Point];
		PvPoint gp_prev = pv_anchor_point_get_handle(data->anchor_points[ix], PvAnchorPointIndex_HandlePrev);
		PvPoint gp_next = pv_anchor_point_get_handle(data->anchor_points[ix], PvAnchorPointIndex_HandleNext);
		gp_current.x *= render_context.scale;
		gp_current.y *= render_context.scale;
		gp_prev.x *= render_context.scale;
		gp_prev.y *= render_context.scale;
		gp_next.x *= render_context.scale;
		gp_next.y *= render_context.scale;
		cairo_set_line_width(cr, 1.0);
		_pv_render_workingcolor_cairo_set_source_rgb(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_prev.x, gp_prev.y);
		cairo_stroke(cr);
		cairo_move_to(cr, gp_current.x, gp_current.y);
		cairo_line_to(cr, gp_next.x, gp_next.y);
		cairo_stroke(cr);

		_pv_render_workingcolor_cairo_set_source_rgb(cr);
		cairo_arc (cr, gp_prev.x, gp_prev.y, 2.0, 0., 2 * M_PI);
		cairo_arc (cr, gp_next.x, gp_next.y, 2.0, 0., 2 * M_PI);
		cairo_fill (cr);
	}

	return true;
}

bool _pv_element_bezier_is_touch_element(
		bool *is_touch,
		const PvElement *element,
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

	PvRenderContext render_context = PvRenderContext_default;
	if(!_pv_element_bezier_command_path(
		cr,
		render_context,
		10,
		element))
	{
		pv_error("");
		return false;
	}

	// PvRect crect_extent = _pv_renderer_get_rect_extent_from_cr(cr);
	*is_touch = cairo_in_stroke(cr, gx, gy);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return true;
}

bool _pv_element_bezier_is_diff_one(
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

/* ****************
 * Raster
 **************** */

gpointer _pv_element_raster_data_new()
{
	PvElementRasterData *data = (PvElementRasterData *)malloc(sizeof(PvElementRasterData));
	if(NULL == data){
		pv_critical("");
		exit(-1);
	}

	data->path = NULL;
	data->pixbuf = NULL;

	return (gpointer)data;
}

bool _pv_element_raster_data_delete(void *_data)
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

gpointer _pv_element_raster_data_copy_new(void *_data)
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

bool _pv_element_raster_draw(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element)
{
	const PvRenderContext render_context = render_option.render_context;

	PvElementRasterData *data = element->data;
	GdkPixbuf *pixbuf = data->pixbuf;
	double w = (double)gdk_pixbuf_get_width(pixbuf);
	double h = (double)gdk_pixbuf_get_height(pixbuf);
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
		gdk_cairo_set_source_pixbuf (cr, pb, 0, 0);
		cairo_paint (cr);
		g_object_unref(G_OBJECT(pb));
	}

	return true;
}

bool _pv_element_raster_is_touch_element(
		bool *is_touch,
		const PvElement *element,
		double gx,
		double gy)
{
	*is_touch = false;
	return true;
}

bool _pv_element_raster_is_diff_one(
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

	// ** dataX->path is not check.

	// ** TODO: check raster pixbuf.
	if(data0->pixbuf != data1->pixbuf){
		*is_diff = true;
		return true;
	}

	*is_diff = false;
	return true;
}

/* ****************
 * ElementInfo配列の定義
 **************** */

const PvElementInfo _pv_element_infos[] = {
	{PvElementKind_NotDefined, "NotDefined",
		_pv_element_error_return_null_new,
		_pv_element_error_return_null_delete,
		_pv_element_error_return_null_copy_new,
		_pv_element_write_svg_notimplement,
		.func_draw = _pv_element_notimplement_draw,
		.func_is_touch_element = _pv_element_notimplement_is_touch_element,
		.func_is_diff_one		= _pv_element_notimplement_is_diff_one,
	},
	{PvElementKind_Root, "Root",
		_pv_element_group_data_new,
		_pv_element_group_data_delete,
		_pv_element_group_data_copy_new,
		_pv_element_group_write_svg,
		.func_draw = _pv_element_group_draw,
		.func_is_touch_element = _pv_element_group_is_touch_element,
		.func_is_diff_one		= _pv_element_group_is_diff_one,
	},
	{PvElementKind_Layer, "Layer",
		_pv_element_group_data_new,
		_pv_element_group_data_delete,
		_pv_element_group_data_copy_new,
		_pv_element_group_write_svg,
		.func_draw = _pv_element_group_draw,
		.func_is_touch_element = _pv_element_group_is_touch_element,
		.func_is_diff_one		= _pv_element_group_is_diff_one,
	},
	{PvElementKind_Group, "Group",
		_pv_element_group_data_new,
		_pv_element_group_data_delete,
		_pv_element_group_data_copy_new,
		_pv_element_group_write_svg,
		.func_draw = _pv_element_group_draw,
		.func_is_touch_element = _pv_element_group_is_touch_element,
		.func_is_diff_one		= _pv_element_group_is_diff_one,
	},
	{PvElementKind_Bezier, "Bezier",
		_pv_element_bezier_data_new,
		_pv_element_bezier_data_delete,
		_pv_element_bezier_data_copy_new,
		_pv_element_bezier_write_svg,
		.func_draw = _pv_element_bezier_draw,
		.func_is_touch_element = _pv_element_bezier_is_touch_element,
		.func_is_diff_one		= _pv_element_bezier_is_diff_one,
	},
	{PvElementKind_Raster, "Raster",
		_pv_element_raster_data_new,
		_pv_element_raster_data_delete,
		_pv_element_raster_data_copy_new,
		_pv_element_write_svg_notimplement,
		.func_draw = _pv_element_raster_draw,
		.func_is_touch_element = _pv_element_raster_is_touch_element,
		.func_is_diff_one		= _pv_element_raster_is_diff_one,
	},
	/* 番兵 */
	{PvElementKind_EndOfKind, "EndOfKind",
		_pv_element_error_return_null_new,
		_pv_element_error_return_null_delete,
		_pv_element_error_return_null_copy_new,
		_pv_element_write_svg_notimplement,
		.func_draw = _pv_element_notimplement_draw,
		.func_is_touch_element = _pv_element_notimplement_is_touch_element,
		.func_is_diff_one		= _pv_element_notimplement_is_diff_one,
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

	int num = sizeof(_pv_element_infos) / sizeof(PvElementInfo);
	for(int i = 0; i < num; i++){
		if(kind == _pv_element_infos[i].kind){
			return &_pv_element_infos[i];
		}
	}

	pv_bug("%d", kind);
	return NULL;
}

