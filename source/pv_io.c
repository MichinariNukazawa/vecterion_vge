#include "pv_io.h"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include "pv_error.h"
#include "pv_element_general.h"
#include "pv_element_infos.h"

typedef struct{
	InfoTargetSvg *target;
	const ConfWriteSvg *conf;
}PvIoSvgRecursiveData;

bool _pv_io_svg_from_element_in_recursive_before(PvElement *element, gpointer data, int level)
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
		pv_error("%d\n", ret);
		return false;
	}

	return true;
}

bool _pv_io_svg_from_element_in_recursive_after(PvElement *element, gpointer data, int level)
{
	PvIoSvgRecursiveData *_data = data;
	InfoTargetSvg *target = _data->target;
	const ConfWriteSvg *conf = _data->conf;

	// pop parent node stack
	int num = pv_general_get_parray_num((void **)target->xml_parent_nodes);
	if(!(level < num)){
		pv_bug("");
		return false;
	}
	target->xml_parent_node = target->xml_parent_nodes[level];

	return true;
}

bool _pv_io_svg_from_pvvg_element_recurseve(xmlNodePtr xml_svg, PvElement *element_root,
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
	if(!pv_element_recursive(element_root,
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
	xmlDocPtr doc = NULL;	   /* document pointer */
	xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;/* node pointers */
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "svg");
	xmlNewProp(root_node, BAD_CAST "xmlns",
			BAD_CAST "http://www.w3.org/2000/svg");
	xmlNewProp(root_node, BAD_CAST "xmlns:svg",
			BAD_CAST "http://www.w3.org/2000/svg");
	xmlNewProp(root_node, BAD_CAST "xmlns:inkscape",
			BAD_CAST "http://www.inkscape.org/namespaces/inkscape");
	xmlDocSetRootElement(doc, root_node);

	// ** width, height
	pv_debug("x:%f y:%f w:%f h:%f\n",
			(vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);

	xmlNewProp(root_node, BAD_CAST "version", BAD_CAST "1.1");
	g_free(p);

	p = g_strdup_printf("%f %f %f %f",
			(vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);
	xmlNewProp(root_node, BAD_CAST "viewBox", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f", (vg->rect).w);
	xmlNewProp(root_node, BAD_CAST "width", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f", (vg->rect).h);
	xmlNewProp(root_node, BAD_CAST "height", BAD_CAST p);
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

// PvVg *pv_io_new_vg_from_file(const char *path);

