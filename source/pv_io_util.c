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
	PvColor color = *ret_color;

	int ret;
	unsigned int r = 0, g = 0, b = 0;
	double a = 100.0;
	if(4 == (ret = sscanf(str, " rgba(%3u,%3u,%3u,%lf)", &r, &g, &b, &a))){
		color.none_weight = 1;
		goto match_rgba;
	}

	char strv[9];
	if(1 == (ret = sscanf(str, " #%6[0-9A-Fa-f]", strv))){
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
					color.none_weight = 1;

					goto match_rgb;
				}
				break;
			case 6:
				{
					if(3 != sscanf(strv, "%2x%2x%2x", &r, &g, &b)){
						pv_warning("'%s'", str);
						goto error;
					}

					a = 1.0;
					color.none_weight = 1;

					goto match_rgb;
				}
				break;
			default:
				pv_warning("'%s'", str);
				goto error;
		}
	}

	if(NULL != strstr(str, "none")){
		r = 0;
		g = 0;
		b = 0;
		a = 1;
		color.none_weight = 0;
		goto match_rgba;
	}

	pv_warning("'%s'", str);
	goto error;

match_rgba:
	if(!pv_color_set_parameter(&color, PvColorParameterIx_O, (double)a * 100)){
		pv_warning("'%s'", str);
		goto error;
	}

match_rgb:
	{
		// pv_debug(" rgba(%u,%u,%u,%f)", r, g, b, a);
		bool ok = (pv_color_set_parameter(&color, PvColorParameterIx_R, (double)r)
				&& pv_color_set_parameter(&color, PvColorParameterIx_G, (double)g)
				&& pv_color_set_parameter(&color, PvColorParameterIx_B, (double)b)
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

bool pv_read_args_from_str(double *args, size_t num_args, const char **str)
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
		if(!(i < (int)num_args)){
			break;
		}
	}
	for(; i < (int)num_args; i++){
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

PvStrMap *pv_new_css_str_maps_from_str(const char *style_str)
{
	const char *head = style_str;

	size_t num = 0;
	PvStrMap *map = NULL;
	map = realloc(map, sizeof(PvStrMap) * (num + 1));
	map[num - 0].key = NULL;
	map[num - 0].value = NULL;

	while(NULL != head && '\0' != *head){

		char *skey;
		char *svalue;
		if(2 != sscanf(head, " %m[^:;] : %m[^;]", &skey, &svalue)){
			break;
		}
		num++;

		map = realloc(map, sizeof(PvStrMap) * (num + 1));
		pv_assert(map);

		map[num - 0].key = NULL;
		map[num - 0].value = NULL;
		map[num - 1].key = skey;
		map[num - 1].value = svalue;

		head = strchr(head, ';');
		if(NULL == head){
			break;
		}
		if(';' == *head){
			head++;
		}
	}

	return map;
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

