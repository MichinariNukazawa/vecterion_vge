#ifndef include_ET_MOUSE_ACTION_H
#define include_ET_MOUSE_ACTION_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_point.h"

typedef enum _EtMouseButtonType{
	EtMouseButton_Unknown,
	EtMouseButton_Right,
	EtMouseButton_Left,
	EtMouseButton_Center,
}EtMouseButtonType;

typedef enum _EtMouseActionType{
	EtMouseAction_Unknown,
	EtMouseAction_Down,
	EtMouseAction_Move,
	EtMouseAction_Up,
}EtMouseActionType;

struct _EtMouseAction;
typedef struct _EtMouseAction EtMouseAction;
struct _EtMouseAction{
	EtMouseButtonType	button;
	EtMouseActionType	action;
	EtPoint			point;
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_MOUSE_ACTION_H
