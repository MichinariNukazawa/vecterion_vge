#include "pv_focus.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"
#include "pv_general.h"
#include "pv_element_info.h"

PvFocus *pv_focus_new(const PvVg *vg)
{
	if(NULL == vg){
		pv_bug("");
		return NULL;
	}

	PvFocus *self = malloc(sizeof(PvFocus));
	if(NULL == self){
		pv_critical("");
		return NULL;
	}

	int num = pv_general_get_parray_num((void **)vg->element_root->childs);
	if(num < 1){
		pv_bug("");
		goto error;
	}
	self->elements = malloc(sizeof(PvElement *) * 2);
	if(NULL == self->elements){
		pv_critical("");
		goto error;
	}
	self->elements[0] = vg->element_root->childs[0];
	self->elements[1] = NULL;

	self->anchor_points = malloc(sizeof(PvAnchorPoint *) * 2);
	pv_assert(self->anchor_points);
	self->anchor_points[1] = NULL;
	self->anchor_points[0] = NULL;


	return self;

error:
	free(self);
	return NULL;
}

bool pv_focus_is_focused(const PvFocus *focus)
{
	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
		switch(focus->elements[0]->kind){
			case PvElementKind_Root:
			case PvElementKind_Layer:
				// NOP
				break;
			default:
				return true;
		}
	}

	if(1 != num){
		pv_bug("");
	}

	return false;
}

bool pv_focus_is_exist_element(const PvFocus *focus, const PvElement *element)
{
	if(NULL == focus){
		pv_bug("");
		return false;
	}
	if(NULL == element){
		pv_bug("");
		return false;
	}

	// check already exist in focus
	int num = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < num; i++){
		if(element == focus->elements[i]){
			return true;
		}
	}

	return false;
}

bool pv_focus_add_element(PvFocus *focus, PvElement *element)
{
	if(NULL == focus){
		pv_bug("");
		return false;
	}
	if(NULL == element){
		pv_bug("");
		return false;
	}

	// check already exist in focus
	if(! pv_focus_remove_element(focus, element)){
		pv_bug("");
		return false;
	}

	// add element.
	int num = pv_general_get_parray_num((void **)focus->elements);
	if(!pv_focus_is_focused(focus)){
		return pv_focus_clear_set_element(focus, element);
	}else{
		PvElement **elements = realloc(focus->elements, sizeof(PvElement *) * (num + 2));
		pv_assert(elements);

		memmove(&(elements[1]), &(elements[0]), sizeof(PvElement *) * num);
		elements[num + 1] = NULL;
		elements[0] = element;

		focus->elements = elements;
	}

	return true;
}

bool pv_focus_remove_element(PvFocus *focus, PvElement *element)
{
	int num = pv_general_get_parray_num((void **)focus->elements);
	if(1 == num && focus->elements[0] == element){
		return pv_focus_clear_to_first_layer(focus);
	}
	for(int i = 0; i < num; i++){
		if(element == focus->elements[i]){
			memmove(&focus->elements[i], &focus->elements[i + 1],
					sizeof(PvElement *) * (num - i));
			focus->elements[num - 1] = NULL;
			return true;
		}
	}

	return true;
}


PvElement *pv_focus_get_first_element(const PvFocus *focus)
{
	if(NULL == focus){
		pv_bug("");
		return NULL;
	}

	int num = pv_general_get_parray_num((void **)focus->elements);
	if(0 == num){
		pv_bug("");
		return NULL;
	}
	return focus->elements[0];
}

static bool pv_focus_is_layer_root_null_from_element_(const PvElement *element)
{
	if(NULL == element){
		return true;
	}
	if(PvElementKind_Root == element->kind){
		return true;
	}
	if(PvElementKind_Layer == element->kind){
		return true;
	}

	return false;
}

PvElement *pv_focus_get_first_layer(const PvFocus *focus)
{
	if(NULL == focus){
		pv_error("");
		return NULL;
	}

	PvElement *element = pv_focus_get_first_element(focus);
	if(NULL == element){
		pv_bug("");
		return NULL;
	}

	while(!pv_focus_is_layer_root_null_from_element_(element)){
		element = element->parent;
	}

	return element;
}

bool pv_focus_clear_set_element_index(PvFocus *focus, PvElement *element, int index)
{
	if(NULL == focus){
		pv_error("");
		return false;
	}
	if(NULL == element){
		pv_error("");
		return false;
	}

	PvElement **new = realloc(focus->elements, sizeof(PvElement *) * 2);
	pv_assert(new);

	new[1] = NULL;
	new[0] = element;
	focus->elements = new;

	if(0 <= index){
		if(PvElementKind_Curve != element->kind){
			pv_error("%d", element->kind);
			return false;
		}

		PvElementCurveData *data = element->data;
		PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, index, PvAnchorPathIndexTurn_Disable);
		pv_assertf(ap, "%d", index);

		PvAnchorPoint **aps = realloc(focus->anchor_points, sizeof(PvAnchorPoint *) * 2);
		aps[1] = NULL;
		aps[0] = ap;
		focus->anchor_points = aps;
	}

	return true;
}

bool pv_focus_clear_set_element(PvFocus *focus, PvElement *element)
{
	return pv_focus_clear_set_element_index(focus, element, -1);
}

bool pv_focus_clear_to_first_layer(PvFocus *focus)
{
	if(NULL == focus){
		pv_bug("");
		return false;
	}

	PvElement *element = pv_focus_get_first_layer(focus);
	if(NULL == element){
		pv_bug("");
		return false;
	}

	return pv_focus_clear_set_element(focus, element);
}

void pv_focus_free(PvFocus *focus)
{
	free(focus->elements);
	free(focus);
}

/*
   bool pv_focus_remove_anchor_point(PvFocus *focus, PvAnchorPoint *ap)
   {
   size_t num = pv_general_get_parray_num((void **)focus->anchor_points);
   for(int i = 0; i < (int)num; i++){
   if(focus->anchor_points[i] == ap){
   memmove(&focus->anchor_points[i], &focus->anchor_points[i + 1],
   sizeof(PvAnchorPoint *) * (num - i - 1));
   focus->anchor_points[num] = NULL;
   return true;
   }
   }

   return false;
   }
 */

PvAnchorPoint *pv_focus_get_first_anchor_point(const PvFocus *focus)
{
	size_t num = pv_general_get_parray_num((void **)focus->anchor_points);
	if(0 == num){
		return NULL;
	}

	size_t num_elements = pv_general_get_parray_num((void **)focus->elements);
	for(int i = 0; i < (int)num_elements; i++){
		const PvElementInfo *info = pv_element_get_info_from_kind(focus->elements[i]->kind);
		pv_assertf(info, "%d", focus->elements[i]->kind);

		if(info->func_is_exist_anchor_point(focus->elements[i], focus->anchor_points[0])){
			return focus->anchor_points[0];
		}
	}

	return NULL;
}

