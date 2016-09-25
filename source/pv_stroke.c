#include "pv_stroke.h"

bool pv_stroke_is_equal(PvStroke stroke0, PvStroke stroke1)
{
	if(stroke0.width != stroke1.width){
		return false;
	}

	return true;
}
