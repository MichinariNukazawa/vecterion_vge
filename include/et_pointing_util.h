/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_POINTING_UTIL_H
#define include_ET_POINTING_UTIL_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_type.h"
#include "et_mouse_action.h"
#include "et_mouse_util.h"

EtMouseAction et_pointing_util_get_mouse_action(
		PvPoint *pointing_context_previous_mouse_point,
		PvPoint *pointing_context_down_mouse_point,
		PvPoint event_point,
		GdkModifierType event_state,
		int margin,
		double scale,
		EtMouseButtonType mouse_button,
		EtMouseActionType mouse_action);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_POINTING_UTIL_H
