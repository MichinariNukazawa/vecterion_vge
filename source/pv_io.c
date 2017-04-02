#include "pv_io.h"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "pv_error.h"
#include "pv_element_general.h"
#include "pv_element_info.h"
#include "pv_svg_element_info.h"
#include "pv_io_util.h"

typedef struct{
	InfoTargetSvg *target;
	const ConfWriteSvg *conf;
}PvIoSvgRecursiveData;



static bool _pv_io_svg_from_element_in_recursive_before(
		PvElement *element, gpointer data, int level)
{
	PvIoSvgRecursiveData *_data = data;
	InfoTargetSvg *target = _data->target;
	const ConfWriteSvg *conf = _data->conf;

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	if(NULL == info){
		pv_error("");
		return false;
	}
	if(NULL == info->func_write_svg){
		pv_error("");
		return false;
	}

	// push parent node stack
	xmlNodePtr *new_nodes = realloc(target->xml_parent_nodes,
			(sizeof(xmlNodePtr) * (level + 2)));
	if(NULL == new_nodes){
		pv_critical("");
		exit(-1);
	}
	new_nodes[level+1] = NULL;
	new_nodes[level] = target->xml_parent_node;
	target->xml_parent_nodes = new_nodes;

	int ret = info->func_write_svg(target, element, conf);
	if(0 > ret){
		pv_error("%d", ret);
		return false;
	}

	return true;
}

static bool _pv_io_svg_from_element_in_recursive_after(
		PvElement *element, gpointer data, int level)
{
	PvIoSvgRecursiveData *_data = data;
	InfoTargetSvg *target = _data->target;
	// const ConfWriteSvg *conf = _data->conf;

	// pop parent node stack
	int num = pv_general_get_parray_num((void **)target->xml_parent_nodes);
	if(!(level < num)){
		pv_bug("");
		return false;
	}
	target->xml_parent_node = target->xml_parent_nodes[level];

	return true;
}

static bool _pv_io_svg_from_pvvg_element_recursive(
		xmlNodePtr xml_svg, PvElement *element_root,
		const ConfWriteSvg *conf)
{
	if(NULL == xml_svg){
		pv_bug("");
		return false;
	}

	if(NULL == element_root){
		pv_warning("");
		return true;
	}

	if(NULL == conf){
		pv_bug("");
		return false;
	}


	InfoTargetSvg target = {
		.xml_parent_nodes = NULL,
		.xml_parent_node = xml_svg,
		.xml_new_node = NULL,
	};
	PvIoSvgRecursiveData data = {
		.target = &target,
		.conf = conf,
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_asc(element_root,
				_pv_io_svg_from_element_in_recursive_before,
				_pv_io_svg_from_element_in_recursive_after,
				&data, &error)){
		pv_error("level:%d", error.level);
		return false;
	}
	free(target.xml_parent_nodes);

	return true;
}

bool pv_io_write_file_svg_from_vg(PvVg *vg, const char *path)
{
	if(NULL == vg){
		pv_bug("");
		return false;
	}

	if(NULL == path){
		pv_bug("");
		return false;
	}

	char *p = NULL;

	xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "svg");
	if(NULL == doc || NULL == root_node){
		pv_error("");
		return false;
	}
	xmlNewProp(root_node, BAD_CAST "xmlns",
			BAD_CAST "http://www.w3.org/2000/svg");
	xmlNewProp(root_node, BAD_CAST "xmlns:svg",
			BAD_CAST "http://www.w3.org/2000/svg");
	xmlNewProp(root_node, BAD_CAST "xmlns:inkscape",
			BAD_CAST "http://www.inkscape.org/namespaces/inkscape");
	xmlNewProp(root_node, BAD_CAST "xmlns:xlink",
			BAD_CAST "http://www.w3.org/1999/xlink");
	xmlDocSetRootElement(doc, root_node);

	// ** width, height
	pv_debug("x:%f y:%f w:%f h:%f",
			(vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);

	xmlNewProp(root_node, BAD_CAST "version", BAD_CAST "1.1");

	p = g_strdup_printf("%f", (vg->rect).x);
	xmlNewProp(root_node, BAD_CAST "x", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f", (vg->rect).y);
	xmlNewProp(root_node, BAD_CAST "y", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f", (vg->rect).w);
	xmlNewProp(root_node, BAD_CAST "width", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f", (vg->rect).h);
	xmlNewProp(root_node, BAD_CAST "height", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f %f %f %f",
			(vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);
	xmlNewProp(root_node, BAD_CAST "viewBox", BAD_CAST p);
	g_free(p);

	//		vg->element_root;
	ConfWriteSvg conf;
	if(!_pv_io_svg_from_pvvg_element_recursive(root_node, vg->element_root, &conf)){
		pv_error("");
		return false;
	}

	xmlSaveFormatFileEnc(path, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	return true;
}

/*
   static void print_element_names(xmlNode * a_node)
   {
   xmlNode *cur_node = NULL;

   for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
   if (cur_node->type == XML_ELEMENT_NODE) {
   pv_debug("node type: Element, name: %s(%d)", cur_node->name, cur_node->line);
   }

   print_element_names(cur_node->children);
   }
   }
 */

static bool _pv_io_get_svg_from_xml(xmlNode **xmlnode_svg, xmlNode *xmlnode)
{
	*xmlnode_svg = NULL;

	for (xmlNode *cur_node = xmlnode; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			if(0 == strcmp("svg", (char*)cur_node->name)){
				*xmlnode_svg = cur_node;
				return true;
			}
		}
		if(_pv_io_get_svg_from_xml(xmlnode_svg, cur_node->children)){
			return true;
		}
	}

	return false;
}

static bool _pv_io_get_px_from_str(double *value, const char *str, const char **str_error)
{
	char *endptr = NULL;
	if(!pv_general_strtod(value, str, &endptr, str_error)){
		return false;
	}

	// convert to px
	double dpi = 90.0;
	if(NULL == endptr){
		*str_error = "Internal error.";
		return false;
	}

	if(0 == strcmp("", endptr)){
		// return true;
	}else if(0 == strcmp("px", endptr)){
		// return true;
	}else if(0 == strcmp("pt", endptr)){
		*value *= 1.25 * (dpi / 90.0);
	}else if(0 == strcmp("pc", endptr)){
		*value *= 15 * (dpi / 90.0);
	}else if(0 == strcmp("mm", endptr)){
		*value *= 3.543307 * (dpi / 90.0);
	}else if(0 == strcmp("cm", endptr)){
		*value *= 35.43307 * (dpi / 90.0);
	}else if(0 == strcmp("in", endptr)){
		*value *= 90.0 * (dpi / 90.0);
	}else{
		*str_error = "Unit undefined.";
		pv_error("%s'%s'", *str_error, endptr);
		return false;
	}

	return true;
}

static bool _pv_io_set_vg_from_xmlnode_svg(PvVg *vg, xmlNode *xmlnode_svg)
{
	pv_debug("");
	PvRect rect = {0, 0, -1, -1};
	xmlAttr* attribute = xmlnode_svg->properties;
	while(attribute)
	{
		bool isOk = false;
		xmlChar* xmlValue = xmlNodeListGetString(xmlnode_svg->doc,
				attribute->children, 1);
		const char *strValue = (char*)xmlValue;
		const char *name = (char*)attribute->name;
		const char *str_error = "Not process.";
		if(0 == strcmp("x", name)){
			double value = 0.0;
			if(_pv_io_get_px_from_str(&value, strValue, &str_error)){
				rect.x = value;
				isOk = true;
			}
		}else if(0 == strcmp("y", name)){
			double value = 0.0;
			if(_pv_io_get_px_from_str(&value, strValue, &str_error)){
				rect.y = value;
				isOk = true;
			}
		}else if(0 == strcmp("width", name)){
			double value = 0.0;
			if(_pv_io_get_px_from_str(&value, strValue, &str_error)){
				rect.w = value;
				isOk = true;
			}
		}else if(0 == strcmp("height", name)){
			double value = 0.0;
			if(_pv_io_get_px_from_str(&value, strValue, &str_error)){
				rect.h = value;
				isOk = true;
			}
		}

		if(!isOk){
			pv_debug("Can not use:'%s':'%s' %s",
					name, strValue, str_error);
		}

		xmlFree(xmlValue); 
		attribute = attribute->next;
	}

	if(rect.w <= 0 && rect.h <= 0){
		rect.w = rect.h = 1;
	}else if(rect.w <= 0){
		rect.w = rect.h; // TODO: svg spec is "100%"
	}else if(rect.h <= 0){
		rect.h = rect.w; // TODO: svg spec is "100%"
	}

	vg->rect = rect;

	return true;
}

static bool set_attribute_cache_(
		PvElement *element,
		PvSvgReadConf *conf,
		const PvSvgAttributeCache *attribute_cache)
{
	if(attribute_cache->attributes[PvSvgAttributeKind_fill].is_exist){
		element->color_pair.colors[PvColorPairGround_BackGround]
			= attribute_cache->attributes[PvSvgAttributeKind_fill].color;
		conf->color_pair.colors[PvColorPairGround_BackGround]
			= attribute_cache->attributes[PvSvgAttributeKind_fill].color;
	}
	if(attribute_cache->attributes[PvSvgAttributeKind_stroke].is_exist){
		element->color_pair.colors[PvColorPairGround_ForGround]
			= attribute_cache->attributes[PvSvgAttributeKind_stroke].color;
		conf->color_pair.colors[PvColorPairGround_ForGround]
			= attribute_cache->attributes[PvSvgAttributeKind_stroke].color;
	}
	if(attribute_cache->attributes[PvSvgAttributeKind_stroke_width].is_exist){
		element->stroke.width
			= attribute_cache->attributes[PvSvgAttributeKind_stroke_width].value;
		conf->stroke_width
			= attribute_cache->attributes[PvSvgAttributeKind_stroke_width].value;
	}
	if(attribute_cache->attributes[PvSvgAttributeKind_fill_opacity].is_exist){
		PvColor *color = NULL;
		double opacity = attribute_cache->attributes[PvSvgAttributeKind_fill_opacity].value;

		color = &(element->color_pair.colors[PvColorPairGround_BackGround]);
		pv_color_set_parameter(color, PvColorParameterIx_O, opacity * 100);

		color = &(conf->color_pair.colors[PvColorPairGround_BackGround]);
		pv_color_set_parameter(color, PvColorParameterIx_O, opacity * 100);
	}
	if(attribute_cache->attributes[PvSvgAttributeKind_stroke_opacity].is_exist){
		PvColor *color = NULL;
		double opacity = attribute_cache->attributes[PvSvgAttributeKind_stroke_opacity].value;

		color = &(element->color_pair.colors[PvColorPairGround_ForGround]);
		pv_color_set_parameter(color, PvColorParameterIx_O, opacity * 100);

		color = &(conf->color_pair.colors[PvColorPairGround_ForGround]);
		pv_color_set_parameter(color, PvColorParameterIx_O, opacity * 100);
	}

	return true;
}

static bool _new_elements_from_svg_elements_recursive_inline(PvElement *element_parent,
		xmlNode *xmlnode,
		gpointer data,
		PvSvgReadConf *conf)
{
	PvSvgReadConf conf_save = *conf;

	const PvSvgElementInfo *svg_element_info = pv_svg_get_svg_element_info_from_tagname((char *)xmlnode->name);
	if(NULL == svg_element_info){
		pv_warning("Not implement:'%s'(%d)",
				xmlnode->name, xmlnode->line);
		if(0 == strcmp("comment", (char *)xmlnode->name)){
			return true;
		}
		if(conf->imageFileReadOption->is_strict){
			goto failed0;
		}
		goto skiped;
	}
	pv_assertf(svg_element_info->func_new_element_from_svg, "'%s'", (char *)xmlnode->name);

	PvSvgAttributeCache attribute_cache_;
	PvSvgAttributeCache *attribute_cache = &attribute_cache_;
	pv_svg_attribute_cache_init(attribute_cache);

	bool isDoChild = true;
	PvElement *element_current = svg_element_info->func_new_element_from_svg(
			element_parent,
			attribute_cache,
			xmlnode,
			&isDoChild,
			data,
			conf);
	if(NULL == element_current){
		pv_error("");
		goto failed0;
	}

	element_current->color_pair = conf->color_pair;
	element_current->stroke.width = conf->stroke_width;

	{
		xmlAttr* attribute = xmlnode->properties;
		while(attribute){
			const PvSvgAttributeInfo *info = pv_svg_get_svg_attribute_info_from_name((const char *)attribute->name);

			if(info){
				xmlChar *value = xmlGetProp(xmlnode, BAD_CAST attribute->name);
				pv_assertf(value, "'%s'(%d) on '%s'",
						attribute->name, xmlnode->line, xmlnode->name);

				bool ret = info->pv_svg_attribute_func_set(
						element_current, attribute_cache, conf,
						(const char *)value,
						xmlnode, attribute);
				if(!ret){
					pv_warning("'%s'(%d) on '%s'",
							attribute->name, xmlnode->line, xmlnode->name);
					if(conf->imageFileReadOption->is_strict){
						pv_error("strict");
						goto failed1;
					}
				}

				xmlFree(value);
			}else{
				pv_warning("Not implement:'%s'(%d) on '%s'",
						attribute->name, xmlnode->line, xmlnode->name);
				if(conf->imageFileReadOption->is_strict){
					pv_error("strict");
					goto failed1;
				}
			}

			attribute = attribute->next;
		}
	}

	if(PvElementKind_BasicShape == element_current->kind){
		PvElementBasicShapeData *element_data = element_current->data;
		if(PvBasicShapeKind_Raster == element_data->kind){
			if(!attribute_cache->attributes[PvSvgAttributeKind_xlink_href].is_exist){
				if(conf->imageFileReadOption->is_strict){
					pv_error("strict");
					goto failed1;
				}
				pv_warning("remove empty image element by vecterion.");
				pv_element_remove_free_recursive(element_current);
				goto skiped;
			}
		}
	}

/*
	if(0 == strcmp("g", svg_element_info->tagname)){
		 conf->color_pair = PvColorPair_SvgDefault;
		 conf->stroke_width = 1.0;
	}
*/

	bool ret = svg_element_info->func_set_attribute_cache(element_current, attribute_cache);
	if(!ret){
		pv_warning("%d %s(%d)", ret, (char *)xmlnode->name, xmlnode->line);
		if(conf->imageFileReadOption->is_strict){
			pv_error("strict");
			goto failed1;
		}
	}
	if(!set_attribute_cache_(element_current, conf, attribute_cache)){
		if(conf->imageFileReadOption->is_strict){
			pv_error("strict");
			goto failed1;
		}
	}

	//! none color temporary implement.
	{
		double o;
		PvColor *color = NULL;
		color = &(element_current->color_pair.colors[PvColorPairGround_BackGround]);
		o = pv_color_get_parameter(color, PvColorParameterIx_O);
		o *= color->none_weight;
		pv_color_set_parameter(color, PvColorParameterIx_O, o);

		color = &(element_current->color_pair.colors[PvColorPairGround_ForGround]);
		o = pv_color_get_parameter(color, PvColorParameterIx_O);
		o *= color->none_weight;
		pv_color_set_parameter(color, PvColorParameterIx_O, o);
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element_current->kind);
	pv_assert(info);
	PvAppearance *a[3] = {NULL, NULL, NULL,};
	a[0] = &(conf->appearances[PvAppearanceKind_Translate]);
	a[1] = &(conf->appearances[PvAppearanceKind_Resize]);
	info->func_apply_appearances(element_current, a);

	if(isDoChild){
		for (xmlNode *cur_node = xmlnode->children; cur_node; cur_node = cur_node->next) {
			if(!_new_elements_from_svg_elements_recursive_inline(element_current,
						cur_node,
						data,
						conf))
			{
				pv_error("");
				goto failed1;
			}
		}
	}

skiped:
	*conf = conf_save;

	return true;
failed1:
	pv_element_remove_free_recursive(element_current);
failed0:

	pv_debug("fail:'%s'(%d)",
			xmlnode->name, xmlnode->line);
	return false;
}

static bool _new_elements_from_svg_elements_recursive(
		PvElement *parent_element,
		xmlNodePtr xml_svg, 
		PvSvgReadConf *conf)
{
	pv_assert(parent_element);
	pv_assert(xml_svg);
	pv_assert(conf);

	if(!_new_elements_from_svg_elements_recursive_inline(
				parent_element,
				xml_svg,
				NULL,
				conf))
	{
		pv_error("");
		return false;
	}

	return true;
}

static PvElement *pv_io_new_element_from_filepath_with_vg_(
		PvVg *vg,
		const char *filepath,
		const PvImageFileReadOption *imageFileReadOption)
{
	PvElement *top_layer = NULL;

	LIBXML_TEST_VERSION

		xmlDoc *xml_doc = xmlReadFile(filepath, NULL, 0);
	if(NULL == xml_doc){
		pv_error("");
		goto error;
	}
	xmlNode *xml_root_element = xmlDocGetRootElement(xml_doc);
	xmlNode *xmlnode_svg = NULL;
	if(!_pv_io_get_svg_from_xml(&xmlnode_svg, xml_root_element)){
		pv_error("");
		goto error;
	}

	if(NULL != vg){
		if(!_pv_io_set_vg_from_xmlnode_svg(vg, xmlnode_svg)){
			pv_error("");
			goto error;
		}
	}

	PvElement *layer = pv_element_new(PvElementKind_Layer);
	pv_assert(layer);
	PvSvgReadConf conf = PvSvgReadConf_Default;
	conf.imageFileReadOption = imageFileReadOption;
	if(!_new_elements_from_svg_elements_recursive(layer, xmlnode_svg, &conf)){
		pv_error("");
		goto error;
	}

	int num_svg_top = pv_general_get_parray_num((void **)layer->childs);
	if(1 == num_svg_top && PvElementKind_Layer == layer->childs[0]->kind){
		// cut self root layer if exist root layer in svg
		top_layer = layer->childs[0];
		top_layer->parent = NULL;
		pv_element_free(layer);
	}else{
		top_layer = layer;
	}

error:
	xmlFreeDoc(xml_doc);
	xmlCleanupParser();

	return top_layer;
}

PvVg *pv_io_new_from_file(const char *filepath, const PvImageFileReadOption *imageFileReadOption)
{
	if(NULL == filepath){
		pv_error("");
		return NULL;
	}

	PvVg *vg = pv_vg_new();
	if(NULL == vg){
		pv_error("");
		return NULL;
	}

	PvElement *layer = pv_io_new_element_from_filepath_with_vg_(vg, filepath, imageFileReadOption);
	if(NULL == layer){
		pv_warning("");
		goto error;
	}

	bool is_toplevel_layer_all = false;
	int num_svg_top = pv_general_get_parray_num((void **)layer->childs);
	for(int i = 0; i < num_svg_top; i++){
		is_toplevel_layer_all = true;
		if(PvElementKind_Layer != layer->childs[i]->kind){
			is_toplevel_layer_all = false;
			break;
		}
	}
	if(is_toplevel_layer_all){
		// castle root layer
		PvElement *_root = vg->element_root;
		layer->kind = PvElementKind_Root;
		vg->element_root = layer;
		pv_assert(pv_element_remove_free_recursive(_root));
	}else{
		// append root child
		pv_assert(pv_element_append_child(vg->element_root, NULL, layer));
		pv_assert(pv_element_remove_free_recursive(vg->element_root->childs[0]));
		assert(1 == pv_general_get_parray_num((void **)(vg->element_root->childs)));
	}

	return vg;

error:
	pv_vg_free(vg);

	return NULL;
}

PvElement *pv_io_new_element_from_filepath(const char *filepath)
{
	return pv_io_new_element_from_filepath_with_vg_(NULL, filepath, &PvImageFileReadOption_Default);
}

