#ifndef include_ET_SNAP_H
#define include_ET_SNAP_H

#include <stdbool.h>

typedef struct{
	bool is_snap_for_pixel;
}EtSnap;

static const EtSnap EtSnap_Default = {
	false,
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_SNAP_H
