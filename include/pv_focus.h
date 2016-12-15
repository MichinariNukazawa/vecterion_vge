#ifndef include_PV_FOCUS_H
#define include_PV_FOCUS_H

#include "pv_element.h"
#include "pv_vg.h"

struct PvFocus;
typedef struct PvFocus PvFocus;
struct PvFocus{
	PvElement **elements;
	int index;
};

PvFocus *pv_focus_new(const PvVg *vg);
bool pv_focus_is_focused(const PvFocus *);
bool pv_focus_is_exist_element(const PvFocus *focus, const PvElement *element);
bool pv_focus_add_element(PvFocus *focus, PvElement *element);
bool pv_focus_remove_element(PvFocus *focus, PvElement *element);
// void pv_focus_detouch_element(PvFocus *focus, const PvElement *element);
PvElement *pv_focus_get_first_element(const PvFocus *focus);
PvElement *pv_focus_get_first_layer(const PvFocus *focus);
int pv_focus_get_index(const PvFocus *focus);
bool pv_focus_clear_set_element(PvFocus *focus, PvElement *element);
bool pv_focus_clear_set_element_index(PvFocus *focus, PvElement *element, int index);
bool pv_focus_clear_to_first_layer(PvFocus *focus);
void pv_focus_free(PvFocus *focus);

bool pv_focus_is_layer_root_null_from_element(const PvElement *element);
// PvFocus pv_focus_get_nofocus();

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_FOCUS_H
