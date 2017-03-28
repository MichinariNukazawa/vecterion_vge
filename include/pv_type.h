/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_TYPE_H
#define include_PV_TYPE_H

#include <stdbool.h>

// http://stackoverflow.com/questions/7356523/linking-math-library-to-a-c90-code-using-gcc
// http://www.sbin.org/doc/glibc/libc_19.html
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

#define DELTA_OF_RESIZE (0.000001)
#define PV_DELTA (0.000001)


// ******** ********
// PvPoint
// ******** ********
typedef struct PvPoint{
	double x;
	double y;
}PvPoint;

static const PvPoint PvPoint_Default = {0, 0};

PvPoint pv_point_add(PvPoint, PvPoint);
PvPoint pv_point_sub(PvPoint, PvPoint);
PvPoint pv_point_mul(PvPoint, PvPoint);
PvPoint pv_point_div(PvPoint, PvPoint);
PvPoint pv_point_add_value(PvPoint, double);
PvPoint pv_point_mul_value(PvPoint, double);
PvPoint pv_point_div_value(PvPoint, double);

PvPoint pv_point_abs(PvPoint);
PvPoint pv_point_exchange(PvPoint);
PvPoint pv_point_subdivide(PvPoint, PvPoint, double percent);
double pv_point_distance(PvPoint, PvPoint);

bool pv_point_is_diff(PvPoint, PvPoint);

PvPoint pv_point_rescale(PvPoint point, PvPoint scale, PvPoint center);

// ******** ********
// PvRect
// ******** ********
//! clockwise rotation because use "for()"
typedef enum{
	PvRectEdgeKind_UpLeft,
	PvRectEdgeKind_UpRight,
	PvRectEdgeKind_DownRight,
	PvRectEdgeKind_DownLeft,
}PvRectEdgeKind;

typedef struct PvRect{
	double x;
	double y;
	double w;
	double h;
}PvRect;

static const PvRect PvRect_Default = {0, 0, 0, 0,};

PvRect pv_rect_mul_value(PvRect, double);
PvRect pv_rect_abs_size(PvRect);

bool pv_rect_is_inside(PvRect, PvPoint);
PvRect pv_rect_expand(PvRect rect0, PvRect rect1);
PvPoint pv_rect_get_edge_point(PvRect, PvRectEdgeKind);
PvPoint pv_rect_get_center(PvRect);


#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_TYPE_H

