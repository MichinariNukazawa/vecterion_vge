#ifndef include_ET_KEY_ACTION_H
#define include_ET_KEY_ACTION_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

typedef enum Enum_EtKeyType{
	EtKeyType_Enter = 65293,
}Enum_EtKeyType;

typedef unsigned int EtKeyType;

typedef enum _EtKeyActionType{
	EtKeyAction_Unknown,
	EtKeyAction_Down,
	EtKeyAction_Up,
}EtKeyActionType;

typedef unsigned int EtKeyState;

struct EtKeyAction;
typedef struct EtKeyAction EtKeyAction;
struct EtKeyAction{
	EtKeyType	key;
	EtKeyActionType	action;
	EtKeyState	state;
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_KEY_ACTION_H
