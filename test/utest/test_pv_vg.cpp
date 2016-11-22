#include <gtest/gtest.h>

extern "C"
{
#include "pv_vg.h"
#include "pv_io.h"
}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

TEST(Test, PvVg){
	PvVg *vg = pv_vg_new();
	ASSERT_TRUE(NULL != vg);

	PvElement *elem = pv_vg_get_layer_top(vg);
	ASSERT_TRUE(NULL != elem);
	ASSERT_EQ(PvElementKind_Layer, elem->kind);

	ASSERT_TRUE(NULL != vg->element_root);
	ASSERT_TRUE(NULL != vg->element_root->childs[0]);
	int num = pv_general_get_parray_num((void **)vg->element_root->childs);
	ASSERT_EQ(1, num);

	pv_vg_free(vg);
}

TEST(Test, CopyAndDiff){

	{
		PvVg *vg = pv_vg_new();
		PvVg *vg2 = pv_vg_copy_new(vg);
		ASSERT_TRUE(NULL != vg2);
		ASSERT_FALSE(pv_vg_is_diff(vg, vg2));

		PvVg *vg3 = pv_vg_new();
		if(NULL == vg3){
			FAIL();
		}
		bool ret;
		ret = pv_vg_copy_overwrite(vg3, vg);
		ASSERT_TRUE(ret);
		ASSERT_FALSE(pv_vg_is_diff(vg, vg3));
	}

	const char *filepath = "test/utest/image_01.svg";
	PvVg *vg = pv_io_new_from_file(filepath);
	ASSERT_TRUE(NULL != vg);
	ASSERT_TRUE(NULL != vg->element_root);
	ASSERT_TRUE(NULL != vg->element_root->childs[0]);
	/*
	   int num0 = pv_general_get_parray_num((void **)vg->element_root->childs);
	   int num1 = pv_general_get_parray_num((void **)vg->element_root->childs[0]->childs);
	   int num2 = pv_general_get_parray_num((void **)vg->element_root->childs[1]->childs);
	   printf("nums:%d,%d,%d,\n", num0, num1, num2);
	   ASSERT_EQ(1, num0);
	 */

	PvVg *vg2 = pv_vg_copy_new(vg);
	ASSERT_TRUE(NULL != vg2);

	PvVg *vg3 = pv_vg_new();
	if(NULL == vg3){
		FAIL();
	}
	bool ret;
	ret = pv_vg_copy_overwrite(vg3, vg);
	ASSERT_TRUE(ret);

	// ** copy not difference
	ret = pv_vg_is_diff(vg, vg2);
	ASSERT_FALSE(ret);
	ret = pv_vg_is_diff(vg, vg3);
	ASSERT_FALSE(ret);

	// ** difference vg (rect)
	PvVg *vg_diff = NULL;
	vg_diff = pv_vg_copy_new(vg);
	if(NULL == vg_diff){
		FAIL();
	}
	(vg_diff->rect.x)++;
	ret = pv_vg_is_diff(vg, vg_diff);
	ASSERT_TRUE(ret);
	pv_vg_free(vg_diff);

	// ** difference tree (add)
	{
		vg_diff = pv_vg_copy_new(vg);
		if(NULL == vg_diff){
			FAIL();
		}
		PvElement *elem_new = pv_element_new(PvElementKind_Layer);
		PvElement *elem_layer = pv_vg_get_layer_top(vg_diff);
		if(!pv_element_append_child(elem_layer, NULL, elem_new)){
			FAIL();
		}
		ret = pv_vg_is_diff(vg, vg_diff);
		ASSERT_TRUE(ret);
	}

	// ** difference tree (remove)
	{
		vg_diff = pv_vg_copy_new(vg);
		if(NULL == vg_diff){
			FAIL();
		}
		// TODO: fix this (from pv_io ?)
		// PvElement *elem_layer = pv_vg_get_layer_top(vg_diff);
		int num_ = pv_general_get_parray_num((void **)(vg->element_root->childs));
		assert(1 == num_);
		PvElement *elem_layer = vg->element_root->childs[0];
		int num = pv_general_get_parray_num((void **)(elem_layer->childs));
		if(num < 1){
			FAIL();
		}
		if(!pv_element_remove_delete_recursive(elem_layer->childs[0])){
			FAIL();
		}
		ret = pv_vg_is_diff(vg, vg_diff);
		ASSERT_TRUE(ret);
	}

	// ** difference element (add AnchorPoint)
	{
		vg_diff = pv_vg_copy_new(vg);
		if(NULL == vg_diff){
			FAIL();
		}
		PvElement *elem_layer = vg->element_root->childs[0];
		int num = pv_general_get_parray_num((void **)(elem_layer->childs));
		if(num < 1){
			FAIL();
		}
		PvElement *elem_bezier = elem_layer->childs[0];
		if(PvElementKind_Bezier != elem_bezier->kind){
			FAIL();
		}
		PvAnchorPoint ap = {
			.points = {{0,0},{10,20},{0,0}},
		};
		if(!pv_element_bezier_add_anchor_point(elem_bezier, ap)){
			FAIL();
		}
		ret = pv_vg_is_diff(vg, vg_diff);
		ASSERT_TRUE(ret);
	}

	// independ (copyed vg2,3 is fine when after free vg)
	pv_vg_free(vg);
	pv_vg_is_diff(vg2, vg3);
	pv_vg_free(vg3);
	pv_vg_free(vg2);
}

