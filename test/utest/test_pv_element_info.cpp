#include <gtest/gtest.h>

extern "C"
{
#include "pv_error.h"
#include "pv_element.h"
#include "pv_element_info.h"

static PvAnchorPoint *anchor_path_get_ap_from_index(
		PvAnchorPath *anchor_path,
		int index)
{
	return pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			index,
			PvAnchorPathIndexTurn_Disable);
}

}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

TEST(Test, curve_removeDeleteAnchorPoint){
	const PvElementInfo *info = pv_element_get_info_from_kind(PvElementKind_Curve);
	EXPECT_TRUE(NULL != info);

	PvElement *src_element_parent = pv_element_new(PvElementKind_Layer);
	pv_assert(src_element_parent);
	PvElement *src_element = pv_element_curve_new_from_rect((PvRect){0.0,1.0,1.0,1.0});
	pv_assert(src_element);
	pv_element_append_child(src_element_parent, NULL, src_element);

	PvElement *element_parent = NULL;
	PvElement *element = NULL;
	PvElement *foot_element = NULL;
	PvAnchorPoint *anchor_point = NULL;
	bool is_delete;
	PvElementCurveData *data = NULL;
	PvElementCurveData *foot_data = NULL;
	int num;
	PvAnchorPoint *ap2;
	PvAnchorPoint *aps[4];
	PvAnchorPoint *ap_aft_h;

	// delete in close path
	element_parent = pv_element_copy_recursive(src_element_parent);
	pv_assert(element_parent);
	pv_assert(element_parent->childs[0]);
	element = element_parent->childs[0];
	foot_element = NULL;
	data = (PvElementCurveData *)element->data;
	anchor_point = pv_anchor_path_get_anchor_point_from_index(
			element->anchor_path,
			1,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(anchor_point);
	is_delete = false;
	ap2 = pv_anchor_path_get_anchor_point_from_index(
			element->anchor_path,
			2,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap2);
	pv_assert(true == pv_anchor_path_get_is_close(element->anchor_path));
	info->func_remove_delete_anchor_point(
			element, anchor_point,
			&foot_element, &is_delete);
	EXPECT_TRUE(false == pv_anchor_path_get_is_close(element->anchor_path));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)element_parent->childs));
	EXPECT_TRUE(NULL == foot_element);
	EXPECT_TRUE(false == is_delete);
	num = pv_element_curve_get_num_anchor_point(element);
	EXPECT_EQ(3, num);
	EXPECT_TRUE(false == pv_anchor_path_get_is_close(element->anchor_path));
	ap_aft_h = pv_anchor_path_get_anchor_point_from_index(
			element->anchor_path,
			0,
			PvAnchorPathIndexTurn_Disable);
	EXPECT_EQ(ap2, ap_aft_h);
	pv_element_remove_free_recursive(element_parent);



	// delete in open path
	//
	element_parent = pv_element_copy_recursive(src_element_parent);
	pv_assert(element_parent);
	pv_assert(element_parent->childs[0]);
	element = element_parent->childs[0];
	foot_element = NULL;
	data = (PvElementCurveData *)element->data;
	anchor_point = pv_anchor_path_get_anchor_point_from_index(
			element->anchor_path,
			1,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(anchor_point);
	is_delete = false;
	for(int i = 0; i < 4; i++){
	aps[i] = pv_anchor_path_get_anchor_point_from_index(
			element->anchor_path,
			i,
			PvAnchorPathIndexTurn_Disable);
	}
	pv_assert(ap2);
	pv_anchor_path_set_is_close(element->anchor_path, false);
	//
	info->func_remove_delete_anchor_point(
			element, anchor_point,
			&foot_element, &is_delete);
	//
	EXPECT_TRUE(false == pv_anchor_path_get_is_close(element->anchor_path));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)element_parent->childs));
	EXPECT_TRUE(NULL != foot_element);
	EXPECT_TRUE(false == is_delete);
	EXPECT_EQ(1, pv_element_curve_get_num_anchor_point(element));
	EXPECT_EQ(2, pv_element_curve_get_num_anchor_point(foot_element));
	foot_data = (PvElementCurveData *)foot_element->data;
	EXPECT_TRUE(false == pv_anchor_path_get_is_close(element->anchor_path));
	EXPECT_TRUE(false == pv_anchor_path_get_is_close(foot_element->anchor_path));
	EXPECT_EQ(aps[0], anchor_path_get_ap_from_index(element->anchor_path, 0));
	EXPECT_EQ(aps[2], anchor_path_get_ap_from_index(foot_element->anchor_path, 0));
	EXPECT_EQ(aps[3], anchor_path_get_ap_from_index(foot_element->anchor_path, 1));
	pv_element_remove_free_recursive(element_parent);

}

