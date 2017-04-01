/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
// ******** ********
//! @file
// ******** ********
#ifndef include_PV_ELEMENT_H
#define include_PV_ELEMENT_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element_general.h"
#include "pv_color.h"
#include "pv_stroke.h"
#include "pv_appearance.h"



// ******** ********
// PvElement
// ******** ********
#define NUM_WORK_APPEARANCE (3)
struct PvElement;
typedef struct PvElement PvElement;
struct PvElement{
	PvElement *parent;
	PvElement **childs; // I know "children".

	PvColorPair color_pair;
	PvStroke stroke;

	PvElementKind kind;
	// kind固有の情報を格納した型のオブジェクト
	gpointer data;

	PvAppearance **etaion_work_appearances;
};

/*!
 * use pv_element_recursive_desc_before();
 * @return false: cancel recursive(search childs).
 * false is not error.
 * (this is no tracking "*error" is true.
 *  please use your own "data" struct.)
 */
typedef bool (*PvElementRecursiveFunc)(PvElement *element, gpointer data, int level);
typedef struct PvElementRecursiveError{
	bool is_error;
	int level;
	const PvElement *element;
}PvElementRecursiveError;
static const PvElementRecursiveError PvElementRecursiveError_Default = {
	.is_error	= false,
	.level		= 0,
	.element	= NULL,
};


PvElement *pv_element_new(const PvElementKind);
void pv_element_free(PvElement *);

PvElement *pv_element_copy_recursive(const PvElement *);
bool pv_element_remove_free_recursive(PvElement *);
bool pv_element_remove(PvElement *);

void pv_element_copy_property(PvElement *dst, PvElement *src);

bool pv_element_append_child(PvElement *parent, const PvElement *prev, PvElement *element);
bool pv_element_append_nth(PvElement *parent, const int nth, PvElement *element);
bool pv_element_append_on_focusing(PvElement *focusing_element, PvElement *element);

PvElement *pv_element_get_first_parent_layer_or_root(PvElement *);

PvElement *pv_element_get_in_elements_from_member_anchor_point(PvElement **elements, const PvAnchorPoint *);

/*! @brief
 * @return false: error from self function.
 */
bool pv_element_recursive_asc(
		PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		PvElementRecursiveError *error);
bool pv_element_recursive_desc(
		PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		PvElementRecursiveError *error);
bool pv_element_recursive_desc_before(
		PvElement *element,
		PvElementRecursiveFunc func, gpointer data,
		PvElementRecursiveError *error);

bool pv_element_is_diff_recursive(PvElement *element0, PvElement *element1);

const char *pv_element_get_name_from_kind(PvElementKind kind);
bool pv_element_kind_is_viewable_object(PvElementKind kind);
bool pv_element_kind_is_object(PvElementKind kind);



// ******** ********
// PvElement Curve
// ******** ********
PvElement *pv_element_curve_new_from_rect(PvRect);
PvElement *pv_element_curve_new_set_anchor_path(PvAnchorPath *);
PvAnchorPath *pv_element_curve_get_anchor_path(PvElement *);
PvElement *pv_element_curve_new_set_anchor_point(PvAnchorPoint *);
void pv_element_curve_append_anchor_point(PvElement *, PvAnchorPoint *, int index);
bool pv_element_curve_add_anchor_point(PvElement *, const PvAnchorPoint);
int pv_element_curve_get_num_anchor_point(const PvElement *);
void pv_element_curve_set_close_anchor_point(PvElement *, bool);



// ******** ********
// PvElement BasicShape
// ******** ********
PvElement *pv_element_basic_shape_new_from_filepath(const char *filepath);



// ******** ********
// Debug
// ******** ********
void pv_element_debug_print(const PvElement *);



#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_H

