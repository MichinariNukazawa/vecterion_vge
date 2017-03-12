/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_CAIRO_H
#define include_PV_CAIRO_H

#include <stdbool.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "pv_type.h"

void pv_cairo_set_source_rgba_workingcolor(cairo_t *);
void pv_cairo_set_source_rgba_subworkingcolor(cairo_t *);
void pv_cairo_fill_checkboard(cairo_t *, PvRect);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_CAIRO_H

