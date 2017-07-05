/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_DOCUMENT_PREFERENCE_H
#define include_PV_DOCUMENT_PREFERENCE_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "pv_snap_context.h"

typedef struct{
	PvSnapContext snap_context;
}PvDocumentPreference;

static const PvDocumentPreference PvDocumentPreference_Default = {
	{
		.is_snap_for_grid = false,
		.grid = {50, 50},
	},
};

#ifdef include_PV_TEST
#endif // include_PV_TEST

#endif // include_PV_DOCUMENT_PREFERENCE_H

