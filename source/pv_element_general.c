#include "pv_element_general.h"

#include "pv_error.h"

int pv_general_get_parray_num(void **pointers)
{
	if(NULL == pointers){
		return 0;
	}

	int i = 0;
	while(NULL != pointers[i]){
		i++;
	}

	if(15 < i){
		pv_debug("num:%d\n", i);
	}

	return i;
}

