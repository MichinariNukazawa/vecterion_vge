/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_MOUSE_ACTION_H
#define include_ET_MOUSE_ACTION_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_type.h"

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

#define MOUSE_BUTTON_LEFT_MASK (GDK_BUTTON1_MASK)

struct EtMouseAction;
typedef struct EtMouseAction EtMouseAction;
struct EtMouseAction{
	EtMouseButtonType	button;
	EtMouseActionType	action;
	PvPoint			point;	//<! position
	// ** used in moving
	PvPoint			move;		// mouse move difference previous.
	PvPoint			diff_down;	// mouse move difference mouse down.
	GdkModifierType		state;		// mouse buttons state.

	double			scale;
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_MOUSE_ACTION_H
