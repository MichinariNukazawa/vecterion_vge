#include <gtest/gtest.h>

extern "C"
{
#include "pv_error.h"
#include "pv_anchor_path.h"
}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

TEST(Test, Split){
	PvAnchorPath *path_src = pv_anchor_path_new();
	pv_assert(path_src);
	pv_anchor_path_set_is_close(path_src, true);

	for(int i = 0; i < 6; i++){
		PvAnchorPoint ap = pv_anchor_point_from_point((PvPoint){0, (double)i});
		pv_anchor_path_add_anchor_point(path_src, &ap);
	}
	size_t num;
	num = pv_anchor_path_get_anchor_point_num(path_src);
	EXPECT_EQ(6, (int)num);

	PvAnchorPath *path = NULL;
	PvAnchorPath *path_t = NULL;
	const PvAnchorPoint *ap_h = NULL;
	const PvAnchorPoint *ap_f = NULL;

	// split tail
	path = pv_anchor_path_copy_new(path_src);
	ap_h = pv_anchor_path_get_anchor_point_from_index_const(path, 0);
	pv_assert(ap_h);
	ap_f = pv_anchor_path_get_anchor_point_from_index_const(path, 5);
	pv_assert(ap_f);
	path_t = pv_anchor_path_split_new_from_index(path, 5);
	EXPECT_TRUE(path_t != NULL);
	EXPECT_TRUE(false == pv_anchor_path_get_is_close(path));
	num = pv_anchor_path_get_anchor_point_num(path);
	EXPECT_EQ(6, (int)num);
	num = pv_anchor_path_get_anchor_point_num(path_t);
	EXPECT_EQ(1, (int)num);
	EXPECT_EQ(ap_h, pv_anchor_path_get_anchor_point_from_index_const(path, 0));
	EXPECT_EQ(ap_f, pv_anchor_path_get_anchor_point_from_index_const(path_t, 0));
	pv_anchor_path_free(path_t);
	pv_anchor_path_free(path);

	// split head
	path = pv_anchor_path_copy_new(path_src);
	ap_h = pv_anchor_path_get_anchor_point_from_index_const(path, 0);
	pv_assert(ap_h);
	ap_f = pv_anchor_path_get_anchor_point_from_index_const(path, 5);
	pv_assert(ap_f);
	path_t = pv_anchor_path_split_new_from_index(path, 0);
	EXPECT_TRUE(path_t != NULL);
	EXPECT_TRUE(false == pv_anchor_path_get_is_close(path));
	num = pv_anchor_path_get_anchor_point_num(path);
	EXPECT_EQ(1, (int)num);
	num = pv_anchor_path_get_anchor_point_num(path_t);
	EXPECT_EQ(6, (int)num);
	//EXPECT_EQ(ap_h, pv_anchor_path_get_anchor_point_from_index_const(path, 0));
	EXPECT_EQ(ap_f, pv_anchor_path_get_anchor_point_from_index_const(path_t, 5));
	pv_anchor_path_free(path_t);
	pv_anchor_path_free(path);

	// split internal
	path = pv_anchor_path_copy_new(path_src);
	ap_h = pv_anchor_path_get_anchor_point_from_index_const(path, 0);
	pv_assert(ap_h);
	ap_f = pv_anchor_path_get_anchor_point_from_index_const(path, 5);
	pv_assert(ap_f);
	path_t = pv_anchor_path_split_new_from_index(path, 1);
	EXPECT_TRUE(path_t != NULL);
	EXPECT_TRUE(false == pv_anchor_path_get_is_close(path));
	num = pv_anchor_path_get_anchor_point_num(path);
	EXPECT_EQ(2, (int)num);
	num = pv_anchor_path_get_anchor_point_num(path_t);
	EXPECT_EQ(5, (int)num);
	EXPECT_EQ(ap_h, pv_anchor_path_get_anchor_point_from_index_const(path, 0));
	//EXPECT_EQ(ap_f, pv_anchor_path_get_anchor_point_from_index_const(path_t, 0));
	pv_anchor_path_free(path_t);
	pv_anchor_path_free(path);

	pv_anchor_path_free(path_src);
}

