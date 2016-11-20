#ifndef include_PV_ANCHOR_POINT_H
#define include_PV_ANCHOR_POINT_H
/*! ******************************
 * @brief 
 ****************************** */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>

#include "pv_type.h"

typedef enum _PvAnchorPointIndex{
	PvAnchorPointIndex_HandlePrev = 0,
	PvAnchorPointIndex_Point = 1,
	PvAnchorPointIndex_HandleNext = 2,
}PvAnchorPointIndex;

struct PvAnchorPoint;
typedef struct PvAnchorPoint PvAnchorPoint;
struct PvAnchorPoint{
	PvPoint points[3];
};
static const PvAnchorPoint PvAnchorPoint_Default = {
	.points = {{0,0}, {0,0}, {0,0}, },
};



/*! @brief handle set to zero.
 *	@param ap_index read to ~_set_handle() function.
 */
void pv_anchor_point_set_handle_zero(
		PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index);
/** @brief set handle from graphic point.
 * @param ap_index PvAnchorPointIndex_Point is next and mirror reverse handle prev.
 */
void pv_anchor_point_set_handle(PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index, PvPoint gpoint);
void pv_anchor_point_set_handle_relate(PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index, PvPoint gpoint);

PvPoint pv_anchor_point_get_point(const PvAnchorPoint *ap);
void pv_anchor_point_set_point(PvAnchorPoint *ap, PvPoint point);
void pv_anchor_point_move_point(PvAnchorPoint *ap, PvPoint move);

/** @brief
 *
 * @return PvPoint to graphic point.
 *		if rise error return value is not specitication.(ex. {0,0})
 */
PvPoint pv_anchor_point_get_handle(const PvAnchorPoint *ap, PvAnchorPointIndex ap_index);
PvPoint pv_anchor_point_get_handle_relate(const PvAnchorPoint *ap, PvAnchorPointIndex ap_index);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ANCHOR_POINT_H

