#include "pv_svg_info.h"

#include <strings.h>
#include "pv_error.h"

static PvElement *_pv_svg_svg_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	return element_parent;
}

static PvElement *_pv_svg_g_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	// ** is exist groupmode=layer
	PvElementKind kind = PvElementKind_Group;
	xmlChar *value = NULL;
	/*
	   value = xmlGetNsProp(xmlnode,"groupmode",
	   "http://www.inkscape.org/namespaces/inkscape");
	   if(NULL == value){
	   value = xmlGetProp(xmlnode,"inkscape:groupmode");
	   }
	 */
	if(NULL == value){
		value = xmlGetProp(xmlnode, BAD_CAST "groupmode");
	}
	if(NULL != value){
		if(0 == strcasecmp("layer", (char*)value)){
			kind = PvElementKind_Layer;
		}
	}
	xmlFree(value);

	PvElement *element_new = pv_element_new(kind);
	if(NULL == element_new){
		pv_error("");
		return NULL;
	}
	if(!pv_element_append_child(element_parent, NULL, element_new)){
		pv_error("");
		return NULL;
	}

	return element_new;
}

void pv_element_anchor_point_init(PvAnchorPoint *ap)
{
	*ap = PvAnchorPoint_Default;
}

static const char *_pv_svg_read_args_from_str(double *args, int num_args, const char *str)
{
	const char *p = str;
	char *next;
	int i = 0;
	const char *str_error = NULL;
	while('\0' != *p){
		if(',' == *p){
			p++;
			continue;
		}
		if(!pv_general_strtod(&args[i], p, &next, &str_error)){
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

	return p;
}


static void _pv_svg_fill_double_array(double *dst, double value, int size)
{
	for(int i = 0; i < size; i++){
		dst[i] = value;
	}
}

static bool _pv_svg_path_set_anchor_points_from_str(PvElement *element, const char *str)
{
	const int num_args = 10;
	double args[num_args];
	double prev_args[num_args];
	_pv_svg_fill_double_array(args, 0, num_args);
	_pv_svg_fill_double_array(prev_args, 0, num_args);

	PvAnchorPoint ap;
	if(NULL == element || NULL == element->data){
		pv_error("");
		return false;
	}
	PvElementBezierData *data = element->data;

	const char *p = str;
	while('\0' != *p){
		bool is_append = false;
		switch(*p){
			case 'M':
			case 'L':
				p = _pv_svg_read_args_from_str(args, 2, ++p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_Point].x = args[0];
				ap.points[PvAnchorPointIndex_Point].y = args[1];
				is_append = true;
				break;
			case 'C':
				p = _pv_svg_read_args_from_str(args, 6, ++p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_HandlePrev].x = args[2] - args[4];
				ap.points[PvAnchorPointIndex_HandlePrev].y = args[3] - args[5];
				ap.points[PvAnchorPointIndex_Point].x = args[4];
				ap.points[PvAnchorPointIndex_Point].y = args[5];
				ap.points[PvAnchorPointIndex_HandleNext].x = 0;
				ap.points[PvAnchorPointIndex_HandleNext].y = 0;
				if(0 < data->anchor_points_num){
					PvAnchorPoint *ap_prev = &data->anchor_points[data->anchor_points_num - 1];
					PvPoint gpoint_next = {args[0], args[1]};
					pv_anchor_point_set_handle(ap_prev,
							PvAnchorPointIndex_HandleNext, gpoint_next);
				}else{
					// 'C' command on top?
					pv_warning("");
				}
				is_append = true;
				break;
			case 'S':
				p = _pv_svg_read_args_from_str(args, 4, ++p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_HandlePrev].x = args[0] - args[2];
				ap.points[PvAnchorPointIndex_HandlePrev].y = args[1] - args[3];
				ap.points[PvAnchorPointIndex_Point].x = args[2];
				ap.points[PvAnchorPointIndex_Point].y = args[3];
				ap.points[PvAnchorPointIndex_HandleNext].x = 0;
				ap.points[PvAnchorPointIndex_HandleNext].y = 0;
				is_append = true;
				break;
			case 'Z':
			case 'z':
				p++;
				data->is_close = true;
				break;
			case ' ':
			case ',':
				p++;
				break;
			default:
				p++;
		}

		if(is_append){
			if(!pv_element_bezier_add_anchor_point(element, ap)){
				pv_error("");
				return false;
			}
		}
	}

	return true;
}

static bool _set_stroke_linecap_from_str(PvElement *element, const char *str)
{
	int num = get_num_stroke_linecap_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinecapInfo *info = get_stroke_linecap_info_from_id(i);
		if(0 == strcasecmp(str, info->name)){
			element->stroke.linecap = info->linecap;
			return true;
		}
	}

	return true;
}

static bool _set_stroke_linejoin_from_str(PvElement *element, const char *str)
{
	int num = get_num_stroke_linejoin_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinejoinInfo *info = get_stroke_linejoin_info_from_id(i);
		if(0 == strcasecmp(str, info->name)){
			element->stroke.linejoin = info->linejoin;
			return true;
		}
	}

	return true;
}

static PvElement *_pv_svg_path_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	PvElement *element_new = pv_element_new(PvElementKind_Bezier);
	if(NULL == element_new){
		pv_error("");
		return NULL;
	}

	xmlChar *value = NULL;
	value = xmlGetProp(xmlnode, BAD_CAST "d");
	if(NULL != value){
		// pv_debug("d='%s'", (char *)value);
		if(!_pv_svg_path_set_anchor_points_from_str(element_new, (char*)value)){
			pv_error("");
			return NULL;
		}

		// pv_element_debug_print(element_new);
	}
	xmlFree(value);

	//! ** stroke-* attributes.
	value = xmlGetProp(xmlnode, BAD_CAST "stroke-width");
	if(NULL != value){
		double width = 1.0;
		int ret = sscanf((const char *)value, "%lf", &width);
		pv_debug("stroke-width='%s' %d %.3f",
				(char *)value, ret, width);
		if(1 == ret){
			element_new->stroke.width = width;
		}
	}
	value = xmlGetProp(xmlnode, BAD_CAST "stroke-linecap");
	if(NULL != value){
		if(!_set_stroke_linecap_from_str(element_new, (char*)value)){
			pv_error("");
			return NULL;
		}
	}
	value = xmlGetProp(xmlnode, BAD_CAST "stroke-linejoin");
	if(NULL != value){
		if(!_set_stroke_linejoin_from_str(element_new, (char*)value)){
			pv_error("");
			return NULL;
		}
	}

	if(!pv_element_append_child(element_parent, NULL, element_new)){
		pv_error("");
		return NULL;
	}

	return element_new;
}

static PvElement *_pv_svg_text_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	// nop
	return element_parent;
}

static PvElement *_pv_svg_unknown_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	pv_warning("Not implement:'%s'(%d)", xmlnode->name, xmlnode->line);

	return element_parent;
}

const PvSvgInfo _pv_svg_infos[] = {
	{
		.tagname = "svg",
		.func_new_element_from_svg = _pv_svg_svg_new_element_from_svg,
	},
	{
		.tagname = "g",
		.func_new_element_from_svg = _pv_svg_g_new_element_from_svg,
	},
	{
		.tagname = "path",
		.func_new_element_from_svg = _pv_svg_path_new_element_from_svg,
	},
	{
		.tagname = "text",
		.func_new_element_from_svg = _pv_svg_text_new_element_from_svg,
	},
	/* Unknown (or not implement) tag */
	{
		.tagname = NULL,
		.func_new_element_from_svg = _pv_svg_unknown_new_element_from_svg,
	},
};

const PvSvgInfo *pv_svg_get_svg_info_from_tagname(const char *tagname)
{
	const PvSvgInfo *info = &_pv_svg_infos[0];
	for(int i = 0; NULL != info->tagname; i++){
		if(0 == strcasecmp(tagname, info->tagname)){
			return info;
		}
		info = &_pv_svg_infos[i];
	}

	return info;
}

