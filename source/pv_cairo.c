#include "pv_cairo.h"

void pv_cairo_set_source_rgba_workingcolor(cairo_t *cr)
{
	//! @todo workingcolor constant value join to PvColor_Working
	cairo_set_source_rgba (cr, 0.2, 0.4, 0.9, 1.0);
}

void pv_cairo_set_source_rgba_subworkingcolor(cairo_t *cr)
{
	//! @todo workingcolor constant value join to PvColor_Working
	cairo_set_source_rgba (cr, 0.3, 0.6, 0.9, 1.0);
}

void pv_cairo_fill_checkboard(cairo_t *cr, PvRect rect)
{
	// clipping
	cairo_save(cr);
	cairo_rectangle(cr, rect.x, rect.y, rect.w, rect.h);
	cairo_clip_preserve(cr);

	// ** fill background of checkboard
	cairo_set_source_rgba(cr, 0.6, 0.6, 0.6, 1.0);
	cairo_fill(cr);

	// ** draw checkboard
	cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
	int unit = 8;
	for(int y = 0; y < rect.h; y += unit){
		for(int x = 0 + (((y/unit) % 2) * unit); x < rect.w; x += (unit * 2)){
			cairo_rectangle (cr, x, y, unit, unit);
		}
	}
	cairo_fill (cr);

	cairo_restore(cr); // clear clipping
}

