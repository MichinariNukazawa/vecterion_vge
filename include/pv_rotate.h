//! @file
#ifndef include_PV_ROTATE_H
#define include_PV_ROTATE_H
#include "pv_type.h"

PvPoint pv_rotate_point(PvPoint src, double degree, PvPoint center);
PvRect pv_rotate_rect(PvRect rect, double degree, PvPoint center);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ROTATE_H
