/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_SVG_ATTRIBUTE_INFO_H
#define include_PV_SVG_ATTRIBUTE_INFO_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libxml/xmlwriter.h>
#include <stdbool.h>
#include "pv_element.h"
#include "pv_image_file_read_option.h"

typedef struct{
	double stroke_width;
	PvColorPair color_pair;

	PvAppearance appearances[5];
	const PvImageFileReadOption *imageFileReadOption;
}PvSvgReadConf;

static const PvSvgReadConf PvSvgReadConf_Default = {
	1.0,
	{ {{{0, 0, 0, 100,}}, {{255, 255, 255, 100,}},}, },
	{},
	&PvImageFileReadOption_Default,
};

typedef enum{
	PvSvgAttributeKind_x,
	PvSvgAttributeKind_y,
	PvSvgAttributeKind_width,
	PvSvgAttributeKind_height,
	PvSvgAttributeKind_xlink_href,
	PvSvgAttributeKind_NUM,
}PvSvgAttributeKind;

typedef struct{
	bool is_exist;
	double value;
}PvSvgAttributeItem;

typedef struct{
	PvSvgAttributeItem attributes[PvSvgAttributeKind_NUM];
}PvSvgAttributeCache;

void pv_svg_attribute_cache_init(PvSvgAttributeCache *);

typedef bool (*PvSvgAttributeFuncSet)(
		PvElement *element,
		PvSvgAttributeCache *attributeCache,
		PvSvgReadConf *conf,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		);

typedef struct{
	const char *name;
	PvSvgAttributeFuncSet pv_svg_attribute_func_set;
}PvSvgAttributeInfo;

extern const PvSvgAttributeInfo _pv_svg_attribute_infos[];

const PvSvgAttributeInfo *pv_svg_get_svg_attribute_info_from_name(const char *name);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_SVG_ATTRIBUTE_INFO_H

