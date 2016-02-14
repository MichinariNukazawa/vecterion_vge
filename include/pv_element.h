#ifndef __PV_ELEMENT_H__
#define __PV_ELEMENT_H__
/** ******************************
 * @brief PhotonVector Vector Graphics Format.
 *
 ****************************** */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element_general.h"


struct _PvElement;
typedef struct _PvElement PvElement;
struct _PvElement{
	PvElement *parent;
	PvElement **childs; // I know "children".

	PvElementKind kind;
	// kind固有の情報を格納した型のオブジェクト
	gpointer data;
};

/**
 * use pv_element_recursive();
 * @return false: cancel recursive(search childs).
 * false is not error.
 * (this is no tracking "*error" is true.
 *  please use your own "data" struct.)
 */
typedef bool (*PvElementRecursiveFunc)(PvElement *element, gpointer data, int level);
typedef struct _PvElementRecursiveError{
	bool is_error;
	int level;
	PvElement *element;
}PvElementRecursiveError;


PvElement *pv_element_new(const PvElementKind kind);
/** @brief
 * @return false: error from this function.
 */
bool pv_element_recursive(PvElement *element,
		PvElementRecursiveFunc func, gpointer data,
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

const char *pv_element_get_name_from_kind(PvElementKind kind);


bool pv_element_raster_read_file(PvElement * const this,
		const char * const path);
/** @brief 
 *
 * @param this
 *		Not Bezier: return Error.
 */
bool pv_element_bezier_add_anchor_point(PvElement * const this,
		const PvAnchorPoint anchor_point);


#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_ELEMENT_H__
