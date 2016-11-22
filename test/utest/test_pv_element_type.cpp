#include <gtest/gtest.h>

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

#define RASTER_FILEPATH_1 "./test/testdata/daisy_bell_header_r2.jpg"
	PvElement *element_raster1
		= pv_element_raster_new_from_filepath(RASTER_FILEPATH_1);
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

