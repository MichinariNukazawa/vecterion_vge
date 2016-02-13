#ifndef __PV_FOCUS_H__
#define __PV_FOCUS_H__

#include "pv_element.h"

struct _PvFocus;
typedef struct _PvFocus PvFocus;
struct _PvFocus{
	PvElement *element;
};

PvFocus pv_focus_get_nofocus();

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_FOCUS_H__
