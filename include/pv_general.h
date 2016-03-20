#ifndef include_PV_GENERAL_H
#define include_PV_GENERAL_H

#include <stdbool.h>

/** @brief pointer arrayの内容数を返す
 * (実長さは番兵のNULL終端があるため、return+1)
 */
int pv_general_get_parray_num(void **pointers);


bool pv_general_strtod(double *value, const char *str,
		char **endptr, const char **str_error);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_GENERAL_H

