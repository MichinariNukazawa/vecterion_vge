/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_SNAP_CONTEXT_H
#define include_PV_SNAP_CONTEXT_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_type.h"

typedef struct{
	bool is_snap_for_grid;
	PvPoint grid;
}PvSnapContext;

#ifdef include_PV_TEST
#endif // include_PV_TEST

#endif // include_PV_SNAP_CONTEXT_H

