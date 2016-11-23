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



// ******** ********
// PvElement
// ******** ********
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
void pv_element_delete(PvElement *);

PvElement *pv_element_copy_recursive(const PvElement *);
/*! @brief
 * element remove in parent->childs
 * delete element and childs recursive.
 */
bool pv_element_remove_delete_recursive(PvElement *);


bool pv_element_append_child(PvElement *parent, const PvElement *prev, PvElement *element);

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



// ******** ********
// PvElement Bezier
// ******** ********
bool pv_element_bezier_add_anchor_point(PvElement *, const PvAnchorPoint);
int pv_element_bezier_get_num_anchor_point(const PvElement *);



// ******** ********
// PvElement Raster
// ******** ********
PvElement *pv_element_raster_new_from_filepath(const char *filepath);



// ******** ********
// Debug
// ******** ********
void pv_element_debug_print(const PvElement *);



#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_H

