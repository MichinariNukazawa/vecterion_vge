#include "pv_appearance.h"

#include <stdlib.h>
#include "pv_error.h"
#include "pv_general.h"



PvAppearance *pv_appearance_new(PvAppearanceKind kind)
{
	PvAppearance *self = malloc(sizeof(PvAppearance));
	pv_assert(self);

	*self = PvAppearance_Default;

	self->kind = kind;

	return self;
}

PvAppearance *pv_appearance_copy_new(const PvAppearance *src)
{
	PvAppearance *dst = pv_appearance_new(src->kind);
	pv_assert(dst);

	*dst = *src;

	return dst;
}

void pv_appearance_free(PvAppearance *self)
{
	pv_assert(self);

	free(self);
}

bool pv_appearance_is_diff(const PvAppearance *appearance0, const PvAppearance *appearance1)
{
	if(appearance0->kind != appearance1->kind){
		return true;
	}

	switch(appearance0->kind){
		case PvAppearanceKind_None:
			return false;
			break;

		case PvAppearanceKind_Translate:
			{
				const PvAppearanceTranslateData *translate0 = &(appearance0->translate);
				const PvAppearanceTranslateData *translate1 = &(appearance1->translate);
				if(pv_point_is_diff(translate0->move, translate1->move)){
					return true;
				}

				return false;
			}
			break;

		case PvAppearanceKind_Resize:
			{
				const PvAppearanceResizeData *resize0 = &(appearance0->resize);
				const PvAppearanceResizeData *resize1 = &(appearance1->resize);
				if(pv_point_is_diff(resize0->resize, resize1->resize)){
					return true;
				}

				return false;
			}
			break;
		case PvAppearanceKind_Rotate:
			{
				const PvAppearanceRotateData *rotate0 = &(appearance0->rotate);
				const PvAppearanceRotateData *rotate1 = &(appearance1->rotate);
				if(rotate0->degree != rotate1->degree){
					return true;
				}

				return false;
			}
			break;
		default:
			pv_assertf(false, "%d", appearance0->kind);
	}
}


PvAppearance ** pv_appearance_parray_new_from_num(size_t num)
{
	if(0 == num){
		// is normal.
		return NULL;
	}

	PvAppearance **parray = malloc(sizeof(PvAppearance *) * (num + 1));
	pv_assert(parray);
	parray[0] = NULL; // default guard of parray

	for(int i = 0; i < (int)num; i++){
		PvAppearance *appearance = pv_appearance_new(PvAppearanceKind_None);
		pv_assert(appearance);
		parray[i + 1] = NULL;
		parray[i] = appearance;
	}

	return parray;
}

PvAppearance ** pv_appearance_parray_copy_new(PvAppearance **src_parray)
{
	if(NULL == src_parray){
		// is normal
		return NULL;
	}

	size_t num = pv_general_get_parray_num((void **)src_parray);
	PvAppearance **dst_parray = pv_appearance_parray_new_from_num(num);
	pv_assert(dst_parray);

	for(int i = 0; i < (int)num; i++){
		*dst_parray[i] = *src_parray[i];
	}

	return dst_parray;
}

void pv_appearance_parray_free(PvAppearance **parray)
{
	size_t num = pv_general_get_parray_num((void **)parray);
	for(int i = 0; i < (int)num; i++){
		pv_appearance_free(parray[i]);
	}

	if(NULL != parray){
		free(parray);
	}
}

bool pv_appearance_parray_is_diff(PvAppearance **parray0, PvAppearance **parray1)
{
	//! @todo appearances logical difference

	int num0 = pv_general_get_parray_num((void **)parray0);
	int num1 = pv_general_get_parray_num((void **)parray1);

	if(num0 != num1){
		return true;
	}

	for(int i = 0; i < num0; i++){
		if(pv_appearance_is_diff(parray0[i], parray1[i])){
			return true;
		}
	}

	return false;
}

