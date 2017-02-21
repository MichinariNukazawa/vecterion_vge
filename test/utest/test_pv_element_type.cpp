#include <gtest/gtest.h>

#define RASTER_FILEPATH_1 "./resource/vecterion/daisy_bell_header_r2.jpg"

extern "C"
{
#include "pv_vg.h"
#include "pv_element.h"
}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

TEST(Test, PvElement_Raster){
	PvVg *vg = pv_vg_new();
	assert(NULL != vg);

	PvElement *element_parent = pv_vg_get_layer_top(vg);

	PvElement *element_raster0 = pv_element_new(PvElementKind_Raster);
	ASSERT_TRUE(NULL != element_raster0);
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_raster0));

	PvElement *element_raster1 = pv_element_raster_new_from_filepath(RASTER_FILEPATH_1);
	ASSERT_TRUE(NULL != element_raster1);
	PvElementRasterData *data = (PvElementRasterData *)element_raster1->data;
	ASSERT_TRUE(NULL != data);
	ASSERT_TRUE(NULL != data->path);
	ASSERT_STREQ(data->path, RASTER_FILEPATH_1);
	ASSERT_TRUE(NULL != data->pixbuf);
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_raster1));

	PvElement *element_raster2
		= pv_element_raster_new_from_filepath("./test/testdata/invalid.jpg");
	ASSERT_TRUE(NULL == element_raster2);

	pv_vg_free(vg);
}

TEST(Test, PvVg_OnAllElementKind){
	PvVg *vg = pv_vg_new();
	assert(NULL != vg);

	PvElement *element_parent = pv_vg_get_layer_top(vg);

	// ** vg on all kind (default empty) element
	for(int k = (PvElementKind_NotDefined + 1); k < PvElementKind_EndOfKind; k++){
		if(PvElementKind_Root == k){
			continue;
		}
		PvElement *element = pv_element_new((PvElementKind)k);
		ASSERT_TRUE(NULL != element);
		ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element));
	}

	// **curve
	PvElement *element_curve0 = pv_element_new(PvElementKind_Curve);
	PvAnchorPoint ap = {
		.points = {{0,0},{10,20},{0,0}},
	};
	assert(pv_element_curve_add_anchor_point(element_curve0, ap));
	assert(1 == pv_element_curve_get_num_anchor_point(element_curve0));
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_curve0));
	// ** raster
	PvElement *element_raster1 = pv_element_raster_new_from_filepath(RASTER_FILEPATH_1);
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_raster1));

	// ** copy
	PvVg *vg2 = pv_vg_copy_new(vg);
	PvVg *vg3 = pv_vg_copy_new(vg);

	// ** copy not difference
	bool ret;
	ret = pv_vg_is_diff(vg, vg2);
	ASSERT_FALSE(ret);
	ret = pv_vg_is_diff(vg, vg3);
	ASSERT_FALSE(ret);

	// ** independ vg (copyed vg2,3 is fine when after free vg)
	pv_vg_free(vg);
	pv_vg_is_diff(vg2, vg3);
	pv_vg_free(vg3);
	pv_vg_free(vg2);
}

