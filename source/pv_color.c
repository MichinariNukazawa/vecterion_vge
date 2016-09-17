#include "pv_color.h"

#include "pv_error.h"

const PvColorParameterProperty color_parameter_propertys[] = {
	{
		PvColorParameterIx_R,
		"R",
		0,
		255,
	},
	{
		PvColorParameterIx_G,
		"G",
		0,
		255,
	},

	{
		PvColorParameterIx_B,
		"B",
		0,
		255,
	},

	{
		PvColorParameterIx_O,
		"A",
		0,
		100,
	},
};

int pv_color_parameter_property_get_num()
{
	return sizeof(color_parameter_propertys)/sizeof(color_parameter_propertys[0]);
}

const PvColorParameterProperty *pv_color_get_parameter_property_from_ix(PvColorParameterIx ix)
{
	if(ix < 0 || pv_color_parameter_property_get_num() <= (int)ix){
		pv_bug("%d", ix);
		exit(-1);
		ix = 0;
	}

	return &(color_parameter_propertys[ix]);
}

GdkColor pv_color_get_gdk_from_pv(PvColor pv_color)
{
	GdkColor gdk_color;
	gdk_color.red	= pv_color.values[PvColorParameterIx_R] * 255;
	gdk_color.green	= pv_color.values[PvColorParameterIx_G] * 255;
	gdk_color.blue	= pv_color.values[PvColorParameterIx_B] * 255;

	return gdk_color;
}

PvCairoRgbaColor pv_color_get_cairo_rgba(PvColor color)
{
	PvCairoRgbaColor c = {
			(color.values[PvColorParameterIx_R] / 255.0),
			(color.values[PvColorParameterIx_G] / 255.0),
			(color.values[PvColorParameterIx_B] / 255.0),
			(color.values[PvColorParameterIx_O] / 100.0)
	};

	return c;
}

char* pv_color_new_str_svg_rgba_simple(PvColor color)
{
	return g_strdup_printf("rgba(%d,%d,%d,%f)",
			(color.values[PvColorParameterIx_R]),
			(color.values[PvColorParameterIx_G]),
			(color.values[PvColorParameterIx_B]),
			(color.values[PvColorParameterIx_O] / 100.0)
			);
}

bool pv_color_set_parameter(PvColor *color, PvColorParameterIx ix, double value)
{
	assert(color);
	bool ret = true;

	const PvColorParameterProperty *color_parameter_property
			= pv_color_get_parameter_property_from_ix(ix);
	assert(color_parameter_property);

	if(value < color_parameter_property->min){
		pv_warning("%f", value);
		value = color_parameter_property->min;
		ret = false;
	}else if(color_parameter_property->max < value){
		pv_warning("%f", value);
		value = color_parameter_property->max;
		ret = false;
	}

	color->values[ix] = value;

	return ret;
}

bool pv_color_is_equal(PvColor color1, PvColor color2)
{
	for(int i = 0; i < 4; i++){
		if(color1.values[i] != color2.values[i]){
			return false;
		}
	}

	return true;
}

bool pv_color_pair_is_equal(PvColorPair cp1, PvColorPair cp2)
{
	if(!pv_color_is_equal(cp1.colors[PvColorPairGround_ForGround],
			cp2.colors[PvColorPairGround_ForGround])){
		return false;
	}

	if(!pv_color_is_equal(cp1.colors[PvColorPairGround_BackGround],
			cp2.colors[PvColorPairGround_BackGround])){
		return false;
	}

	return true;
}

