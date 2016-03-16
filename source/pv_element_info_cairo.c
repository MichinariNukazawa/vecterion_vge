#include "pv_element_info_cairo.h"

void _pv_render_workingcolor_cairo_set_source_rgb(cairo_t *cr)
{
	cairo_set_source_rgb (cr, 0.2, 0.4, 0.9);
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
	cairo_path_extents(cr, &x1, &y1, &x2, &y2);
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

