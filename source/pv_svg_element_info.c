#include "pv_svg_element_info.h"

#include <string.h>
#include <strings.h>
#include "pv_error.h"
#include "pv_io_util.h"
#include "pv_element_info.h"

static PvElement *_pv_svg_svg_new_element_from_svg(
		PvElement *element_parent,
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	return element_parent;
}

static bool func_nop_set_attribute_cache_(
		PvElement *element,
		const PvSvgAttributeCache *attribute_cache
		)
{
	pv_assert(element);
	pv_assert(attribute_cache);

	// pv_warning("not implement.");

	return true;
}

static PvElement *_pv_svg_g_new_element_from_svg(
		PvElement *element_parent,
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	PvElement *element_new = pv_element_new(PvElementKind_Group);
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
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	PvElement *element_new = pv_element_new(PvElementKind_Curve);
	if(NULL == element_new){
		pv_error("");
		return NULL;
	}

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
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	PvElement *element = _pv_svg_path_new_element_from_svg(
			element_parent,
			attribute_cache,
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
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	PvElement *element = _pv_svg_path_new_element_from_svg(
			element_parent,
			attribute_cache,
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
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	PvElement *element = _pv_svg_path_new_element_from_svg(
			element_parent,
			attribute_cache,
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

static PvElement *_pv_svg_rect_new_element_from_svg(
		PvElement *element_parent,
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	PvElement *element_new = pv_element_basic_shape_new_from_kind(PvBasicShapeKind_Rect);
	pv_assert(element_new);

	if(!pv_element_append_child(element_parent, NULL, element_new)){
		pv_error("");
		goto failed;
	}

	return element_new;

failed:
	pv_element_free(element_new);

	return NULL;
}

static PvElement *_pv_svg_image_new_element_from_svg(
		PvElement *element_parent,
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	PvElement *element_new = pv_element_new(PvElementKind_BasicShape);
	if(NULL == element_new){
		pv_error("");
		return NULL;
	}

	if(!pv_element_append_child(element_parent, NULL, element_new)){
		pv_error("");
		goto failed;
	}

	return element_new;

failed:
	pv_element_free(element_new);

	return NULL;
}

static bool func_image_set_attribute_cache_(
		PvElement *element,
		const PvSvgAttributeCache *attribute_cache
		)
{
	pv_assert(element);
	pv_assert(attribute_cache);

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assert(info);

	bool is_run = false;
	PvRect rect = info->func_get_rect_by_anchor_points(element);
	if(attribute_cache->attributes[PvSvgAttributeKind_x].is_exist){
		is_run = true;
		rect.x = attribute_cache->attributes[PvSvgAttributeKind_x].value;
	}
	if(attribute_cache->attributes[PvSvgAttributeKind_y].is_exist){
		is_run = true;
		rect.y = attribute_cache->attributes[PvSvgAttributeKind_y].value;
	}
	if(attribute_cache->attributes[PvSvgAttributeKind_width].is_exist){
		is_run = true;
		rect.w = attribute_cache->attributes[PvSvgAttributeKind_width].value;
	}
	if(attribute_cache->attributes[PvSvgAttributeKind_height].is_exist){
		is_run = true;
		rect.h = attribute_cache->attributes[PvSvgAttributeKind_height].value;
	}

	bool res = true;
	if(is_run){
		res = info->func_set_rect_by_anchor_points(element, rect);
		if(!res){
			pv_debug("#### %f %f %f %f", rect.x, rect.y, rect.w, rect.h);
		}
	}

	return res;
}

static PvElement *_pv_svg_text_new_element_from_svg(
		PvElement *element_parent,
		PvSvgAttributeCache *attribute_cache,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		PvSvgReadConf *conf
		)
{
	// nop
	return element_parent;
}


const PvSvgElementInfo _pv_svg_element_infos[] = {
	{
		.tagname = "svg",
		.func_new_element_from_svg	= _pv_svg_svg_new_element_from_svg,
		.func_set_attribute_cache	= func_nop_set_attribute_cache_,
	},
	{
		.tagname = "g",
		.func_new_element_from_svg	= _pv_svg_g_new_element_from_svg,
		.func_set_attribute_cache	= func_nop_set_attribute_cache_,
	},
	{
		.tagname = "path",
		.func_new_element_from_svg	= _pv_svg_path_new_element_from_svg,
		.func_set_attribute_cache	= func_nop_set_attribute_cache_,
	},
	{
		.tagname = "polygon",
		.func_new_element_from_svg	= _pv_svg_polygon_new_element_from_svg,
		.func_set_attribute_cache	= func_nop_set_attribute_cache_,
	},
	{
		.tagname = "polyline",
		.func_new_element_from_svg	= _pv_svg_polyline_new_element_from_svg,
		.func_set_attribute_cache	= func_nop_set_attribute_cache_,
	},
	{
		.tagname = "line",
		.func_new_element_from_svg	= _pv_svg_line_new_element_from_svg,
		.func_set_attribute_cache	= func_nop_set_attribute_cache_,
	},
	{
		.tagname = "rect",
		.func_new_element_from_svg	= _pv_svg_rect_new_element_from_svg,
		.func_set_attribute_cache	= func_image_set_attribute_cache_,
	},
	{
		.tagname = "image",
		.func_new_element_from_svg	= _pv_svg_image_new_element_from_svg,
		.func_set_attribute_cache	= func_image_set_attribute_cache_,
	},
	{
		.tagname = "text",
		.func_new_element_from_svg	= _pv_svg_text_new_element_from_svg,
		.func_set_attribute_cache	= func_nop_set_attribute_cache_,
	},
	{
		.tagname = "comment",
		.func_new_element_from_svg	= _pv_svg_text_new_element_from_svg,
		.func_set_attribute_cache	= func_nop_set_attribute_cache_,
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

	return NULL;
}

