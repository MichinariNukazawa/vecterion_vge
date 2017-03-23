#include "pv_io_util.h"

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "pv_error.h"
#include "pv_general.h"

double pv_io_util_get_double_from_str(const char *str)
{
	char *endptr = NULL;

	errno = 0;    /* To distinguish success/failure after call */
	double val = strtod(str, &endptr);
	if(0 != errno || str == endptr){
		pv_warning("%s", str);
		return 0.0;
	}

	return val;
}

bool pv_io_util_get_pv_color_from_svg_str_rgba(PvColor *ret_color, const char *str)
{
	PvColor color = PvColor_None;

	int ret;
	unsigned int r = 0, g = 0, b = 0;
	double a = 100.0;
	if(4 == (ret = sscanf(str, " rgba(%3u,%3u,%3u,%lf)", &r, &g, &b, &a))){
		goto match;
	}

	char strv[9];
	if(1 == (ret = sscanf(str, " #%8[0-9A-Fa-f]", strv))){
		for(int i = 0; i < (int)strlen(strv); i++){
			strv[i] = (char)tolower(strv[i]);
		}
		switch(strlen(strv)){
			case 3:
				{
					if(3 != sscanf(strv, "%1x%1x%1x", &r, &g, &b)){
						pv_warning("'%s'", str);
						goto error;
					}
					a = 1.0;

					goto match;
				}
				break;
			case 4:
				{
					unsigned int a_x = 0;
					if(4 != sscanf(strv, "%1x%1x%1x%1x", &r, &g, &b, &a_x)){
						pv_warning("'%s'", str);
						goto error;
					}
					a = ((double)a_x / ((int)0xF));

					goto match;
				}
				break;
			case 6:
				{
					if(3 != sscanf(strv, "%2x%2x%2x", &r, &g, &b)){
						pv_warning("'%s'", str);
						goto error;
					}
					a = 1.0;

					goto match;
				}
				break;
			case 8:
				{
					unsigned int a_x = 0;
					if(4 != sscanf(strv, "%2x%2x%2x%2x", &r, &g, &b, &a_x)){
						pv_warning("'%s'", str);
						goto error;
					}
					a = ((double)a_x / ((int)0xFF));

					goto match;
				}
				break;
			default:
				goto error;
				pv_warning("'%s'", str);
		}
	}

	if(NULL != strstr(str, "none")){
		*ret_color = PvColor_None;
		return true;
	}

	goto error;

match:
	{
		// pv_debug(" rgba(%u,%u,%u,%f)", r, g, b, a);
		bool ok = (pv_color_set_parameter(&color, PvColorParameterIx_R, (double)r)
				&& pv_color_set_parameter(&color, PvColorParameterIx_G, (double)g)
				&& pv_color_set_parameter(&color, PvColorParameterIx_B, (double)b)
				&& pv_color_set_parameter(&color, PvColorParameterIx_O, (double)a * 100.0)
			  );
		if(!ok){
			pv_warning("'%s'", str);
			goto error;
		}
	}

	*ret_color = color;
	return true;

error:
	pv_warning("'%s'", str);

	*ret_color = PvColor_None;
	return false;
}

bool pv_read_args_from_str(double *args, int num_args, const char **str)
{
	const char *p = *str;
	bool res = true;

	int i = 0;
	while('\0' != *p){
		if(',' == *p){
			p++;
			continue;
		}
		char *next = NULL;
		const char *str_error = NULL;
		if(!pv_general_strtod(&args[i], p, &next, &str_error)){
			res = false;
			break;
		}
		p = next;
		i++;
		if(!(i < num_args)){
			break;
		}
	}
	for(; i < num_args; i++){
		args[i] = 0;
	}

	*str = p;

	return res;
}

void pv_double_array_fill(double *dst, double value, int size)
{
	for(int i = 0; i < size; i++){
		dst[i] = value;
	}
}

void pv_str_maps_free(PvStrMap *map)
{
	if(NULL == map){
		return;
	}

	for(int i = 0; NULL != map[i].key; i++){
		free(map[i].key);
		free(map[i].value);
	}
	free(map);
}

