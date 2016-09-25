/*! 	@file
 *	@brief stroke struct and utilitys.
 *
 * 	@attention convert methods is hard coding Magick Number.
 */
#ifndef include_PV_STROKE_H
#define include_PV_STROKE_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct{
	double width;
}PvStroke;

static const PvStroke PvStroke_Default = {
	1.0
};

bool pv_stroke_is_equal(PvStroke, PvStroke);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_STROKE_H

