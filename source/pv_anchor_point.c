#include "pv_anchor_point.h"

#include "pv_error.h"


PvAnchorPoint *pv_anchor_point_new_from_point(PvPoint point)
{
	PvAnchorPoint src = pv_anchor_point_from_point(point);

	return pv_anchor_point_copy_new(&src);
}

PvAnchorPoint *pv_anchor_point_copy_new(const PvAnchorPoint *src)
{
	pv_assert(src);

	PvAnchorPoint *dst = malloc(sizeof(PvAnchorPoint));
	pv_assert(dst);

	*dst = *src;

	return dst;
}

void pv_anchor_point_free(PvAnchorPoint *self)
{
	free(self);
}

PvAnchorPoint pv_anchor_point_from_point(PvPoint point)
{
	PvAnchorPoint ap = PvAnchorPoint_Default;
	ap.points[PvAnchorPointIndex_Point] = point;

	return ap;
}

void pv_anchor_point_set_handle_zero(
		PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index)
{
	assert(ap);

	switch(ap_index){
		case PvAnchorPointIndex_Point:
			ap->points[PvAnchorPointIndex_HandlePrev] = (PvPoint){0, 0};
			ap->points[PvAnchorPointIndex_HandleNext] = (PvPoint){0, 0};
			break;
		case PvAnchorPointIndex_HandlePrev:
			ap->points[PvAnchorPointIndex_HandlePrev] = (PvPoint){0, 0};
			break;
		case PvAnchorPointIndex_HandleNext:
			ap->points[PvAnchorPointIndex_HandleNext] = (PvPoint){0, 0};
			break;
		default:
			pv_bug("%d", ap_index);
			return;
	}
}

void pv_anchor_point_set_handle(PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index, PvPoint gpoint)
{
	assert(ap);

	PvPoint p_handle = pv_point_sub(gpoint, ap->points[PvAnchorPointIndex_Point]);

	switch(ap_index){
		case PvAnchorPointIndex_Point:
		{
			PvPoint p_handle_r = pv_point_mul_value(p_handle, -1.0);
			ap->points[PvAnchorPointIndex_HandleNext] = p_handle;
			ap->points[PvAnchorPointIndex_HandlePrev] = p_handle_r;
		}
			break;
		case PvAnchorPointIndex_HandlePrev:
			ap->points[PvAnchorPointIndex_HandlePrev] = p_handle;
			break;
		case PvAnchorPointIndex_HandleNext:
			ap->points[PvAnchorPointIndex_HandleNext] = p_handle;
			break;
		default:
			pv_bug("%d", ap_index);
			return;
	}
}

void pv_anchor_point_set_handle_relate(PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index, PvPoint gpoint)
{
	PvPoint p_handle = gpoint;

	switch(ap_index){
		case PvAnchorPointIndex_Point:
		{
			PvPoint p_handle_r = pv_point_mul_value(p_handle, -1.0);
			ap->points[PvAnchorPointIndex_HandleNext] = p_handle;
			ap->points[PvAnchorPointIndex_HandlePrev] = p_handle_r;
		}
			break;
		case PvAnchorPointIndex_HandlePrev:
			ap->points[PvAnchorPointIndex_HandlePrev] = p_handle;
			break;
		case PvAnchorPointIndex_HandleNext:
			ap->points[PvAnchorPointIndex_HandleNext] = p_handle;
			break;
		default:
			pv_bug("%d", ap_index);
			return;
	}
}

void pv_anchor_point_reverse_handle(PvAnchorPoint *ap)
{
	PvPoint p = ap->points[PvAnchorPointIndex_HandleNext];
	ap->points[PvAnchorPointIndex_HandleNext] = ap->points[PvAnchorPointIndex_HandlePrev];
	ap->points[PvAnchorPointIndex_HandlePrev] = p;
}

PvPoint pv_anchor_point_get_point(const PvAnchorPoint *ap)
{
	return ap->points[PvAnchorPointIndex_Point];
}

void pv_anchor_point_set_point(PvAnchorPoint *ap, PvPoint point)
{
	ap->points[PvAnchorPointIndex_Point] = point;
}

void pv_anchor_point_move_point(PvAnchorPoint *ap, PvPoint move)
{
	ap->points[PvAnchorPointIndex_Point] = pv_point_add(ap->points[PvAnchorPointIndex_Point], move);
}

PvPoint pv_anchor_point_get_handle(const PvAnchorPoint *ap, PvAnchorPointIndex ap_index)
{
	switch(ap_index){
		case PvAnchorPointIndex_HandlePrev:
			{
				PvPoint gp = {
					ap->points[PvAnchorPointIndex_HandlePrev].x
						+ ap->points[PvAnchorPointIndex_Point].x,
					ap->points[PvAnchorPointIndex_HandlePrev].y
						+ ap->points[PvAnchorPointIndex_Point].y};
				return gp;
			}
			break;
		case PvAnchorPointIndex_HandleNext:
			{
				PvPoint gp = {
					ap->points[PvAnchorPointIndex_HandleNext].x
						+ ap->points[PvAnchorPointIndex_Point].x,
					ap->points[PvAnchorPointIndex_HandleNext].y
						+ ap->points[PvAnchorPointIndex_Point].y};
				return gp;
			}
			break;
		case PvAnchorPointIndex_Point:
			return ap->points[PvAnchorPointIndex_Point];
			break;
		default:
			{
				pv_bug("%d", ap_index);
				PvPoint gp = {0,0};
				return gp;
			}
	}
}

PvPoint pv_anchor_point_get_handle_relate(const PvAnchorPoint *ap, PvAnchorPointIndex ap_index)
{
	switch(ap_index){
		case PvAnchorPointIndex_HandlePrev:
		case PvAnchorPointIndex_HandleNext:
		case PvAnchorPointIndex_Point:
			return ap->points[ap_index];
			break;
		default:
			{
				pv_bug("%d", ap_index);
				PvPoint gp = {0,0};
				return gp;
			}
	}
}

PvPoint *pv_anchor_point_get_point_ref(PvAnchorPoint *ap, PvAnchorPointIndex ap_index)
{
	return &(ap->points[ap_index]);
}

void pv_anchor_point_rescale(PvAnchorPoint *ap, PvPoint scale, PvPoint center)
{
	PvPoint pp = pv_anchor_point_get_point(ap);
	pp = pv_point_rescale(pp, scale, center);
	pv_anchor_point_set_point(ap, pp);

	PvPoint hp = pv_anchor_point_get_handle_relate(ap, PvAnchorPointIndex_HandlePrev);
	hp = pv_point_mul(hp, scale);
	pv_anchor_point_set_handle_relate(ap, PvAnchorPointIndex_HandlePrev, hp);

	PvPoint hn = pv_anchor_point_get_handle_relate(ap, PvAnchorPointIndex_HandleNext);
	hn = pv_point_mul(hn, scale);
	pv_anchor_point_set_handle_relate(ap, PvAnchorPointIndex_HandleNext, hn);
}

