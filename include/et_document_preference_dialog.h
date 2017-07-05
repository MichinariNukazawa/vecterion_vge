/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_DOCUMENT_PREFERENCE_DIALOG_H
#define include_ET_DOCUMENT_PREFERENCE_DIALOG_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_document_preference.h"

GtkBuilder *pv_document_preference_dialog_init(const char *application_base_dir);
/** @return result code (ex.GTK_RESPONSE_ACCEPT) */
int pv_document_preference_dialog_run(GtkBuilder *);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOCUMENT_PREFERENCE_DIALOG_H

