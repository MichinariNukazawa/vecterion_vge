#include "pv_focus.h"

PvFocus pv_focus_get_nofocus()
{
	PvFocus focus = {
		.element = NULL,
	};

	return focus;
}
