#include "pv_focus.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"
#include "pv_general.h"

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

	self->index = -1;

	return self;

error:
	free(self);
	return NULL;
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
	if(0 < num && PvElementKind_Layer == focus->elements[0]->kind){
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
		return pv_focus_clear_to_parent_layer(focus);
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

bool pv_focus_is_layer_root_null_from_element(const PvElement *element)
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

PvElement *pv_focus_get_first_element_parent_layer(const PvFocus *focus)
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

	while(!pv_focus_is_layer_root_null_from_element(element)){
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
	if(NULL == focus->elements){
		pv_critical("");
		return false;
	}

	new[0] = element;
	new[1] = NULL;
	focus->elements = new;
	focus->index = index;

	return true;
}

bool pv_focus_clear_set_element(PvFocus *focus, PvElement *element)
{
	return pv_focus_clear_set_element_index(focus, element, -1);
}

bool pv_focus_clear_to_parent_layer(PvFocus *focus)
{
	if(NULL == focus){
		pv_bug("");
		return false;
	}

	PvElement *element = pv_focus_get_first_element_parent_layer(focus);
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

