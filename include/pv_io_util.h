/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_IO_UTIL_H
#define include_PV_IO_UTIL_H

#include "pv_color.h"

typedef struct{
	char *key;
	char *value;
}PvStrMap;

double pv_io_util_get_double_from_str(const char *str);
bool pv_io_util_get_pv_color_from_svg_str_rgba(PvColor *ret_color, const char *str);
bool pv_read_args_from_str(double *args, int num_args, const char **str);
void pv_double_array_fill(double *dst, double value, int size);
void pv_str_maps_free(PvStrMap *);

#ifdef include_PV_TEST
#endif // include_PV_TEST

#endif // include_PV_IO_UTIL_H

