#ifndef __ET_KEY_ACTION_H__
#define __ET_KEY_ACTION_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_point.h"

typedef enum _EtKeyType{
	EtKey_Unknown,
	EtKey_Right,
	EtKey_Left,
	EtKey_Enter = 65293,
}EtKeyType;

typedef enum _EtKeyActionType{
	EtKeyAction_Unknown,
	EtKeyAction_Down,
	EtKeyAction_Up,
}EtKeyActionType;

struct _EtKeyAction;
typedef struct _EtKeyAction EtKeyAction;
struct _EtKeyAction{
	EtKeyType	key;
	EtKeyActionType	action;
};

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_KEY_ACTION_H__
