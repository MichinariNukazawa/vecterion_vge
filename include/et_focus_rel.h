#ifndef include_ET_FOCUS_REL_H
#define include_ET_FOCUS_REL_H

#include "pv_focus.h"
#include "pv_element.h"
#include "pv_vg.h"


//! @todo EtElementRelation,EtFocusRelation need good name.
struct EtElementRel;
typedef struct EtElementRel EtElementRel;
struct EtElementRel{
	int *indexes;
};

struct EtFocusRel;
typedef struct EtFocusRel EtFocusRel;
struct EtFocusRel{
	EtElementRel **element_rels;
};

EtElementRel *et_element_rel_new(const PvElement *);
void et_element_rel_free(EtElementRel *);
PvElement *et_element_rel_get_element_from_vg(const EtElementRel *, PvVg *);

EtFocusRel *et_focus_rel_new(const PvFocus *);
void et_focus_rel_free(EtFocusRel *);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_FOCUS_REL_H

