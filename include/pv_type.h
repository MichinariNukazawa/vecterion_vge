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

typedef struct PvRect{
	double x;
	double y;
	double w;
	double h;
}PvRect;

bool pv_rect_is_inside(PvRect, int x, int y);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_TYPE_H

