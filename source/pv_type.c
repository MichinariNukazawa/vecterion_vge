#include "pv_type.h"

#include <math.h>
#include "pv_error.h"



// ******** ********
// PvPoint
// ******** ********
PvPoint pv_point_add(PvPoint p, PvPoint p_diff)
{
	PvPoint ret = {
		.x = p.x + p_diff.x,
		.y = p.y + p_diff.y,
	};

	return ret;
}

PvPoint pv_point_sub(PvPoint p0, PvPoint p1)
{
	PvPoint ret = {
		.x = p0.x - p1.x,
		.y = p0.y - p1.y,
	};

	return ret;
}

PvPoint pv_point_mul(PvPoint p0, PvPoint p1)
{
	PvPoint ret = {
		.x = p0.x * p1.x,
		.y = p0.y * p1.y,
	};

	return ret;
}

PvPoint pv_point_add_value(PvPoint p, double diff)
{
	PvPoint ret = {
		.x = p.x + diff,
		.y = p.y + diff,
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

PvPoint pv_point_div_value(PvPoint p, double scale)
{
	PvPoint ret = {
		.x = p.x / scale,
		.y = p.y / scale
	};

	return ret;
}

PvPoint pv_point_abs(PvPoint p)
{
	PvPoint ret = {
		.x = fabs(p.x),
		.y = fabs(p.y),
	};

	return ret;
}

PvPoint pv_point_exchange(PvPoint p)
{
	PvPoint ret = {
		.x = p.y,
		.y = p.x,
	};

	return ret;
}

PvPoint pv_point_subdivide(PvPoint start, PvPoint end, double percent)
{
	PvPoint diff = pv_point_sub(end, start);
	PvPoint ret = pv_point_add(start, pv_point_mul_value(diff, (percent / 100.0)));

	return ret;
}

double pv_point_distance(PvPoint p0, PvPoint p1)
{
	PvPoint p = pv_point_abs(pv_point_sub(p0, p1));
	double ret = sqrt((p.x * p.x) + (p.y * p.y));

	return ret;
}

bool pv_point_is_diff(PvPoint point0, PvPoint point1)
{
	if(point0.x != point1.x){
		return true;
	}
	if(point0.y != point1.y){
		return true;
	}

	return false;
}


// ******** ********
// PvRect
// ******** ********
PvRect pv_rect_mul_value(PvRect r, double value)
{
	PvRect ret = {
		.x = r.x * value,
		.y = r.y * value,
		.w = r.w * value,
		.h = r.h * value,
	};

	return ret;
}

PvRect pv_rect_abs_size(PvRect r)
{
	if(r.w < 0){
		r.x += r.w;
		r.w = fabs(r.w);
	}
	if(r.h < 0){
		r.y += r.h;
		r.h = fabs(r.h);
	}

	return r;
}

bool pv_rect_is_inside(PvRect rect, PvPoint point)
{
	if(point.x < rect.x || (rect.x + rect.w) < point.x){
		return false;
	}
	if(point.y < rect.y || (rect.y + rect.h) < point.y){
		return false;
	}

	return true;
}

//! @brief expanding PvRect area, logical operation "and"
PvRect pv_rect_expand(PvRect rect0, PvRect rect1)
{
	PvRect ret = rect0;
	ret.x = (rect0.x < rect1.x)? rect0.x : rect1.x;
	ret.y = (rect0.y < rect1.y)? rect0.y : rect1.y;
	double x0 = rect0.x + rect0.w;
	double y0 = rect0.y + rect0.h;
	double x1 = rect1.x + rect1.w;
	double y1 = rect1.y + rect1.h;
	double x = (x0 > x1)? x0 : x1;
	double y = (y0 > y1)? y0 : y1;
	ret.w = x - ret.x;
	ret.h = y - ret.y;

	return ret;
}

PvPoint pv_rect_get_edge_point(PvRect rect, PvRectEdgeKind edgeKind)
{
	switch(edgeKind){
		case PvRectEdgeKind_UpLeft:
			return (PvPoint){rect.x, rect.y};
		case PvRectEdgeKind_UpRight:
			return (PvPoint){(rect.x + rect.w), rect.y};
		case PvRectEdgeKind_DownLeft:
			return (PvPoint){rect.x, (rect.y + rect.h)};
		case PvRectEdgeKind_DownRight:
			return (PvPoint){(rect.x + rect.w), (rect.y + rect.h)};
		default:
			pv_assertf(false, "%d", edgeKind);
	}
}

PvPoint pv_rect_get_center(PvRect rect)
{
	PvPoint center = {
		.x = rect.x + (rect.w / 2),
		.y = rect.y + (rect.h / 2),
	};

	return center;
}

