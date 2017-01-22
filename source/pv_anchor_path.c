#include "pv_anchor_path.h"

#include <stdlib.h>
#include <string.h>
#include "pv_general.h"
#include "pv_error.h"


struct PvAnchorPath{
	bool is_close;
	PvAnchorPoint **anchor_points;
};

void pv_anchor_points_free_(PvAnchorPoint **anchor_points)
{
	size_t num = pv_general_get_parray_num((void **)anchor_points);
	for(int i = 0; i < (int)num; i++){
		pv_anchor_point_free(anchor_points[i]);
	}

	free(anchor_points);
}
void pv_anchor_path_realloc_anchor_points_(PvAnchorPath *anchor_path)
{
	size_t num = pv_anchor_path_get_anchor_point_num(anchor_path);
	if(0 == num){
		pv_anchor_points_free_(anchor_path->anchor_points);
		anchor_path->anchor_points = NULL;
	}else{
		PvAnchorPoint **anchor_points = realloc(anchor_path->anchor_points, sizeof(PvAnchorPoint *) * (num + 1));
		pv_assert(anchor_points);
		anchor_path->anchor_points = anchor_points;
	}

	return;
}

PvAnchorPath *pv_anchor_path_new()
{
	PvAnchorPath *self = malloc(sizeof(PvAnchorPath));
	pv_assert(self);

	self->is_close = false;
	self->anchor_points = NULL;

	return self;
}

void pv_anchor_path_free(PvAnchorPath *self)
{
	pv_anchor_points_free_(self->anchor_points);

	free(self);
}

static PvAnchorPath *pv_anchor_path_copy_new_range(const PvAnchorPath *src, int head, int foot)
{
	pv_assert(src);
	size_t src_num = pv_anchor_path_get_anchor_point_num(src);
	pv_assertf((head <= (int)src_num && foot <= (int)src_num), "%zu %d %d", src_num , head, foot);
	int num = foot - head + 1;
	pv_assertf((0 <= num), "%zu %d %d", src_num , head, foot);

	PvAnchorPath *dst = pv_anchor_path_new();
	pv_assert(dst);

	*dst = *src;

	dst->anchor_points = (PvAnchorPoint **)malloc(sizeof(PvAnchorPoint *) * (num + 1));
	pv_assertf(dst->anchor_points, "%d", num);
	for(int i = 0; i < (int)num; i++){
		dst->anchor_points[i] = pv_anchor_point_copy_new(src->anchor_points[head + i]);
		pv_assert(dst->anchor_points[i]);
	}
	dst->anchor_points[num] = NULL;

	return dst;
}

PvAnchorPath *pv_anchor_path_copy_new(const PvAnchorPath *src_anchor_path)
{
	size_t num = pv_anchor_path_get_anchor_point_num(src_anchor_path);
	int num_ = (int)num - 1;
	PvAnchorPath *dst_anchor_path = pv_anchor_path_copy_new_range(src_anchor_path, 0, num_);
	if(NULL == dst_anchor_path){
		return NULL;
	}

	dst_anchor_path->is_close = src_anchor_path->is_close;

	return dst_anchor_path;
}

PvAnchorPath *pv_anchor_path_split_new_from_index(PvAnchorPath *src_anchor_path, int index)
{
	size_t num = pv_anchor_path_get_anchor_point_num(src_anchor_path);
	pv_assertf(0 <= index, "%d %zu", index, num);
	pv_assertf(index < (int)num, "%d %zu", index, num);

	int dst_num = (int)num - index;
	PvAnchorPoint **dst_anchor_points = malloc(sizeof(PvAnchorPoint *) * (dst_num + 1));
	pv_assert(dst_anchor_points);
	for(int i = 0; i < dst_num; i++){
		dst_anchor_points[i + 0] = src_anchor_path->anchor_points[index + i];
		dst_anchor_points[i + 1] = NULL;
	}

	src_anchor_path->anchor_points[index] = NULL;
	pv_anchor_path_realloc_anchor_points_(src_anchor_path);

	PvAnchorPath *dst_anchor_path = pv_anchor_path_new();
	pv_assert(dst_anchor_path);
	free(dst_anchor_path->anchor_points);
	dst_anchor_path->anchor_points = dst_anchor_points;

	const PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index_const(dst_anchor_path, 0);
	pv_assert(ap);
	pv_anchor_path_add_anchor_point(src_anchor_path, ap);

	src_anchor_path->is_close = false;
	dst_anchor_path->is_close = false;

	return dst_anchor_path;
}

PvAnchorPath *pv_anchor_path_split_new_from_index_remove_delete(PvAnchorPath *src_anchor_path, int index)
{
	size_t num = pv_anchor_path_get_anchor_point_num(src_anchor_path);
	pv_assertf(0 <= index, "%d %zu", index, num);
	pv_assertf(index < (int)num, "%d %zu", index, num);

	src_anchor_path->is_close = false;

	PvAnchorPath *dst_anchor_path = NULL;
	if(0 == index){
		pv_anchor_path_remove_delete_range(src_anchor_path, 0, 0);
	}else if(index == ((int)num - 1)){
		int src_foot = (int)num - 1;
		pv_anchor_path_remove_delete_range(src_anchor_path, src_foot, src_foot);
		//! need check if anchor_points zero to delete element by parent caller function.
	}else{
		dst_anchor_path = pv_anchor_path_split_new_from_index(src_anchor_path, index);
		int src_foot = pv_anchor_path_get_anchor_point_num(src_anchor_path) - 1;
		pv_anchor_path_remove_delete_range(src_anchor_path, src_foot, src_foot);
		pv_anchor_path_remove_delete_range(dst_anchor_path, 0, 0);
	}

	return dst_anchor_path;
}

void pv_anchor_path_add_anchor_point(PvAnchorPath *self, const PvAnchorPoint *anchor_point)
{
	size_t num = pv_anchor_path_get_anchor_point_num(self);
	PvAnchorPoint **anchor_points = (PvAnchorPoint **)realloc(self->anchor_points,
			sizeof(PvAnchorPoint *) * (num + 2));
	pv_assert(anchor_points);

	PvAnchorPoint *ap = pv_anchor_point_copy_new(anchor_point);
	pv_assert(ap);
	anchor_points[num + 1] = NULL;
	anchor_points[num + 0] = ap;
	self->anchor_points = anchor_points;
}

static PvAnchorPoint *pv_anchor_path_duplicating_anchor_point_(PvAnchorPath *self, int index)
{
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			self,
			index,
			PvAnchorPathIndexTurn_Disable);
	pv_assertf(ap, "%d", index);

	return pv_anchor_path_insert_anchor_point(self, ap, index);
}

PvAnchorPoint *pv_anchor_path_insert_anchor_point(PvAnchorPath *self, const PvAnchorPoint *ap, int index)
{
	PvAnchorPoint *ap_ = pv_anchor_point_copy_new(ap);
	pv_assert(ap_);

	size_t num = pv_anchor_path_get_anchor_point_num(self);
	PvAnchorPoint **aps = realloc(self->anchor_points, sizeof(PvAnchorPoint *) * (num + 2));
	pv_assert(aps);

	// memmove(&(aps[0]), &(aps[0]), sizeof(PvElement *) * index);
	memmove(&(aps[index + 1]), &(aps[index + 0]), sizeof(PvAnchorPoint *) * ((int)num - index));
	aps[index + 1] = ap_;
	aps[num + 1] = NULL;

	self->anchor_points = aps;

	return ap_;
}

bool pv_anchor_path_remove_delete_range(PvAnchorPath *anchor_path, int head, int foot)
{
	pv_assertf(0 <= head, "%d", head);

	size_t num = pv_anchor_path_get_anchor_point_num(anchor_path);
	if(-1 == foot){
		foot = num;
	}

	for(int i = foot; head <= i; i--){
		pv_anchor_point_free(anchor_path->anchor_points[i]);
	}

	memmove(
			&anchor_path->anchor_points[head],
			&anchor_path->anchor_points[foot + 1],
			sizeof(PvAnchorPoint *) * (num - foot));
	anchor_path->anchor_points[head + (num - foot)] = NULL;

	size_t num_aft = pv_anchor_path_get_anchor_point_num(anchor_path);
	PvAnchorPoint **aps = realloc(anchor_path->anchor_points, sizeof(PvAnchorPoint *) * (num_aft + 1));
	pv_assert(aps);
	anchor_path->anchor_points = aps;

	return true;
}

bool pv_anchor_path_remove_delete_anchor_point(PvAnchorPath *anchor_path, const PvAnchorPoint *anchor_point)
{
	int index = pv_anchor_path_get_index_from_anchor_point(anchor_path, anchor_point);
	if(index < 0){
		return false;
	}

	bool ret = pv_anchor_path_remove_delete_range(anchor_path, index, index);

	return ret;
}

PvAnchorPoint *pv_anchor_path_get_anchor_point_from_index(PvAnchorPath *self, int index, PvAnchorPathIndexTurn turn)
{
	size_t num = pv_anchor_path_get_anchor_point_num(self);

	switch(turn){
		case PvAnchorPathIndexTurn_Disable:
			{
				if(index < 0 || (int)num <= index){
					pv_bug("");
					return NULL;
				}else{
					return self->anchor_points[index];
				}
			}
			break;
		case PvAnchorPathIndexTurn_OnlyLastInClosed:
			{
				int index_ = index;
				if(index < 0){
					pv_bug("");
					return NULL;
				}else if((int)num == index){
					index_ = 0;
				}else if((int)num < index){
					pv_bug("");
					return NULL;
				}
				return self->anchor_points[index_];
			}
			break;
		default:
			pv_assertf(false, "%d", turn);
	}
}

const PvAnchorPoint *pv_anchor_path_get_anchor_point_from_index_const(const PvAnchorPath *self, int index)
{
	return self->anchor_points[index];
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
	size_t num = pv_general_get_parray_num((void **)self->anchor_points);
	return num;
}

int pv_anchor_path_get_index_from_anchor_point(const PvAnchorPath *anchor_path, const PvAnchorPoint *ap)
{
	size_t num = pv_general_get_parray_num((void **)anchor_path->anchor_points);
	for(int i = 0; i < (int)num; i++){
		if(ap == anchor_path->anchor_points[i]){
			return i;
		}
	}

	return -1;
}

bool pv_anchor_path_is_exist_anchor_point(const PvAnchorPath *anchor_path, const PvAnchorPoint *ap)
{
	int index = pv_anchor_path_get_index_from_anchor_point(anchor_path, ap);

	return (-1 != index);
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
	size_t num0 = pv_anchor_path_get_anchor_point_num(anchor_path0);
	size_t num1 = pv_anchor_path_get_anchor_point_num(anchor_path1);
	if(num0 != num1){
		return true;
	}

	for(int i = 0; i < (int)num0; i++){
		const PvAnchorPoint *ap0 = anchor_path0->anchor_points[i];
		const PvAnchorPoint *ap1 = anchor_path1->anchor_points[i];
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

static void pv_anchor_path_change_head_index_(
		PvAnchorPath *self,
		int head)
{
	size_t num = pv_anchor_path_get_anchor_point_num(self);
	PvAnchorPoint **aps = malloc(sizeof(PvAnchorPoint *) * num);
	pv_assert(aps);

	pv_debug( "%d %zu", head, num);
	pv_assertf((head < (int)num), "%d %zu", head, num);
	for(int i = 0; i < (int)num; i++){
		aps[i] = self->anchor_points[(head + i) % (int)num];
	}
	for(int i = 0; i < (int)num; i++){
		self->anchor_points[i] = aps[i];
	}

	free(aps);
}

bool pv_anchor_path_reorder_open_from_index(PvAnchorPath *self, int index)
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

	PvAnchorPoint *ap = pv_anchor_path_duplicating_anchor_point_(self, index);
	pv_assert(ap);

	pv_anchor_path_change_head_index_(self, index + 1);
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
	size_t num = pv_anchor_path_get_anchor_point_num(self);
	pv_debug("anchor:%zu(%s)", num, (self->is_close)? "true":"false");

	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = self->anchor_points[i];
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

