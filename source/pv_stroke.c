#include "pv_stroke.h"

#include <cairo.h>
#include "pv_error.h"



const PvStrokeLinecapInfo pvStrokeLinecapInfos[] = {
	{PvStrokeLinecap_Butt, "Butt", CAIRO_LINE_CAP_BUTT, },
	{PvStrokeLinecap_Round, "Round", CAIRO_LINE_CAP_ROUND, },
	{PvStrokeLinecap_Square, "Square", CAIRO_LINE_CAP_SQUARE, },
};

int get_num_stroke_linecap_infos()
{
	return sizeof(pvStrokeLinecapInfos)/sizeof(pvStrokeLinecapInfos[0]);
}

const PvStrokeLinecapInfo *get_stroke_linecap_info_from_id(PvStrokeLinecap linecap)
{
	int num = get_num_stroke_linecap_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinecapInfo *info = &(pvStrokeLinecapInfos[i]);
		if(info->linecap == linecap){
			return info;
		}
	}

	pv_bug("%d", linecap);
	return NULL;
}

const PvStrokeLinejoinInfo pvStrokeLinejoinInfos[] = {
	{PvStrokeLinejoin_Miter, "Miter", CAIRO_LINE_JOIN_MITER, },
	{PvStrokeLinejoin_Round, "Round", CAIRO_LINE_JOIN_ROUND, },
	{PvStrokeLinejoin_Bevel, "Bevel", CAIRO_LINE_JOIN_BEVEL, },
};

int get_num_stroke_linejoin_infos()
{
	return sizeof(pvStrokeLinejoinInfos)/sizeof(pvStrokeLinejoinInfos[0]);
}

const PvStrokeLinejoinInfo *get_stroke_linejoin_info_from_id(PvStrokeLinejoin linejoin)
{
	int num = get_num_stroke_linejoin_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinejoinInfo *info = &(pvStrokeLinejoinInfos[i]);
		if(info->linejoin == linejoin){
			return info;
		}
	}

	pv_bug("%d", linejoin);
	return NULL;
}



bool pv_stroke_is_equal(PvStroke stroke0, PvStroke stroke1)
{
	if(stroke0.width != stroke1.width){
		return false;
	}
	if(stroke0.linecap != stroke1.linecap){
		return false;
	}
	if(stroke0.linejoin != stroke1.linejoin){
		return false;
	}

	return true;
}

