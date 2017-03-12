/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_APPEARANCE_INFO_H
#define include_PV_APPEARANCE_INFO_H

#include <stdbool.h>
#include <stddef.h>
#include "pv_type.h"

typedef enum{
	PvAppearanceKind_None,
	PvAppearanceKind_Translate,
	PvAppearanceKind_Resize,
	PvAppearanceKind_Rotate,
}PvAppearanceKind;

typedef struct{
	PvPoint move;
}PvAppearanceTranslateData;

typedef struct{
	PvPoint resize;
}PvAppearanceResizeData;

typedef struct{
	double degree;
}PvAppearanceRotateData;

typedef struct{
	PvAppearanceKind kind;
	union{
		int none; // not use. only fix worning from compiler 'missing initializer for member'
		PvAppearanceTranslateData	translate;
		PvAppearanceResizeData		resize;
		PvAppearanceRotateData		rotate;
	};
}PvAppearance;

static const PvAppearance PvAppearance_Default = {
	.kind = PvAppearanceKind_None,
	{.none = 0,},
};


PvAppearance *pv_appearance_new(PvAppearanceKind);
PvAppearance *pv_appearance_copy_new(const PvAppearance *);
void pv_appearance_free(PvAppearance *);
bool pv_appearance_is_diff(const PvAppearance *, const PvAppearance *);


PvAppearance ** pv_appearance_parray_new_from_num(size_t num);
PvAppearance ** pv_appearance_parray_copy_new(PvAppearance **);
void pv_appearance_parray_free(PvAppearance **);
bool pv_appearance_parray_is_diff(PvAppearance **, PvAppearance **);


#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_APPEARANCE_INFO_H

