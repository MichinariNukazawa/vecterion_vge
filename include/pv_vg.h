#ifndef include_PV_VG_H
#define include_PV_VG_H
/** ******************************
 * @brief PhotonVector Vector Graphics Format.
 *
 ****************************** */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element.h"

struct PvVg;
typedef struct PvVg PvVg;
struct PvVg{
	PvRect rect;
	PvElement *element_root;
};

/** @brief pointer arrayの内容数を返す
 * (実長さは番兵のNULL終端があるため、return+1)
 */
int pv_general_get_parray_num(void **pointers);

PvVg *pv_vg_new();
void pv_vg_free(PvVg *vg);
/** @brief 
 * @return vg->element_root->childs[0];
 */
PvElement *pv_vg_get_layer_top(const PvVg *vg);
bool pv_vg_copy_overwrite(PvVg *dst, const PvVg *src);
PvVg *pv_vg_copy_new(const PvVg *src);

/*
 * @return is exist difference (internal error is not returned.)
 */
bool pv_vg_is_diff(const PvVg *vg0, const PvVg *vg1);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_VG_H
