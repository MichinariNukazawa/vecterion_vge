#ifndef include_PV_SVG_INFO_H
#define include_PV_SVG_INFO_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libxml/xmlwriter.h>
#include <stdbool.h>
#include "pv_element_general.h"
#include "pv_element.h"

typedef struct{
	int dummy;
}ConfReadSvg;

/** @brief
 * @return PvElement* element_current
 */
typedef PvElement* (*PvSvgFuncNewElementFromSvg)(
				PvElement *element_parent,
				xmlNodePtr xmlnode,
				bool *isDoChild,
				gpointer data,
				const ConfReadSvg *conf
				);

typedef struct{
	const char *tagname;
	PvSvgFuncNewElementFromSvg		func_new_element_from_svg;
}PvSvgInfo;

extern const PvSvgInfo _pv_svg_infos[];

const PvSvgInfo *pv_svg_get_svg_info_from_tagname(const char *tagname);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_SVG_INFO_H

