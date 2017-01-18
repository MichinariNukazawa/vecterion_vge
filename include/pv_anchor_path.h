#ifndef include_PV_BEZIER_H
#define include_PV_BEZIER_H

#include "pv_anchor_point.h"


typedef enum{
	PvAnchorPathIndexTurn_Disable,
	PvAnchorPathIndexTurn_OnlyLastInClosed,
}PvAnchorPathIndexTurn;

struct PvAnchorPath;
typedef struct PvAnchorPath PvAnchorPath;


PvAnchorPath *pv_anchor_path_new();
void pv_anchor_path_free(PvAnchorPath *);
PvAnchorPath *pv_anchor_path_copy_new(const PvAnchorPath *);
PvAnchorPath *pv_anchor_path_copy_new_range(const PvAnchorPath *, int head, int foot);
PvAnchorPath *pv_anchor_path_split_new_from_index(PvAnchorPath *, int index);

void pv_anchor_path_add_anchor_point(PvAnchorPath *, const PvAnchorPoint *);
PvAnchorPoint *pv_anchor_path_insert_anchor_point(PvAnchorPath *, const PvAnchorPoint *, int index);
bool pv_anchor_path_remove_delete_range(PvAnchorPath *, int head, int foot);

PvAnchorPoint *pv_anchor_path_get_anchor_point_from_index(PvAnchorPath *, int index, PvAnchorPathIndexTurn);
const PvAnchorPoint *pv_anchor_path_get_anchor_point_from_index_const(const PvAnchorPath *, int index);
bool pv_anchor_path_set_anchor_point_from_index(PvAnchorPath *, int index, const PvAnchorPoint *);
size_t pv_anchor_path_get_anchor_point_num(const PvAnchorPath *);
int pv_anchor_path_get_index_from_anchor_point(const PvAnchorPath *, const PvAnchorPoint *);
bool pv_anchor_path_is_exist_anchor_point(const PvAnchorPath *, const PvAnchorPoint *);
void pv_anchor_path_set_is_close(PvAnchorPath *, bool is_close);
bool pv_anchor_path_get_is_close(const PvAnchorPath *);
bool pv_anchor_path_is_diff(const PvAnchorPath *, const PvAnchorPath *);
bool pv_anchor_path_split_anchor_point_from_index(PvAnchorPath *, int index);
bool pv_anchor_path_get_anchor_point_p4_from_index(const PvAnchorPath *, PvAnchorPointP4 *, int index);

void pv_anchor_path_debug_print(const PvAnchorPath *);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_BEZIER_H

