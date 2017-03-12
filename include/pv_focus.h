/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_FOCUS_H
#define include_PV_FOCUS_H

#include "pv_element.h"
#include "pv_anchor_point.h"
#include "pv_vg.h"

struct PvFocus;
typedef struct PvFocus PvFocus;
struct PvFocus{
	PvElement **elements;
	PvAnchorPoint **anchor_points;
};

PvFocus *pv_focus_new(const PvVg *vg);
bool pv_focus_is_focused(const PvFocus *);
bool pv_focus_is_exist_element(const PvFocus *, const PvElement *);
bool pv_focus_is_exist_anchor_point(const PvFocus *, const PvElement *, const PvAnchorPoint *);
bool pv_focus_add_element(PvFocus *, PvElement *);
bool pv_focus_add_anchor_point(PvFocus *, PvElement *, PvAnchorPoint *);
bool pv_focus_remove_element(PvFocus *, PvElement *);
bool pv_focus_remove_anchor_point(PvFocus *, PvElement *, PvAnchorPoint *);
PvElement *pv_focus_get_first_element(const PvFocus *);
PvAnchorPoint *pv_focus_get_first_anchor_point(const PvFocus *);
PvElement *pv_focus_get_first_layer(const PvFocus *);
bool pv_focus_clear_set_element(PvFocus *, PvElement *);
bool pv_focus_clear_set_element_index(PvFocus *, PvElement *, int index);
bool pv_focus_clear_set_anchor_point(PvFocus *, PvElement *, PvAnchorPoint *);
bool pv_focus_clear_to_first_layer(PvFocus *);
void pv_focus_free(PvFocus *);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_FOCUS_H

