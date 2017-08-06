/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_SVG_ELEMENT_INFO_H
#define include_PV_SVG_ELEMENT_INFO_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libxml/xmlwriter.h>
#include <stdbool.h>
#include "pv_element_type.h"
#include "pv_element.h"
#include "pv_appearance.h"
#include "pv_svg_attribute_info.h"

/** @brief
 * @return PvElement* element_current
 */
typedef PvElement* (*PvSvgFuncNewElementFromSvg)(
				PvElement *element_parent,
				PvSvgAttributeCache *attribute_cache,
				xmlNodePtr xmlnode,
				bool *isDoChild,
				gpointer data,
				PvSvgReadConf *conf
				);
typedef bool (*PvSvgElementFuncSetAttributeCache)(
				PvElement *element,
				const PvSvgAttributeCache *attribute_cache
				);

typedef struct{
	const char *tagname;
	PvSvgFuncNewElementFromSvg		func_new_element_from_svg;
	PvSvgElementFuncSetAttributeCache	func_set_attribute_cache;
}PvSvgElementInfo;

extern const PvSvgElementInfo _pv_svg_element_infos[];

const PvSvgElementInfo *pv_svg_get_svg_element_info_from_tagname(const char *tagname);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_SVG_ELEMENT_INFO_H

