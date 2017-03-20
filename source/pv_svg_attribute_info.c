#include "pv_svg_attribute_info.h"

#include <strings.h>
#include "pv_error.h"

static void pv_element_anchor_point_init(PvAnchorPoint *ap)
{
	*ap = PvAnchorPoint_Default;
}

static void _pv_svg_fill_double_array(double *dst, double value, int size)
{
	for(int i = 0; i < size; i++){
		dst[i] = value;
	}
}

static bool _pv_svg_read_args_from_str(double *args, int num_args, const char **str)
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



static bool func_stroke_width_set_(
				PvElement *element,
				const xmlNodePtr xmlnode,
				const xmlAttr *attribute
				)
{
	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "stroke-width");
	if(!value){
		pv_error("");
		return false;
	}

	xmlFree(value);

	return true;
}

static bool func_stroke_linecap_set_(
				PvElement *element,
				const xmlNodePtr xmlnode,
				const xmlAttr *attribute
				)
{
	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "stroke-linecap");
	if(!value){
		pv_error("");
		return false;
	}

	int num = get_num_stroke_linecap_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinecapInfo *info = get_stroke_linecap_info_from_id(i);
		if(0 == strcasecmp((char *)value, info->name)){
			element->stroke.linecap = info->linecap;
			return true;
		}
	}

	xmlFree(value);

	return true;
}

static bool func_stroke_linejoin_set_(
				PvElement *element,
				const xmlNodePtr xmlnode,
				const xmlAttr *attribute
				)
{
	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "stroke-linejoin");
	if(!value){
		pv_error("");
		return false;
	}

	int num = get_num_stroke_linejoin_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinejoinInfo *info = get_stroke_linejoin_info_from_id(i);
		if(0 == strcasecmp((char *)value, info->name)){
			element->stroke.linejoin = info->linejoin;
			return true;
		}
	}

	xmlFree(value);
	return true;
}

static bool func_d_set_inline_(
		PvElement *element,
		const char *value
		)
{
	const int num_args = 10;
	double args[num_args];
	double prev_args[num_args];
	_pv_svg_fill_double_array(args, 0, num_args);
	_pv_svg_fill_double_array(prev_args, 0, num_args);

	PvAnchorPoint ap;
	pv_assert(element);
	pv_assert(element->data);
	pv_assert(PvElementKind_Curve == element->kind);

	PvElementCurveData *data = element->data;

	const char *p = value;
	while('\0' != *p){
		bool is_append = false;
		switch(*p){
			case 'M':
			case 'L':
				p++;
				is_append = _pv_svg_read_args_from_str(args, 2, &p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_Point].x = args[0];
				ap.points[PvAnchorPointIndex_Point].y = args[1];
				break;
			case 'C':
				p++;
				is_append = _pv_svg_read_args_from_str(args, 6, &p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_HandlePrev].x = args[2] - args[4];
				ap.points[PvAnchorPointIndex_HandlePrev].y = args[3] - args[5];
				ap.points[PvAnchorPointIndex_Point].x = args[4];
				ap.points[PvAnchorPointIndex_Point].y = args[5];
				ap.points[PvAnchorPointIndex_HandleNext].x = 0;
				ap.points[PvAnchorPointIndex_HandleNext].y = 0;
				size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
				if(0 < num){
					PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(data->anchor_path, (num - 1), PvAnchorPathIndexTurn_Disable);
					PvPoint gpoint_next = {args[0], args[1]};
					pv_anchor_point_set_handle(ap_prev,
							PvAnchorPointIndex_HandleNext, gpoint_next);
				}else{
					// 'C' command on top?
					pv_warning("");
				}
				break;
			case 'S':
				p++;
				is_append = _pv_svg_read_args_from_str(args, 4, &p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_HandlePrev].x = args[0] - args[2];
				ap.points[PvAnchorPointIndex_HandlePrev].y = args[1] - args[3];
				ap.points[PvAnchorPointIndex_Point].x = args[2];
				ap.points[PvAnchorPointIndex_Point].y = args[3];
				ap.points[PvAnchorPointIndex_HandleNext].x = 0;
				ap.points[PvAnchorPointIndex_HandleNext].y = 0;
				break;
			case 'Z':
			case 'z':
				p++;
				pv_anchor_path_set_is_close(data->anchor_path, true);
				break;
			case ' ':
			case ',':
				p++;
				break;
			default:
				p++;
		}

		if(is_append){
			if(!pv_element_curve_add_anchor_point(element, ap)){
				pv_error("");
				return false;
			}
		}
	}

	return true;
}

static bool func_d_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 != strcasecmp("path", (char *)xmlnode->name)){
		pv_error("'%s'", attribute->name);
		return false;
	}

	if(PvElementKind_Curve != element->kind){
		pv_error("");
		return false;
	}

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "d");
	if(!value){
		pv_error("");
		return false;
	}

	bool ret = func_d_set_inline_(element, (const char *)value);

	xmlFree(value);

	return ret;
}


const PvSvgAttributeInfo _pv_svg_attribute_infos[] = {
	{
		.name = "stroke-width",
		.pv_svg_attribute_func_set = func_stroke_width_set_,
	},
	{
		.name = "stroke-linecap",
		.pv_svg_attribute_func_set = func_stroke_linecap_set_,
	},
	{
		.name = "stroke-linejoin",
		.pv_svg_attribute_func_set = func_stroke_linejoin_set_,
	},
	{
		.name = "d",
		.pv_svg_attribute_func_set = func_d_set_,
	},
};

const PvSvgAttributeInfo *pv_svg_get_svg_attribute_info_from_name(const char *name)
{
	int num = sizeof(_pv_svg_attribute_infos) / sizeof(_pv_svg_attribute_infos[0]);
	for(int i = 0; i < num; i++){
		const PvSvgAttributeInfo *info = &_pv_svg_attribute_infos[i];
		if(0 == strcasecmp(name, info->name)){
			return info;
		}
	}

	return NULL;
}

