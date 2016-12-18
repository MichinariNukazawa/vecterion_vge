#include "pv_anchor_path.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"


struct PvAnchorPath{
	bool is_close;
	size_t anchor_points_num;
	PvAnchorPoint *anchor_points;
};


PvAnchorPath *pv_anchor_path_new()
{
	PvAnchorPath *self = malloc(sizeof(PvAnchorPath));
	pv_assert(self);

	self->is_close = false;
	self->anchor_points_num = 0;
	self->anchor_points = NULL;

	return self;
}

void pv_anchor_path_free(PvAnchorPath *self)
{
	if(NULL != self->anchor_points){
		free(self->anchor_points);
	}

	free(self);
}

PvAnchorPath *pv_anchor_path_copy_new(const PvAnchorPath *self)
{
	PvAnchorPath *new_ = pv_anchor_path_new();

	*new_ = *self;

	size_t num = pv_anchor_path_get_anchor_point_num(self);

	if(0 < num){
		size_t size = num * sizeof(PvAnchorPoint);
		new_->anchor_points = malloc(size);
		pv_assert(new_->anchor_points);

		memcpy(new_->anchor_points, self->anchor_points, size);
		new_->anchor_points_num = self->anchor_points_num;
	}

	return new_;
}

void pv_anchor_points_copy_(PvAnchorPoint *dst_aps, const PvAnchorPoint *src_aps, size_t num)
{
	for(int i = 0; i < (int)num; i++){
		dst_aps[i] = src_aps[i];
	}
}

//! @todo need handling "&srd|dst_aps[0]" if "NULL == src|dst_aps" ...?
void pv_anchor_points_copy_range_(
		PvAnchorPoint *dst_aps, int dst_head,
		const PvAnchorPoint *src_aps, int src_head,
		size_t num)
{
	if(NULL == src_aps || NULL == dst_aps){
		pv_assertf(0 == num, "%zu", num);
		return;
	}

	pv_anchor_points_copy_(&dst_aps[dst_head], &src_aps[src_head], num);
}

PvAnchorPath *pv_anchor_path_copy_new_range(const PvAnchorPath *src_anchor_path, int head, int foot)
{
	pv_assert(src_anchor_path);
	size_t src_num = pv_anchor_path_get_anchor_point_num(src_anchor_path);
	pv_assertf((head <= (int)src_num && foot <= (int)src_num), "%zu %d %d", src_num , head, foot);
	int num = foot - head + 1;
	pv_assertf((0 < num), "%zu %d %d", src_num , head, foot);

	PvAnchorPath *dst_anchor_path = pv_anchor_path_new();

	*dst_anchor_path = *src_anchor_path;

	if(0 < num){
		dst_anchor_path->anchor_points = malloc(sizeof(PvAnchorPoint) * num);
		pv_assert(dst_anchor_path->anchor_points);

		pv_anchor_points_copy_(dst_anchor_path->anchor_points, &(src_anchor_path->anchor_points[head]), num);
		dst_anchor_path->anchor_points_num = num;
	}

	return dst_anchor_path;
}

void pv_anchor_path_add_anchor_point(PvAnchorPath *self, const PvAnchorPoint *anchor_point)
{
	PvAnchorPoint *anchor_points = (PvAnchorPoint *)realloc(self->anchor_points,
			sizeof(PvAnchorPoint) * (self->anchor_points_num + 1));
	pv_assert(anchor_points);

	anchor_points[self->anchor_points_num] = *anchor_point;
	self->anchor_points = anchor_points;
	(self->anchor_points_num) += 1;
}

static bool pv_anchor_path_duplicating_anchor_point_(PvAnchorPath *self, int index)
{
	PvAnchorPoint *aps = malloc(sizeof(PvAnchorPoint) * (self->anchor_points_num + 1));
	pv_assert(aps);
	pv_anchor_points_copy_range_(aps, 0, self->anchor_points, 0, index);
	aps[index] = self->anchor_points[index];
	pv_anchor_points_copy_range_(
			aps, index + 1,
			self->anchor_points, index,
			((self->anchor_points_num) - index));

	PvAnchorPoint *old_aps = self->anchor_points;
	self->anchor_points = aps;
	free(old_aps);

	self->anchor_points_num += 1;

	return true;
}

int pv_anchor_path_insert_anchor_point(PvAnchorPath *self, const PvAnchorPoint *ap, int index)
{
	bool ret = pv_anchor_path_duplicating_anchor_point_(self, index);
	pv_assert(ret);

	int new_index = index + 1;
	self->anchor_points[new_index] = *ap;

	return new_index;
}

PvAnchorPoint *pv_anchor_path_get_anchor_point_from_index(PvAnchorPath *self, int index, PvAnchorPathIndexTurn turn)
{
	switch(turn){
		case PvAnchorPathIndexTurn_Disable:
			{
				if(index < 0 || (int)self->anchor_points_num <= index){
					pv_bug("");
					return NULL;
				}else{
					return &(self->anchor_points[index]);
				}
			}
			break;
		case PvAnchorPathIndexTurn_OnlyLastInClosed:
			{
				int index_ = index;
				if(index < 0){
					pv_bug("");
					return NULL;
				}else if((int)self->anchor_points_num == index){
					index_ = 0;
				}else if((int)self->anchor_points_num < index){
					pv_bug("");
					return NULL;
				}
				return &(self->anchor_points[index_]);
			}
			break;
		default:
			pv_assertf(false, "%d", turn);
	}
}

const PvAnchorPoint *pv_anchor_path_get_anchor_point_from_index_const(const PvAnchorPath *self, int index)
{
	return &(self->anchor_points[index]);
}

bool pv_anchor_path_set_anchor_point_from_index(PvAnchorPath *self, int index, const PvAnchorPoint *ap)
{
	PvAnchorPoint *ap_ = pv_anchor_path_get_anchor_point_from_index(self, index, PvAnchorPathIndexTurn_Disable);
	if(NULL == ap_){
		pv_bug("");
		return false;
	}

	*ap_ = *ap;

	return true;
}

size_t pv_anchor_path_get_anchor_point_num(const PvAnchorPath *self)
{
	return self->anchor_points_num;
}

void pv_anchor_path_set_is_close(PvAnchorPath *self, bool is_close)
{
	self->is_close = is_close;
}

bool pv_anchor_path_get_is_close(const PvAnchorPath *self)
{
	return self->is_close;
}

bool pv_anchor_path_is_diff(const PvAnchorPath *anchor_path0, const PvAnchorPath *anchor_path1)
{
	if(anchor_path0->anchor_points_num != anchor_path1->anchor_points_num){
		return true;
	}

	size_t num = pv_anchor_path_get_anchor_point_num(anchor_path0);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap0 = &(anchor_path0->anchor_points[i]);
		const PvAnchorPoint *ap1 = &(anchor_path1->anchor_points[i]);
		if(!(true
					&& ap0->points[0].x == ap1->points[0].x
					&& ap0->points[0].y == ap1->points[0].y
					&& ap0->points[1].x == ap1->points[1].x
					&& ap0->points[1].y == ap1->points[1].y
					&& ap0->points[2].x == ap1->points[2].x
					&& ap0->points[2].y == ap1->points[2].y))
		{
			return true;
		}
	}

	return false;
}

static void pv_anchor_points_change_head_index_(
		PvAnchorPoint *anchor_points,
		size_t anchor_points_num,
		int head)
{
	PvAnchorPoint *aps = malloc(sizeof(PvAnchorPoint) * anchor_points_num);
	pv_assert(aps);

	pv_debug( "%d %zu", head, anchor_points_num);
	pv_assertf((head < (int)anchor_points_num), "%d %zu", head, anchor_points_num);
	for(int i = 0; i < (int)anchor_points_num; i++){
		aps[i] = anchor_points[(head + i) % (int)anchor_points_num];
	}
	for(int i = 0; i < (int)anchor_points_num; i++){
		anchor_points[i] = aps[i];
	}

	free(aps);
}

bool pv_anchor_path_split_anchor_point_from_index(PvAnchorPath *self, int index)
{
	size_t num = pv_anchor_path_get_anchor_point_num(self);
	if(index < 0 || (int)num <= index){
		pv_bug("");
		return false;
	}

	if(!self->is_close){
		pv_bug("");
		return false;
	}

	bool ret = pv_anchor_path_duplicating_anchor_point_(self, index);
	pv_assert(ret);

	pv_anchor_points_change_head_index_(self->anchor_points, self->anchor_points_num, index + 1);
	self->is_close = false;

	return true;
}

bool pv_anchor_path_get_anchor_point_p4_from_index(const PvAnchorPath *self, PvAnchorPointP4 *p4, int index)
{
	const PvAnchorPoint *current_ap = pv_anchor_path_get_anchor_point_from_index_const(self, index);
	pv_assert(current_ap);

	size_t num = pv_anchor_path_get_anchor_point_num(self);
	int next_index = ((index + 1) % (int)num);
	const PvAnchorPoint *next_ap = pv_anchor_path_get_anchor_point_from_index_const(self, next_index);
	pv_assert(next_ap);

	p4->points[0] = pv_anchor_point_get_handle(current_ap, PvAnchorPointIndex_Point);
	p4->points[1] = pv_anchor_point_get_handle(current_ap, PvAnchorPointIndex_HandleNext);
	p4->points[2] = pv_anchor_point_get_handle(next_ap, PvAnchorPointIndex_HandlePrev);
	p4->points[3] = pv_anchor_point_get_handle(next_ap, PvAnchorPointIndex_Point);

	return true;
}

void pv_anchor_path_debug_print(const PvAnchorPath *self)
{
	pv_debug("anchor:%zu(%s)", self->anchor_points_num, (self->is_close)? "true":"false");

	size_t num = pv_anchor_path_get_anchor_point_num(self);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = &self->anchor_points[i];
		pv_debug("%d:% 3.2f,% 3.2f, % 3.2f,% 3.2f, % 3.2f,% 3.2f, ",
				i,
				ap->points[PvAnchorPointIndex_HandlePrev].x,
				ap->points[PvAnchorPointIndex_HandlePrev].y,
				ap->points[PvAnchorPointIndex_Point].x,
				ap->points[PvAnchorPointIndex_Point].y,
				ap->points[PvAnchorPointIndex_HandleNext].x,
				ap->points[PvAnchorPointIndex_HandleNext].y);
	}
}

