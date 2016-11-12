#include "et_focus_rel.h"

#include <stdlib.h>
#include <string.h>
#include "et_error.h"

static int _get_index_from_element(const PvElement *element);
static int *_get_indexes_from_element(const PvElement *element);
static PvElement *_get_element_from_indexes(PvElement *element_root, const int *indexes);

EtElementRel *et_element_rel_new(const PvElement *element)
{
	EtElementRel *self = malloc(sizeof(EtElementRel));
	et_assert(self);

	self->indexes = _get_indexes_from_element(element);
	et_assert(self->indexes);

	return self;
}

void et_element_rel_free(EtElementRel *self)
{
	free(self->indexes);
	free(self);
}

PvElement *et_element_rel_get_element_from_vg(const EtElementRel *element_rel, PvVg *vg)
{
	return _get_element_from_indexes(vg->element_root, element_rel->indexes);
}

EtFocusRel *et_focus_rel_new(const PvFocus *focus)
{
	int num = pv_general_get_parray_num((void **)focus->elements);

	EtFocusRel *self = malloc(sizeof(EtFocusRel));
	et_assert(self);

	self->element_rels = malloc(sizeof(EtElementRel *) * (num + 1));
	et_assert(self->element_rels);

	for(int i = 0; i < (int)num; i++){
		if(PvElementKind_Root == focus->elements[i]->kind){
			et_warning("%d/%d", i, num);
		}
		self->element_rels[i] = et_element_rel_new(focus->elements[i]);
		et_assert(self->element_rels[i]);
	}
	self->element_rels[num] = NULL;

	return self;
}

void et_focus_rel_free(EtFocusRel *self)
{
	int num = pv_general_get_parray_num((void **)self->element_rels);
	for(int i = 0; i < (int)num; i++){
		et_element_rel_free(self->element_rels[i]);
	}

	free(self->element_rels);
	free(self);
}


static int _get_index_from_element(const PvElement *element)
{
	PvElement *parent = element->parent;
	if(NULL == parent){
		et_error("");
		return -1;
	}
	for(int i = 0; NULL != parent->childs[i]; i++){
		if(element == parent->childs[i]){
			return i;
		}
	}

	return -1;
}

static int *_get_indexes_from_element(const PvElement *element)
{
	if(NULL == element){
		et_bug("");
		return NULL;
	}

	if(NULL == element->parent){
		int *indexes = malloc(sizeof(int) * (0 + 1));
		indexes[0] = -1;
		return indexes;
	}

	int level = 0;
	int *indexes = NULL;
	const PvElement *_element = element;
	while(NULL != _element->parent){
		int ix = _get_index_from_element(_element);
		if(ix < 0){
			et_error("");
			goto error;
		}
		int *new = realloc(indexes, sizeof(int) * (level + 2));
		et_assert(new);
		memmove(&new[1], &new[0], sizeof(int) * (level + 1));
		indexes = new;

		indexes[level + 1] = -1;
		indexes[0] = ix;
		level++;

		_element = _element->parent;
	}

	return indexes;
error:
	free(indexes);
	return NULL;
}

static PvElement *_get_element_from_indexes(PvElement *element_root, const int *indexes)
{

	if(NULL == indexes){
		et_bug("");
		return NULL;
	}

	PvElement *_element = element_root;
	for(int level = 0; 0 <= indexes[level]; level++){
		int num = pv_general_get_parray_num((void **)_element->childs);
		if(!(indexes[level] < num)){
			et_error("level:%d ix:%d num:%d", level, indexes[level], num);
			return NULL;
		}

		_element = _element->childs[indexes[level]];
	}

	return _element;
}

