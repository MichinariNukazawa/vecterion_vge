#ifndef include_PV_ELEMENT_INFO_CAIRO_H
#define include_PV_ELEMENT_INFO_CAIRO_H

#include <stdbool.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "pv_type.h"
#include "pv_element.h"
#include "pv_render_context.h"

void _pv_render_workingcolor_cairo_set_source_rgb(cairo_t *cr);
void debug_print_path(cairo_t *cr);
PvRect _pv_renderer_get_rect_extent_from_cr(cairo_t *cr);
void _pv_renderer_draw_extent_from_crect(cairo_t *cr, const PvRect rect);
bool _pv_element_bezier_command_path(
		cairo_t *cr,
		const PvRenderContext render_context,
		int goffset, // use detection
		const PvElement *element);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_INFO_CAIRO_H

