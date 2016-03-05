#ifndef include_PV_FOCUS_H
#define include_PV_FOCUS_H

#include "pv_element.h"

struct PvFocus;
typedef struct PvFocus PvFocus;
struct PvFocus{
	PvElement *element;
};

PvFocus pv_focus_get_nofocus();

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_FOCUS_H
