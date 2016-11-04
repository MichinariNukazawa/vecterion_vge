#include <gtest/gtest.h>

extern "C"
{
#include "pv_file_format.h"
}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

TEST(Test, change){
	{
		const char *filepath01 = "./tes/test.svg";
		char *out01 = pv_file_format_change_new_extension_from_filepath(filepath01, "png");
		EXPECT_STREQ("./tes/test.png", out01);
	}

	{
		const char *filepath02 = "test.svg";
		char *out02 = pv_file_format_change_new_extension_from_filepath(filepath02, "png");
		EXPECT_STREQ("test.png", out02);
	}

	{
		const char *src_path = "tes/test";
		char *out = pv_file_format_change_new_extension_from_filepath(src_path, "png");
		EXPECT_STREQ("tes/test.png", out);
	}

	{
		const char *src_path = "tes.tests/test";
		char *out = pv_file_format_change_new_extension_from_filepath(src_path, "png");
		EXPECT_STREQ("tes.tests/test.png", out);
	}

	{
		const char *src_path = NULL;
		char *out = pv_file_format_change_new_extension_from_filepath(src_path, "png");
		EXPECT_STREQ("untitled_document.png", out);
	}

	{
		const char *src_path = "tes.tests/";
		char *out = pv_file_format_change_new_extension_from_filepath(src_path, "png");
		EXPECT_STREQ("tes.tests/.png", out);
	}

}

