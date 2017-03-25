/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef __PV_IO_H__
#define __PV_IO_H__
/** ******************************
 * @brief PhotonVector Vector Graphics Format.
 *
 ****************************** */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_vg.h"
#include "pv_image_file_read_option.h"

PvVg *pv_io_new_from_file(const char *filepath, const PvImageFileReadOption *imageFileReadOption);
PvElement *pv_io_new_element_from_filepath(const char *filepath);
bool pv_io_write_file_svg_from_vg(PvVg *vg, const char *path);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_IO_H__
