#include "pv_general.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
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

	return i;
}

bool pv_general_strtod(double *value, const char *str,
		char **endptr, const char **str_error)
{
	errno = 0;
	double _value = strtod(str, endptr);
	if(str == *endptr){
		if(NULL != str_error){
			*str_error = "Not number.";
		}
		goto error;
	}
	if(0 != errno){
		// static char _str_error[128];
		// strerror_r(errno, _str_error, sizeof(_str_error));
		if(NULL != str_error){
			*str_error = strerror(errno);
		}
		goto error;
	}

	*value = _value;
	if(NULL != str_error){
		*str_error = NULL;
	}

	return true;
error:
	*value = 0;
	return false;
}

