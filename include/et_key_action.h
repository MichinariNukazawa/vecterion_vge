#ifndef include_ET_KEY_ACTION_H
#define include_ET_KEY_ACTION_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

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

struct EtKeyAction;
typedef struct EtKeyAction EtKeyAction;
struct EtKeyAction{
	EtKeyType	key;
	EtKeyActionType	action;
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_KEY_ACTION_H
