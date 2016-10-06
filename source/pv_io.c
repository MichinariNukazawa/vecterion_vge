#include "pv_io.h"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <string.h>
#include "pv_error.h"
#include "pv_element_general.h"
#include "pv_element_infos.h"
#include "pv_svg_info.h"

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

static bool _pv_io_svg_from_pvvg_element_recurseve(
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
	if(!pv_element_recursive_desc(element_root,
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
	if(!_pv_io_svg_from_pvvg_element_recurseve(root_node, vg->element_root, &conf)){
		pv_error("");
		return false;
	}

	xmlSaveFormatFileEnc(path, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	return true;
}

static void print_element_names(xmlNode * a_node)
{
	xmlNode *cur_node = NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			printf("node type: Element, name: %s", cur_node->name);
		}

		print_element_names(cur_node->children);
	}
}

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

PvColor _pv_io_get_pv_color_from_svg_str_rgba(const char *str)
{
	PvColor color = PvColor_None;

	int ret;
	unsigned int r, g, b;
	double a;
	if(4 != (ret = sscanf(str, " rgba(%3u,%3u,%3u,%lf)", &r, &g, &b, &a))){
		pv_warning("%d '%s'", ret, str);
		goto error;
	}
	pv_debug(" rgba(%u,%u,%u,%f)", r, g, b, a);
	bool ok = (pv_color_set_parameter(&color, PvColorParameterIx_R, (double)r)
		&& pv_color_set_parameter(&color, PvColorParameterIx_G, (double)g)
		&& pv_color_set_parameter(&color, PvColorParameterIx_B, (double)b)
		&& pv_color_set_parameter(&color, PvColorParameterIx_O, (double)a * 100.0)
		);
	if(!ok){
		pv_warning("'%s'", str);
		goto error;
	}

error:
	return color;
}

PvColorPair _pv_io_get_pv_color_pair_from_xmlnode_simple(const xmlNode *xmlnode)
{
	PvColorPair color_pair = PvColorPair_None;
	xmlChar *xc_fill = xmlGetProp(xmlnode, BAD_CAST "fill");
	if(NULL != xc_fill){
		color_pair.colors[PvColorPairGround_BackGround]
			= _pv_io_get_pv_color_from_svg_str_rgba((char *)xc_fill);
	}
	xmlFree(xc_fill);

	xmlChar *xc_stroke = xmlGetProp(xmlnode, BAD_CAST "stroke");
	if(NULL != xc_stroke){
		color_pair.colors[PvColorPairGround_ForGround]
			= _pv_io_get_pv_color_from_svg_str_rgba((char *)xc_stroke);
	}
	xmlFree(xc_stroke);

	return color_pair;
}

static bool _pv_io_element_from_svg_in_recursive_inline(PvElement *element_parent,
		xmlNode *xmlnode,
		gpointer data,
		const ConfReadSvg *conf)
{
	const PvSvgInfo *svg_info = pv_svg_get_svg_info_from_tagname((char *)xmlnode->name);
	if(NULL == svg_info){
		pv_error("");
		goto error;
	}
	if(NULL == svg_info->func_new_element_from_svg){
		pv_error("");
		goto error;
	}

	bool isDoChild = true;
	PvElement *element_current = svg_info->func_new_element_from_svg(
			element_parent, xmlnode, &isDoChild, data, conf);
	if(NULL == element_current){
		pv_error("");
		goto error;
	}

	element_current->color_pair = _pv_io_get_pv_color_pair_from_xmlnode_simple(xmlnode);

	if(isDoChild){
		for (xmlNode *cur_node = xmlnode->children; cur_node; cur_node = cur_node->next) {
			if(!_pv_io_element_from_svg_in_recursive_inline(element_current,
						cur_node,
						data,
						conf))
			{
				pv_error("");
				return false;
			}
		}
	}

	return true;
error:
	return false;
}

static bool _pv_io_pvvg_from_svg_element_recurseve(PvVg *vg,
		xmlNodePtr xml_svg, 
		const ConfReadSvg *conf)
{
	if(NULL == vg){
		pv_bug("");
		return true;
	}
	if(NULL == xml_svg){
		pv_bug("");
		return false;
	}
	if(NULL == conf){
		pv_bug("");
		return false;
	}

	if(!_pv_io_element_from_svg_in_recursive_inline(
				vg->element_root,
				xml_svg,
				NULL,
				conf))
	{
		pv_error("");
		return false;
	}

	return true;
}

PvVg *pv_io_new_from_file(const char *filepath)
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
	if(!_pv_io_set_vg_from_xmlnode_svg(vg, xmlnode_svg)){
		pv_error("");
		goto error;
	}
	ConfReadSvg conf;
	if(!_pv_io_pvvg_from_svg_element_recurseve(vg, xmlnode_svg, &conf)){
		pv_error("");
		return false;
	}
	print_element_names(xmlnode_svg);

	// remove default layer, when after append element from raster image file.
	assert(2 == pv_general_get_parray_num((void **)(vg->element_root->childs)));
	assert(pv_element_remove_delete_recursive(vg->element_root->childs[0]));
	assert(1 == pv_general_get_parray_num((void **)(vg->element_root->childs)));


	xmlFreeDoc(xml_doc);
	xmlCleanupParser();
	return vg;

error:
	xmlFreeDoc(xml_doc);
	xmlCleanupParser();
	pv_vg_free(vg);

	return NULL;
}

