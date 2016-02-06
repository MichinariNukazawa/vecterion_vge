#ifndef __ET_MOUSE_ACTION_H__
#define __ET_MOUSE_ACTION_H__

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

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_MOUSE_ACTION_H__
