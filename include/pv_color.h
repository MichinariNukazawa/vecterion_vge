/*! 	@file
 *	@brief color struct and utilitys.
 *
 * 	@attention convert methods is hard coding Magick Number.
 */
#ifndef include_PV_COLOR_H
#define include_PV_COLOR_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum{
	PvColorParameterIx_R,
	PvColorParameterIx_G,
	PvColorParameterIx_B,
	PvColorParameterIx_O, // Opacity
}PvColorParameterIx;

#define NUM_COLOR_PARAMETER (4)

typedef struct{
	/*! NOTICE: do not direct access! to RGB color parameters.
	 *		please use PvColor utility functions.
	 *		because feature implement to CMYK, color pallet relation.
	 */
	uint8_t values[NUM_COLOR_PARAMETER];
}PvColor;

//! none set color.
static const PvColor PvColor_None = {
	{0, 0, 0, 0,}
};

static const PvColor PvColor_Working = {
	{(uint8_t)(0.2 * 255), (uint8_t)(0.4 * 255), (uint8_t)(0.9 * 255), (uint8_t)(1.0 * 100),},
};

typedef enum{
	PvColorPairGround_ForGround,
	PvColorPairGround_BackGround,
}PvColorPairGround;
#define PvColorPairGrounds (2)

typedef struct{
	PvColor colors[PvColorPairGrounds];
}PvColorPair;

//! none set color. 
static const PvColorPair PvColorPair_None = {
	{{{0, 0, 0, 0,}}, {{0, 0, 0, 0,}},},
};

//! default color. stroke:Black/fill:White
static const PvColorPair PvColorPair_Default = {
	{{{0, 0, 0, 100,}}, {{255, 255, 255, 100,}},},
};

static const PvColorPair PvColorPair_Black = {
	{{{0, 0, 0, 100,}}, {{0, 0, 0, 100,}},},
};

static const PvColorPair PvColorPair_TransparentBlack = {
	{{{0, 0, 0, 0,}}, {{0, 0, 0, 0,}},},
};

typedef struct{
	PvColorParameterIx ix;
	const char *name;
	uint8_t min;
	uint8_t max;
}PvColorParameterProperty;

typedef struct{
	double r, g, b, a;
}PvCairoRgbaColor;

int pv_color_parameter_property_get_num();
const PvColorParameterProperty *pv_color_get_parameter_property_from_ix(PvColorParameterIx);
GdkColor pv_color_get_gdk_from_pv(PvColor);
PvCairoRgbaColor pv_color_get_cairo_rgba(PvColor);
char* pv_color_new_str_svg_rgba_simple(PvColor);

/*! @brief set parameter(and check value)
 *
 * @param double *value
 *		if invalid value(ex. out of range) to set shrink value.
 *		and return false.
 */
bool pv_color_set_parameter(PvColor *, PvColorParameterIx, double);

bool pv_color_is_equal(PvColor, PvColor);
bool pv_color_pair_is_equal(PvColorPair, PvColorPair);

#define debug_print_pv_color(color) \
	do{ \
		pv_debug("PvColor(%3d, %3d, %3d, %3d)", \
				(((PvColor)color).values[PvColorParameterIx_R]), \
				(color.values[PvColorParameterIx_G]), \
				(color.values[PvColorParameterIx_B]), \
				(color.values[PvColorParameterIx_O])); \
	}while(0);

#define debug_print_pv_cairo_rgba_color(cc) \
	do{ \
		pv_debug("PvCairoRgbaColor(%4.3f, %4.3f, %4.3f, %4.3f)", \
				((PvCairoRgbaColor)cc).r, cc.g, cc.b, cc.a); \
	}while(0);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_COLOR_H

