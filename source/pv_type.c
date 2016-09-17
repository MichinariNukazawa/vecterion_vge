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

