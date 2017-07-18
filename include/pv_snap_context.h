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

#define PV_SNAP_CONTEXT_MAX_SNAP_FOR_DEGREES (6)

typedef struct{
	bool is_snap_for_grid;
	PvPoint grid;
	bool is_snap_for_degree;
	size_t num_snap_for_degree;
	double degrees[PV_SNAP_CONTEXT_MAX_SNAP_FOR_DEGREES];
}PvSnapContext;

#ifdef include_PV_TEST
#endif // include_PV_TEST

#endif // include_PV_SNAP_CONTEXT_H

