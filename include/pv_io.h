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

bool pv_io_write_file_svg_from_vg(PvVg *vg, const char *path);
// PvVg *pv_io_new_vg_from_file(const char *path);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_IO_H__
