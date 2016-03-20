#include <gtest/gtest.h>

extern "C"
{
#include "pv_general.h"
}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

TEST(Test, parray_num){
	int num;

	int dummy1;
	int *parray1[12];
	for(int i = 0; i < 12; i++){
		parray1[i] = &dummy1;
	}
	parray1[11] = NULL;
	num = pv_general_get_parray_num((void **)parray1);
	EXPECT_EQ(11, num);

	parray1[7] = NULL;
	num = pv_general_get_parray_num((void **)parray1);
	EXPECT_EQ(7, num);

	num = pv_general_get_parray_num(NULL);
	EXPECT_EQ(0, num);
}

