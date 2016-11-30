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

PvRect pv_rotate_rect(PvRect rect, double degree, PvPoint center)
{
	PvPoint ul = {
		.x = rect.x,
		.y = rect.y,
	};

	ul = pv_rotate_point(ul, degree, center);

	PvPoint dr = {
		.x = rect.x + rect.w,
		.y = rect.y + rect.h,
	};

	dr = pv_rotate_point(dr, degree, center);

	PvRect r = {
		.x = ul.x,
		.y = ul.y,
		.w = dr.x - ul.x,
		.h = dr.y - ul.y,
	};

	return r;
}

