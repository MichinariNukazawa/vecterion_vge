#include <gtest/gtest.h>

extern "C"
{
#include "pv_vg.h"
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
}

