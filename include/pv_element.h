#ifndef include_PV_ELEMENT_H
#define include_PV_ELEMENT_H
/** ******************************
 * @brief PhotonVector Vector Graphics Format.
 *
 ****************************** */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element_general.h"
#include "pv_color.h"
#include "pv_stroke.h"



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

/**
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
static const PvElementRecursiveError PvElementRecursiveError_default = {
	.is_error	= false,
	.level		= 0,
	.element	= NULL,
};


PvElement *pv_element_new(const PvElementKind kind);
/** @brief 
 * copy element_tree parent is NULL.
 */
PvElement *pv_element_copy_recursive(const PvElement *self);
/** @brief
 * @return false: error from self function.
 */
bool pv_element_recursive_asc(PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		PvElementRecursiveError *error);

bool pv_element_recursive_desc_before(PvElement *element,
		PvElementRecursiveFunc func, gpointer data,
		PvElementRecursiveError *error);
bool pv_element_recursive_desc(PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		PvElementRecursiveError *error);

/** @brief 
 *
 * @param parent
 *		NULL: return Error.
 *		Not Layer(Group): return Error.
 * @param prev
 *		NULL: append toplevel element in parent.
 *		Element: append under the prev.
 */
bool pv_element_append_child(PvElement * const parent,
		PvElement * const prev, PvElement * const element);
/** @brief
 * element remove in parent->childs
 * delete element and childs recursive.
 */
bool pv_element_remove_delete_recursive(PvElement * const self);

bool pv_element_is_diff_recursive(
		PvElement *element0,
		PvElement *element1);
/*
		const PvElement * const element0,
		const PvElement * const element1);
*/

const char *pv_element_get_name_from_kind(PvElementKind kind);


bool pv_element_raster_read_file(PvElement * const self,
		const char * const path);

/** @brief 
 *
 * @param self
 *		Not Bezier: return Error.
 */
bool pv_element_bezier_add_anchor_point(PvElement * const self,
		const PvAnchorPoint anchor_point);
int pv_element_bezier_get_num_anchor_point(const PvElement *self);

/*! @brief handle set to zero.
 *	@param ap_index read to ~_set_handle() function.
 */
void pv_element_bezier_anchor_point_set_handle_zero(
		PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index);
/** @brief set handle from graphic point.
 * @param ap_index PvAnchorPointIndex_Point is next and mirror reverse handle prev.
 */
void pv_element_bezier_anchor_point_set_handle(PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index, PvPoint gpoint);

/** @brief
 *
 * @return PvPoint to graphic point.
 *		if rise error return value is not specitication.(ex. {0,0})
 */
PvPoint pv_anchor_point_get_handle(const PvAnchorPoint ap, PvAnchorPointIndex ap_index);

bool pv_element_kind_is_viewable_object(PvElementKind kind);

void pv_element_debug_print(const PvElement *element);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_H
