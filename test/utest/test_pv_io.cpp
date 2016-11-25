#include <gtest/gtest.h>

extern "C"
{
#include "pv_io.h"
}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

#define FILEPATH_SVG "library/23.svg"

TEST(Test, PvIo_Read){
	PvVg *vg = pv_io_new_from_file(FILEPATH_SVG);
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

