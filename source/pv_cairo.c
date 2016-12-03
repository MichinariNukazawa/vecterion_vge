#include "pv_cairo.h"

void pv_cairo_set_source_rgba_workingcolor(cairo_t *cr)
{
	//! @todo workingcolor constant value join to PvColor_Working
	cairo_set_source_rgba (cr, 0.2, 0.4, 0.9, 1.0);
}

