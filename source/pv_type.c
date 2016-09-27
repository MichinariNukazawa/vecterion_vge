#include "pv_type.h"

bool pv_rect_is_inside(PvRect rect, int x, int y)
{
	if(x < rect.x || (rect.x + rect.w) < x){
		return false;
	}
	if(y < rect.y || (rect.y + rect.h) < y){
		return false;
	}

	return true;
}

PvPoint pv_point_sub(PvPoint p0, PvPoint p1)
{
	PvPoint ret = {
		.x = p0.x - p1.x,
		.y = p0.y - p1.y,
	};

	return ret;
}

PvPoint pv_point_mul_value(PvPoint p0, double value)
{
	PvPoint ret = {
		.x = p0.x * value,
		.y = p0.y * value,
	};

	return ret;
}

