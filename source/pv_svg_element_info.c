#include "pv_svg_element_info.h"

#include <string.h>
#include <strings.h>
#include "pv_error.h"
#include "pv_io_util.h"
#include "pv_svg_attribute_info.h"
#include "pv_element_info.h"

static PvElement *_pv_svg_svg_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		ConfReadSvg *conf
		)
{
	return element_parent;
}

static PvStrMap *new_transform_str_maps_from_str_(const char *src_str)
{
	const char *head = src_str;

	int num = 0;
	PvStrMap *map = NULL;
	map = realloc(map, sizeof(PvStrMap) * (num + 1));
	map[num - 0].key = NULL;
	map[num - 0].value = NULL;

	while(NULL != head && '\0' != *head){

		char *skey;
		char *svalue;
		if(2 != sscanf(head, " %m[^(] ( %m[^)] )", &skey, &svalue)){
			break;
		}
		num++;

		map = realloc(map, sizeof(PvStrMap) * (num + 1));
		pv_assert(map);

		map[num - 0].key = NULL;
		map[num - 0].value = NULL;
		map[num - 1].key = skey;
		map[num - 1].value = svalue;

		head = svalue;
		while('\0' == *head
				|| ',' == *head){
			head++;
		}
	}

	return map;
}

static PvElement *_pv_svg_g_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		ConfReadSvg *conf
		)
{
	// ** is exist groupmode=layer
	PvElementKind kind = PvElementKind_Group;
	{
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
	}
	{
		xmlChar *value = NULL;
		value = xmlGetProp(xmlnode, BAD_CAST "transform");

		if(NULL != value){
			PvStrMap *transform_str_maps = new_transform_str_maps_from_str_((const char *)value);
			for(int i = 0; NULL != transform_str_maps[i].key; i++){
				if(0 == strcmp("translate", transform_str_maps[i].key)){

					const int num_args = 10;
					double args[num_args];
					pv_double_array_fill(args, 0, num_args);
					const char *p = transform_str_maps[i].value;
					if(!pv_read_args_from_str(args, 2, &p)){
						pv_warning("'%s','%s'", transform_str_maps[i].key, transform_str_maps[i].value);
					}else{
						conf->appearances[PvAppearanceKind_Translate].kind = PvAppearanceKind_Translate;
						PvAppearanceTranslateData *translate
							= &(conf->appearances[PvAppearanceKind_Translate].translate);
						translate->move = pv_point_add(translate->move, (PvPoint){args[0], args[1]});

						pv_debug("translate:'%s',(%f,%f),(%f,%f)",
								transform_str_maps[i].key, args[0], args[1],
								translate->move.x, translate->move.y);
					}
				}else{
					pv_warning("unknown key: '%s'(%d)'%s':'%s'(%d)",
							(char *)value, xmlnode->line,
							transform_str_maps[i].key, transform_str_maps[i].value, i);
					if(conf->imageFileReadOption->is_strict){
						pv_error("strict");
						return NULL;
					}
				}
			}
			pv_str_maps_free(transform_str_maps);
		}
		xmlFree(value);
	}

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

static PvElement *_pv_svg_path_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		ConfReadSvg *conf
		)
{
	PvElement *element_new = pv_element_new(PvElementKind_Curve);
	if(NULL == element_new){
		pv_error("");
		return NULL;
	}

	xmlAttr* attribute = xmlnode->properties;
	while(attribute){
		const PvSvgAttributeInfo *info = pv_svg_get_svg_attribute_info_from_name((const char *)attribute->name);

		if(info){
			bool ret = info->pv_svg_attribute_func_set(element_new, xmlnode, attribute);
			if(!ret){
				pv_warning("'%s'(%d) on '%s'",
						attribute->name, xmlnode->line, xmlnode->name);
			}
		}else{
			pv_warning("Not implement:'%s'(%d) on '%s'",
					attribute->name, xmlnode->line, xmlnode->name);
			if(conf->imageFileReadOption->is_strict){
				pv_error("strict");
				goto failed;
			}
		}


		attribute = attribute->next;
	}

	// pv_element_debug_print(element_new);

	const PvElementInfo *info = pv_element_get_info_from_kind(PvElementKind_Curve);
	pv_assert(info);
	PvAppearance *a[2] = {NULL, NULL};
	a[0] = &(conf->appearances[1]);
	info->func_apply_appearances(element_new, a);

	if(!pv_element_append_child(element_parent, NULL, element_new)){
		pv_error("");
		goto failed;
	}

	return element_new;

failed:
	pv_element_free(element_new);

	return NULL;
}

static PvElement *_pv_svg_polygon_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		ConfReadSvg *conf
		)
{
	PvElement *element = _pv_svg_path_new_element_from_svg(
			element_parent,
			xmlnode,
			isDoChild,
			data,
			conf
			);

	if(element){
		pv_element_curve_set_close_anchor_point(element, true);
	}

	return element;
}

static PvElement *_pv_svg_polyline_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		ConfReadSvg *conf
		)
{
	PvElement *element = _pv_svg_path_new_element_from_svg(
			element_parent,
			xmlnode,
			isDoChild,
			data,
			conf
			);

	if(element){
		pv_element_curve_set_close_anchor_point(element, false);
	}

	return element;
}

static PvElement *_pv_svg_line_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		ConfReadSvg *conf
		)
{
	PvElement *element = _pv_svg_path_new_element_from_svg(
			element_parent,
			xmlnode,
			isDoChild,
			data,
			conf
			);

	if(element){
		pv_element_curve_set_close_anchor_point(element, false);
	}

	return element;
}

static PvElement *_pv_svg_text_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		ConfReadSvg *conf
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
		ConfReadSvg *conf
		)
{
	pv_warning("Not implement:'%s'(%d)", xmlnode->name, xmlnode->line);

	if(conf->imageFileReadOption->is_strict){
		pv_error("strict");
		return NULL;
	}

	return element_parent;
}

const PvSvgElementInfo _pv_svg_element_infos[] = {
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
		.tagname = "polygon",
		.func_new_element_from_svg = _pv_svg_polygon_new_element_from_svg,
	},
	{
		.tagname = "polyline",
		.func_new_element_from_svg = _pv_svg_polyline_new_element_from_svg,
	},
	{
		.tagname = "line",
		.func_new_element_from_svg = _pv_svg_line_new_element_from_svg,
	},
	{
		.tagname = "text",
		.func_new_element_from_svg = _pv_svg_text_new_element_from_svg,
	},
	/* Unknown (or not implement) tag */
	{
		.tagname = "==tag-unknown==",
		.func_new_element_from_svg = _pv_svg_unknown_new_element_from_svg,
	},
};


const PvSvgElementInfo *pv_svg_get_svg_element_info_from_tagname(const char *tagname)
{
	int num = sizeof(_pv_svg_element_infos) / sizeof(_pv_svg_element_infos[0]);
	for(int i = 0; i < num; i++){
		const PvSvgElementInfo *info = &_pv_svg_element_infos[i];
		if(0 == strcasecmp(tagname, info->tagname)){
			return info;
		}
	}

	return &_pv_svg_element_infos[num - 1];
}

