#include "pv_element_info_cairo.h"

#include "pv_error.h"
#include "pv_color.h"

void _pv_render_workingcolor_cairo_set_source_rgba(cairo_t *cr)
{
	cairo_set_source_rgba (cr, 0.2, 0.4, 0.9, 1.0);
}

void debug_print_path(cairo_t *cr)
{
	double x1, y1, x2, y2;
	cairo_path_extents(cr, &x1, &y1, &x2, &y2);
	pv_debug("Path:%f,%f %f,%f ", x1, y1, x2, y2);
}

PvRect _pv_renderer_get_rect_extent_from_cr(cairo_t *cr)
{
	PvRect rect = {0,0,0,0};
	double x1, y1, x2, y2;
	cairo_stroke_extents(cr, &x1, &y1, &x2, &y2);
	rect.x = x1;
	rect.y = y1;
	rect.w = x2 - x1;
	rect.h = y2 - y1;

	return rect;
}

void _pv_renderer_draw_extent_from_crect(cairo_t *cr, const PvRect rect)
{
	if(NULL == cr){
		pv_bug("");
		return;
	}
	cairo_rectangle (cr, rect.x, rect.y, rect.w, rect.h);
	cairo_set_source_rgba (cr, 0.7, 0, 0, 0.5);
	cairo_fill (cr);
}

bool _pv_element_bezier_command_path(
		cairo_t *cr,
		const PvRenderContext render_context,
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

	// ** path stroking
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

