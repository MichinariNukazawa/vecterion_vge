#ifndef include_PV_TYPE_H
#define include_PV_TYPE_H

#include <stdbool.h>

// http://stackoverflow.com/questions/7356523/linking-math-library-to-a-c90-code-using-gcc
// http://www.sbin.org/doc/glibc/libc_19.html
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

typedef struct PvPoint{
	double x;
	double y;
}PvPoint;

static const PvPoint PvPoint_Default = {0, 0};

PvPoint pv_point_add(PvPoint, PvPoint);
PvPoint pv_point_sub(PvPoint, PvPoint);
PvPoint pv_point_add_value(PvPoint, double);
PvPoint pv_point_mul_value(PvPoint, double);
PvPoint pv_point_div_value(PvPoint, double);

typedef struct PvRect{
	double x;
	double y;
	double w;
	double h;
}PvRect;

static const PvRect PvRect_Default = {0, 0, 0, 0,};

PvRect pv_rect_mul_value(PvRect, double);

bool pv_rect_is_inside(PvRect, int x, int y);
/*! @brief expanding PvRect area, logical operation "and" */
PvRect pv_rect_expand(PvRect rect0, PvRect rect1);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_TYPE_H

