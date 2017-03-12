/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_ET_CLIPBOARD_MANAGER_H
#define include_ET_CLIPBOARD_MANAGER_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "et_doc_id.h"
#include "pv_element.h"

struct EtClipboardManager;
typedef struct EtClipboardManager EtClipboardManager;

EtClipboardManager *et_clipboard_manager_init();

bool et_clipboard_cut_from_doc_id(EtDocId);
bool et_clipboard_copy_from_doc_id(EtDocId);
bool et_clipboard_paste_from_doc_id(EtDocId);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_CLIPBOARD_MANAGER_H

