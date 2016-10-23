#ifndef include_PV_ELEMENT_INFO_CAIRO_H
#define include_PV_ELEMENT_INFO_CAIRO_H

#include <stdbool.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "pv_type.h"
#include "pv_element.h"
#include "pv_render_context.h"
#include "pv_cairo.h"

void debug_print_path(cairo_t *cr);
bool _pv_element_bezier_command_path(
		cairo_t *cr,
		const PvRenderContext render_context,
		const PvElement *element);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_INFO_CAIRO_H

