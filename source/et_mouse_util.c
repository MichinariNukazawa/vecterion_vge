#include "et_mouse_util.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "et_error.h"

void et_mouse_util_modifier_kind(int state)
{
	if(state & GDK_SHIFT_MASK){
		et_debug("Shift");
	}
	if(state & GDK_CONTROL_MASK){
		et_debug("CONTROL");
	}
	if(state & GDK_MOD1_MASK){
		et_debug("Alt");
	}
}

void et_mouse_util_button_kind(int button)
{
	switch(button){
		case 1:
			et_debug("LEFT");
			break;
		case 2:
			et_debug("CENTER");
			break;
		case 3:
			et_debug("RIGHT");
			break;
		default:
			et_debug("UNKNOWN:%d", button);
	}
}
