//! @file
#include "pv_rotate.h"

#include <math.h>



static double _get_radian_from_degree(double degree)
{
	return degree * (M_PI / 180.0);
}

PvPoint pv_rotate_point(PvPoint point, double degree, PvPoint center)
{
	PvPoint rel = pv_point_sub(point, center);
	double radian = _get_radian_from_degree(degree);
	PvPoint aft = {
		.x = (rel.x * cos(radian)) - (rel.y * sin(radian)),
		.y = (rel.x * sin(radian)) + (rel.y * cos(radian)),
	};

	aft = pv_point_add(aft, center);

	return aft;
}

