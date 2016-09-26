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

/*! @brief StrokeLinecap */
typedef enum{
	PvStrokeLinecap_Butt,
	PvStrokeLinecap_Round,
	PvStrokeLinecap_Square,
}PvStrokeLinecap;

typedef struct{
	PvStrokeLinecap linecap;
	const char *name;
	int cairo_value;
}PvStrokeLinecapInfo;

int get_num_stroke_linecap_infos();
const PvStrokeLinecapInfo *get_stroke_linecap_info_from_id(PvStrokeLinecap linecap);



/*! @brief StrokeLinejoin */
typedef enum{
	PvStrokeLinejoin_Miter,
	PvStrokeLinejoin_Round,
	PvStrokeLinejoin_Bevel,
}PvStrokeLinejoin;

typedef struct{
	PvStrokeLinejoin linejoin;
	const char *name;
	int cairo_value;
}PvStrokeLinejoinInfo;

int get_num_stroke_linejoin_infos();
const PvStrokeLinejoinInfo *get_stroke_linejoin_info_from_id(PvStrokeLinejoin linejoin);



/*! @brief Stroke */
typedef struct{
	double width;
	PvStrokeLinecap linecap;
	PvStrokeLinejoin linejoin;
}PvStroke;

static const PvStroke PvStroke_Default = {
	1.0,
	PvStrokeLinecap_Butt,
	PvStrokeLinejoin_Miter,
};

bool pv_stroke_is_equal(PvStroke, PvStroke);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_STROKE_H

