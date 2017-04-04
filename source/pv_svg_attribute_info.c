#include "pv_svg_attribute_info.h"

#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <fenv.h>
#include "pv_error.h"
#include "pv_element_info.h"
#include "pv_io_util.h"

void pv_svg_attribute_cache_init(PvSvgAttributeCache *attribute_cache)
{
	pv_assert(attribute_cache);

	const PvSvgAttributeItem PvSvgAttributeItem_Default = {
		false,
		0,
		{{0, 0, 0, 0, }, 1,},
	};
	for(int i = 0; i < (int)PvSvgAttributeKind_NUM; i++){
		attribute_cache->attributes[i] = PvSvgAttributeItem_Default;
	}
}

static bool func_nop_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	pv_warning("Not implement:'%s'(%d) in '%s'", (char *)attribute->name, xmlnode->line, (char *)xmlnode->name);

	return true;
}

static PvStrMap *new_transform_str_maps_from_str_(const char *src_str)
{
	const char *head = src_str;

	int num = 0;
	PvStrMap *map = NULL;
	map = realloc(map, sizeof(PvStrMap) * (num + 1));
	pv_assert(map);
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

static void pv_element_anchor_point_init(PvAnchorPoint *ap)
{
	*ap = PvAnchorPoint_Default;
}

static bool func_fill_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	PvColor color = conf->color_pair.colors[PvColorPairGround_BackGround];
	if(! pv_io_util_get_pv_color_from_svg_str_rgba(&color, (const char *)value)){
		pv_error("");
		return false;
	}

	attribute_cache->attributes[PvSvgAttributeKind_fill].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_fill].color = color;

	return true;
}

static bool func_stroke_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	PvColor color = conf->color_pair.colors[PvColorPairGround_ForGround];
	if(! pv_io_util_get_pv_color_from_svg_str_rgba(&color, (const char *)value)){
		pv_warning("");
		return false;
	}

	attribute_cache->attributes[PvSvgAttributeKind_stroke].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_stroke].color = color;

	return true;
}

static bool func_stroke_width_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	double width = pv_io_util_get_double_from_str((const char *)value);

	attribute_cache->attributes[PvSvgAttributeKind_stroke_width].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_stroke_width].value = width;

	return true;
}

static bool func_stroke_linecap_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	int num = get_num_stroke_linecap_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinecapInfo *info = get_stroke_linecap_info_from_id(i);
		if(0 == strcasecmp((char *)value, info->name)){
			element->stroke.linecap = info->linecap;
			return true;
		}
	}

	return true;
}

static bool func_stroke_linejoin_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	int num = get_num_stroke_linejoin_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinejoinInfo *info = get_stroke_linejoin_info_from_id(i);
		if(0 == strcasecmp((char *)value, info->name)){
			element->stroke.linejoin = info->linejoin;
			return true;
		}
	}

	return true;
}

static PvElement *_pv_element_to_recursively(PvElement *element)
{
	pv_assert(element);
	PvElement *child_element = (PvElement *)malloc(sizeof(PvElement));
	pv_assert(child_element);

	*child_element = *element;
	// child_element->kind = element->kind;
	// child_element->data = element->data;

	const PvElementInfo *info = pv_element_get_info_from_kind(PvElementKind_Group);
	pv_assertf(info, "%d", PvElementKind_Group);
	element->data = info->func_new_data();
	pv_assert(element->data);
	element->kind = PvElementKind_Group;

	pv_assert(pv_element_append_child(element, NULL, child_element));

	return child_element;
}

static PvElement *_pv_element_append_new(PvElement *parent, PvElement *sister, PvElementKind kind)
{
	PvElement *child_element = pv_element_new(kind);
	pv_assertf(child_element, "%d", kind);

	pv_assert(pv_element_append_child(parent, sister, child_element));

	return child_element;
}

static bool func_d_set_inline_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		const char *value
		)
{
	const int num_args = 10;
	double args[num_args];
	double prev_args[num_args];
	pv_double_array_fill(args, 0, num_args);
	pv_double_array_fill(prev_args, 0, num_args);

	PvAnchorPoint ap;
	pv_assert(element);
	pv_assert(element->data);
	pv_assert(PvElementKind_Curve == element->kind);

	PvElement *parent_element = NULL;
	PvPoint prev_point = PvPoint_Default;

	char command_prev = ' '; // intialize character is nothing prev command.
	const char *p = value;
	while('\0' != *p){
		bool is_append = false;

		char command = ' ';
		switch(*p){
			case 'M':
			case 'L':
			case 'm':
			case 'l':
			case 'H':
			case 'h':
			case 'V':
			case 'v':
			case 'C':
			case 'c':
			case 'S':
			case 's':
			case 'Z':
			case 'z':
				command = *p;
				p++;
				break;
			default:
				if(0 != isdigit(*p)){
					command = command_prev;
				}else if('-' == *p){ // minus number
					command = command_prev;
				}else{
					p++;
				}
				break;
		}

		if(' ' == command){
			continue;
		}

		if(' ' != command_prev){// not first command
			if('M' == command || 'm' == command){// M|m.
				if(NULL == parent_element){
					parent_element = element;
					element = _pv_element_to_recursively(element);
					pv_assert(element);
				}

				element = _pv_element_append_new(
						parent_element, element, PvElementKind_Curve);
				pv_assert(element);
			}
		}

		command_prev = command;

		PvElementCurveData *data = element->data;

		switch(command){
			case 'M':
			case 'L':
				if(!pv_read_args_from_str(args, 2, &p)){
					goto failed;
				}
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_Point].x = args[0];
				ap.points[PvAnchorPointIndex_Point].y = args[1];
				is_append = true;
				break;
			case 'm':
			case 'l':
				{
					if(!pv_read_args_from_str(args, 2, &p)){
						goto failed;
					}

					PvPoint point = PvPoint_Default;
					size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
					if(0 == num){
						point = prev_point;
					}else{
						const PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(
								data->anchor_path,
								(num - 1),
								PvAnchorPathIndexTurn_Disable);
						point = pv_anchor_point_get_point(ap_prev);
					}

					point.x += args[0];
					point.y += args[1];

					pv_element_anchor_point_init(&ap);
					ap.points[PvAnchorPointIndex_Point] = point;
					is_append = true;
				}
				break;
			case 'H':
			case 'h':
			case 'V':
			case 'v':
				{
					if(!pv_read_args_from_str(args, 1, &p)){
						goto failed;
					}

					size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
					if(0 == num){
						goto failed;
					}
					const PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(
							data->anchor_path,
							(num - 1),
							PvAnchorPathIndexTurn_Disable);
					PvPoint point = pv_anchor_point_get_point(ap_prev);

					switch(command){
						case 'H':
							point.x = args[0];
							break;
						case 'h':
							point.x += args[0];
							break;
						case 'V':
							point.y = args[0];
							break;
						case 'v':
						default:
							point.y += args[0];
							break;
					}

					pv_element_anchor_point_init(&ap);
					ap.points[PvAnchorPointIndex_Point] = point;
					is_append = true;
				}
				break;
			case 'C':
			case 'c':
				{
					if(!pv_read_args_from_str(args, 6, &p)){
						goto failed;
					}

					size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
					if(0 == num){
						pv_warning("'C' command on top?");
						goto failed;
					}
					PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(
							data->anchor_path,
							(num - 1),
							PvAnchorPathIndexTurn_Disable);

					PvPoint gpoint_next;
					pv_element_anchor_point_init(&ap);
					ap.points[PvAnchorPointIndex_HandleNext].x = 0;
					ap.points[PvAnchorPointIndex_HandleNext].y = 0;
					if('C' == command){
						ap.points[PvAnchorPointIndex_Point].x = args[4];
						ap.points[PvAnchorPointIndex_Point].y = args[5];
						ap.points[PvAnchorPointIndex_HandlePrev].x = args[2] - args[4];
						ap.points[PvAnchorPointIndex_HandlePrev].y = args[3] - args[5];
						gpoint_next = (PvPoint){.x = args[0], .y = args[1]};
					}else{
						PvPoint point = pv_anchor_point_get_point(ap_prev);

						ap.points[PvAnchorPointIndex_Point].x += point.x + args[4];
						ap.points[PvAnchorPointIndex_Point].y += point.y + args[5];

						pv_anchor_point_set_handle(&ap,
								PvAnchorPointIndex_HandlePrev,
								(PvPoint){point.x + args[2], point.y + args[3]});

						gpoint_next = (PvPoint){point.x + args[0], point.y + args[1]};
					}

					pv_anchor_point_set_handle(ap_prev,
							PvAnchorPointIndex_HandleNext, gpoint_next);

					is_append = true;
				}
				break;
			case 'S':
			case 's':
				{
					if(!pv_read_args_from_str(args, 4, &p)){
						goto failed;
					}
					pv_element_anchor_point_init(&ap);
					ap.points[PvAnchorPointIndex_HandlePrev].x = args[0] - args[2];
					ap.points[PvAnchorPointIndex_HandlePrev].y = args[1] - args[3];
					ap.points[PvAnchorPointIndex_Point].x = args[2];
					ap.points[PvAnchorPointIndex_Point].y = args[3];
					ap.points[PvAnchorPointIndex_HandleNext].x = 0;
					ap.points[PvAnchorPointIndex_HandleNext].y = 0;

					PvPoint point = PvPoint_Default;
					size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
					if(0 == num){
						pv_warning("command on top?");
						goto failed;
					}else{
						PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(
								data->anchor_path,
								(num - 1),
								PvAnchorPathIndexTurn_Disable);
						PvPoint handle = pv_anchor_point_get_handle_relate(
								ap_prev,
								PvAnchorPointIndex_HandlePrev);
						pv_anchor_point_set_handle_relate(
								ap_prev,
								PvAnchorPointIndex_HandleNext,
								(PvPoint){-handle.x, -handle.y});
						point = pv_anchor_point_get_point(ap_prev);
					}

					if('s' == command){
						ap.points[PvAnchorPointIndex_Point].x = args[2] + point.x;
						ap.points[PvAnchorPointIndex_Point].y = args[3] + point.y;

						pv_anchor_point_set_handle(&ap,
								PvAnchorPointIndex_HandlePrev,
								(PvPoint){args[0] + point.x, args[1] + point.y});
					}
					is_append = true;
				}
				break;
			case 'Z':
			case 'z':
				pv_anchor_path_set_is_close(data->anchor_path, true);
				break;
			default:
				pv_abortf("'%c' %td '%s'", command, (p - value), value);
		}

		if(is_append){
			pv_assert(pv_element_curve_add_anchor_point(element, ap));
			prev_point = pv_anchor_point_get_point(&ap);
		}
	}

	return true;
failed:
	pv_debug("%td '%s' '%s'", (p - value), value, p);

	return false;
}

static bool func_d_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
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

	bool ret = func_d_set_inline_(element, attribute_cache, (const char *)value);

	return ret;
}

static bool func_points_set_inline_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		const char *value
		)
{
	const int num_args = 10;
	double args[num_args];
	double prev_args[num_args];
	pv_double_array_fill(args, 0, num_args);
	pv_double_array_fill(prev_args, 0, num_args);

	PvAnchorPoint ap;
	pv_assert(element);
	pv_assert(element->data);
	pv_assert(PvElementKind_Curve == element->kind);

	// PvElementCurveData *data = element->data;

	const char *p = value;
	while('\0' != *p){
		bool is_append = pv_read_args_from_str(args, 2, &p);
		pv_element_anchor_point_init(&ap);
		ap.points[PvAnchorPointIndex_Point].x = args[0];
		ap.points[PvAnchorPointIndex_Point].y = args[1];

		if(is_append){
			if(!pv_element_curve_add_anchor_point(element, ap)){
				pv_error("");
				return false;
			}
		}

		p++;
	}

	return true;
}

static bool func_points_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("polygon", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("polyline", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	if(PvElementKind_Curve != element->kind){
		pv_error("");
		return false;
	}

	bool ret = func_points_set_inline_(element, attribute_cache, (const char *)value);

	return ret;
}

static bool func_x1_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("line", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	if(PvElementKind_Curve == element->kind){
		pv_error("%d '%s'", element->kind, attribute->name);
		return false;
	}

	int num = pv_element_curve_get_num_anchor_point(element);
	if(2 != num){
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	PvAnchorPath *anchor_path = pv_element_curve_get_anchor_path(element);
	pv_assert(anchor_path);
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			0,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);
	PvPoint point = pv_anchor_point_get_point(ap);
	point.x = v;
	pv_anchor_point_set_point(ap, point);

	return true;
}

static bool func_y1_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("line", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	if(PvElementKind_Curve == element->kind){
		pv_error("%d '%s'", element->kind, attribute->name);
		return false;
	}

	int num = pv_element_curve_get_num_anchor_point(element);
	if(2 != num){
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	PvAnchorPath *anchor_path = pv_element_curve_get_anchor_path(element);
	pv_assert(anchor_path);
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			0,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);
	PvPoint point = pv_anchor_point_get_point(ap);
	point.y = v;
	pv_anchor_point_set_point(ap, point);

	return true;
}
static bool func_x2_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("line", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	if(PvElementKind_Curve == element->kind){
		pv_error("%d '%s'", element->kind, attribute->name);
		return false;
	}

	int num = pv_element_curve_get_num_anchor_point(element);
	if(2 != num){
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	PvAnchorPath *anchor_path = pv_element_curve_get_anchor_path(element);
	pv_assert(anchor_path);
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			1,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);
	PvPoint point = pv_anchor_point_get_point(ap);
	point.x = v;
	pv_anchor_point_set_point(ap, point);

	return true;
}
static bool func_y2_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("line", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	if(PvElementKind_Curve == element->kind){
		pv_error("%d '%s'", element->kind, attribute->name);
		return false;
	}

	int num = pv_element_curve_get_num_anchor_point(element);
	if(2 != num){
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	PvAnchorPath *anchor_path = pv_element_curve_get_anchor_path(element);
	pv_assert(anchor_path);
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			1,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);
	PvPoint point = pv_anchor_point_get_point(ap);
	point.y = v;
	pv_anchor_point_set_point(ap, point);

	return true;
}

static bool func_xlink_href_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	pv_assert(PvElementKind_BasicShape == element->kind);

	if(0 == strcasecmp("image", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	bool res = false;

	PvElementBasicShapeData *element_data = element->data;
	PvBasicShapeRasterData *data = element_data->data;
	if(NULL != data->pixbuf){
		pv_warning("");
		goto failed0;
	}

	char *data_str = (char *)value;
	if(0 != strncmp("data:", data_str, strlen("data:"))){
		pv_warning("'%.20s'", data_str);
		goto failed0;
	}

	data_str += strlen("data:");
	char *filetype_str;
	char *encoding_str;
	bool is_failed = false;
	int ret;
	if(2 != (ret = sscanf(data_str, " image / %m[^;] ; %m[^,] ,", &filetype_str, &encoding_str))){
		pv_warning("%d, '%.20s'", ret, data_str);
		is_failed = true;
	}
	if(0 != strcmp("base64", encoding_str)){
		pv_warning("%d '%.20s'", ret, encoding_str);
		is_failed = true;
	}
	pv_debug("'%.20s', '%.20s'", filetype_str, encoding_str);
	free(filetype_str);
	free(encoding_str);
	if(is_failed){
		goto failed0;
	}

	data_str = strchr(data_str, ',');
	if(NULL == data_str){
		pv_warning("");
		goto failed0;
	}

	gsize out_len;
	guchar *raw_data = g_base64_decode((gchar *)data_str, &out_len);
	if(NULL == raw_data){
		pv_warning("");
		goto failed0;
	}

	GError *error = NULL;
	GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
	if(!gdk_pixbuf_loader_write (loader, raw_data, out_len, &error)){
		pv_warning("%s", error->message);
		g_error_free(error);
		goto failed1;
	}
	if(!gdk_pixbuf_loader_close(loader, &error)){
		pv_warning("%s", error->message);
		g_error_free(error);
		goto failed1;
	}

	data->pixbuf = gdk_pixbuf_copy(gdk_pixbuf_loader_get_pixbuf (loader));
	if(NULL == data->pixbuf){
		pv_warning("");
		goto failed1;
	}

	data->urischeme_byte_array = g_byte_array_new();
	pv_assert(data->urischeme_byte_array);
	g_byte_array_append(data->urischeme_byte_array, (guint8 *)value, strlen((const char *)value) + 1);
	if(NULL == data->urischeme_byte_array){
		pv_warning("");
		goto failed1;
	}

	attribute_cache->attributes[PvSvgAttributeKind_xlink_href].is_exist = true;

	res = true;

failed1:
	g_object_unref(loader);
failed0:

	return res;
}

static bool func_x_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("svg", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("image", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("rect", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	if(attribute_cache->attributes[PvSvgAttributeKind_x].is_exist){
		pv_warning("");
		return false;
	}
	attribute_cache->attributes[PvSvgAttributeKind_x].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_x].value = v;

	return true;
}

static bool func_y_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("svg", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("image", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("rect", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	if(attribute_cache->attributes[PvSvgAttributeKind_y].is_exist){
		pv_warning("");
		return false;
	}
	attribute_cache->attributes[PvSvgAttributeKind_y].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_y].value = v;

	return true;
}

static bool func_width_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("svg", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("image", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("rect", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	if(attribute_cache->attributes[PvSvgAttributeKind_width].is_exist){
		pv_warning("");
		return false;
	}
	attribute_cache->attributes[PvSvgAttributeKind_width].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_width].value = v;

	return true;
}

static bool func_height_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("svg", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("image", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("rect", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	if(attribute_cache->attributes[PvSvgAttributeKind_height].is_exist){
		pv_warning("");
		return false;
	}
	attribute_cache->attributes[PvSvgAttributeKind_height].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_height].value = v;

	return true;
}

static bool func_view_box_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("svg", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	return true;
}

static double sqrt_(double value, bool *ok)
{
	errno = 0;
	feclearexcept(FE_ALL_EXCEPT);
	value = sqrt(value);
	if(0 != errno){
		pv_warning("%s", strerror(errno));
		*ok = false;
		return 0;
	}
	int ret;
	if(0 != (ret = fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW))){
		pv_warning("%d", ret);
		*ok = false;
		return 0;
	}

	*ok = true;
	return value;
}

static bool set_appearances_from_cairo_matrix_(PvAppearance *appearances, cairo_matrix_t matrix)
{
	appearances[PvAppearanceKind_Translate].kind = PvAppearanceKind_Translate;
	PvAppearanceTranslateData *translate = &(appearances[PvAppearanceKind_Translate].translate);
	translate->move = pv_point_add(translate->move, (PvPoint){matrix.x0, matrix.y0});

	bool ok;
	PvPoint scale;
	scale.x = sqrt_((matrix.xx * matrix.xx) + (matrix.yx * matrix.yx), &ok);
	if(!ok){
		pv_warning("");
		return false;
	}
	scale.y = sqrt_((matrix.xy * matrix.xy) + (matrix.yy * matrix.yy), &ok);
	if(!ok){
		pv_warning("");
		return false;
	}
	PvAppearanceResizeData *resize = &(appearances[PvAppearanceKind_Resize].resize);
	if(PvAppearanceKind_Resize != appearances[PvAppearanceKind_Resize].kind){
		resize->resize = (PvPoint){1, 1};
	}
	appearances[PvAppearanceKind_Resize].kind = PvAppearanceKind_Resize;
	resize->resize = pv_point_mul(resize->resize, scale);

	translate->move = pv_point_mul(translate->move, scale);

	return true;
}

static bool func_transform_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	bool res = false;

	PvStrMap *transform_str_maps = new_transform_str_maps_from_str_((const char *)value);
	if(NULL == transform_str_maps){
		pv_error("%.20s", (char *)value);
		goto failed;
	}
	for(int i = 0; NULL != transform_str_maps[i].key; i++){
		const int num_args = 10;
		double args[num_args];
		pv_double_array_fill(args, 0, num_args);
		const char *p = transform_str_maps[i].value;

		if(0 == strcmp("translate", transform_str_maps[i].key)){
			if(!pv_read_args_from_str(args, 2, &p)){
				pv_warning("'%s','%s'", transform_str_maps[i].key, transform_str_maps[i].value);
				if(conf->imageFileReadOption->is_strict){
					pv_error("strict");
					goto failed;
				}
				continue;
			}

			conf->appearances[PvAppearanceKind_Translate].kind = PvAppearanceKind_Translate;
			PvAppearanceTranslateData *translate
				= &(conf->appearances[PvAppearanceKind_Translate].translate);
			translate->move = pv_point_add(translate->move, (PvPoint){args[0], args[1]});

			pv_debug("translate:'%s',(%f,%f),(%f,%f)",
					transform_str_maps[i].key, args[0], args[1],
					translate->move.x, translate->move.y);
		}else if(0 == strcmp("matrix", transform_str_maps[i].key)){
			if(!pv_read_args_from_str(args, 6, &p)){
				pv_warning("'%s','%s'", transform_str_maps[i].key, transform_str_maps[i].value);
				if(conf->imageFileReadOption->is_strict){
					pv_error("strict");
					goto failed;
				}
				continue;
			}

			if(1 != args[0] || 0 != args[1] || 0 != args[2] || 1 != args[3]){
				if(conf->imageFileReadOption->is_strict){
					pv_error("Not implement.");
					pv_error("strict");
					goto failed;
				}
			}

			cairo_matrix_t matrix;
			cairo_matrix_init(&matrix, args[0], args[1], args[2], args[3], args[4], args[5]);
			if(!set_appearances_from_cairo_matrix_(conf->appearances, matrix)){
				pv_warning("'%s','%s'", transform_str_maps[i].key, transform_str_maps[i].value);
				if(conf->imageFileReadOption->is_strict){
					pv_error("strict");
					goto failed;
				}
				continue;
			}

			PvPoint resize = conf->appearances[PvAppearanceKind_Resize].resize.resize;
			pv_debug("'%s':"
					" (%9.4f,%9.4f,%9.4f,%9.4f,%9.4f,%9.4f),"
					" (%9.4f,%9.4f),"
					,
					transform_str_maps[i].key,
					args[0], args[1], args[2], args[3], args[4], args[5],
					resize.x, resize.y
				);
		}else{
			pv_warning("unknown key: '%s'(%d)'%s':'%s'(%d)",
					(char *)value, xmlnode->line,
					transform_str_maps[i].key, transform_str_maps[i].value, i);
			if(conf->imageFileReadOption->is_strict){
				pv_error("strict");
				goto failed;
			}
		}
	}

	res = true;
failed:
	pv_str_maps_free(transform_str_maps);

	return res;
}

static bool func_fill_opacity_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	double opacity = pv_io_util_get_double_from_str((const char *)value);

	attribute_cache->attributes[PvSvgAttributeKind_fill_opacity].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_fill_opacity].value = opacity;

	return true;
}

static bool func_stroke_opacity_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	double opacity = pv_io_util_get_double_from_str((const char *)value);

	attribute_cache->attributes[PvSvgAttributeKind_stroke_opacity].is_exist = true;
	attribute_cache->attributes[PvSvgAttributeKind_stroke_opacity].value = opacity;

	return true;
}

static bool func_groupmode_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("g", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	pv_assert(PvElementKind_Layer == element->kind || PvElementKind_Group == element->kind);

	if(0 == strcasecmp("layer", (char*)value)){
		element->kind = PvElementKind_Layer;
	}

	return true;
}

static bool func_style_set_(
		PvElement *element,
		PvSvgAttributeCache *attribute_cache,
		PvSvgReadConf *conf,
		const char *value_,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	bool is_success = false;

	PvStrMap *css_str_maps = pv_new_css_str_maps_from_str((const char *)value_);
	for(int i = 0; NULL != css_str_maps[i].key; i++){
		const char *value = css_str_maps[i].value;
		const PvSvgAttributeInfo *info = pv_svg_get_svg_attribute_info_from_name(css_str_maps[i].key);
		if(NULL == info){
			pv_warning("unknown css style key: '%s'(%d)'%s':'%s'(%d)",
					(const char *)value,
					xmlnode->line,
					css_str_maps[i].key, css_str_maps[i].value,
					i);
			if(conf->imageFileReadOption->is_strict){
				goto failed;
			}
		}else{
			if(!info->is_able_style){
				pv_warning("no able css style key: '%s'(%d)'%s':'%s'(%d)",
						(const char *)value,
						xmlnode->line,
						css_str_maps[i].key, css_str_maps[i].value,
						i);
				if(conf->imageFileReadOption->is_strict){
					goto failed;
				}
			}
			bool ret = info->pv_svg_attribute_func_set(
					element,
					attribute_cache,
					conf,
					value,
					xmlnode,
					attribute);
			if(!ret){
				pv_warning("can not read css style key: '%s'(%d)'%s':'%s'(%d)",
						(const char *)value,
						xmlnode->line,
						css_str_maps[i].key, css_str_maps[i].value,
						i);
				goto failed;
			}
		}
	}

	is_success = true;

failed:
	pv_str_maps_free(css_str_maps);

	if(!is_success){
		if(conf->imageFileReadOption->is_strict){
			pv_error("strict");
			return false;
		}
	}

	return true;
}


const PvSvgAttributeInfo _pv_svg_attribute_infos[] = {
	{
		.name = "fill",
		.pv_svg_attribute_func_set = func_fill_set_,
		.is_able_style = true,
	},
	{
		.name = "stroke",
		.pv_svg_attribute_func_set = func_stroke_set_,
		.is_able_style = true,
	},
	{
		.name = "stroke-width",
		.pv_svg_attribute_func_set = func_stroke_width_set_,
		.is_able_style = true,
	},
	{
		.name = "stroke-linecap",
		.pv_svg_attribute_func_set = func_stroke_linecap_set_,
		.is_able_style = true,
	},
	{
		.name = "stroke-linejoin",
		.pv_svg_attribute_func_set = func_stroke_linejoin_set_,
		.is_able_style = true,
	},
	{
		.name = "d",
		.pv_svg_attribute_func_set = func_d_set_,
		.is_able_style = false,
	},
	{
		.name = "points",
		.pv_svg_attribute_func_set = func_points_set_,
		.is_able_style = false,
	},
	{
		.name = "x1",
		.pv_svg_attribute_func_set = func_x1_set_,
		.is_able_style = false,
	},
	{
		.name = "y1",
		.pv_svg_attribute_func_set = func_y1_set_,
		.is_able_style = false,
	},
	{
		.name = "x2",
		.pv_svg_attribute_func_set = func_x2_set_,
		.is_able_style = false,
	},
	{
		.name = "y2",
		.pv_svg_attribute_func_set = func_y2_set_,
		.is_able_style = false,
	},
	{
		.name = "href",
		.pv_svg_attribute_func_set = func_xlink_href_set_,
		.is_able_style = false,
	},
	{
		.name = "x",
		.pv_svg_attribute_func_set = func_x_set_,
		.is_able_style = false,
	},
	{
		.name = "y",
		.pv_svg_attribute_func_set = func_y_set_,
		.is_able_style = false,
	},
	{
		.name = "width",
		.pv_svg_attribute_func_set = func_width_set_,
		.is_able_style = false,
	},
	{
		.name = "height",
		.pv_svg_attribute_func_set = func_height_set_,
		.is_able_style = false,
	},
	{
		.name = "viewBox",
		.pv_svg_attribute_func_set = func_view_box_set_,
		.is_able_style = false,
	},
	{
		.name = "transform",
		.pv_svg_attribute_func_set = func_transform_set_,
		.is_able_style = false,
	},
	{
		.name = "fill-opacity",
		.pv_svg_attribute_func_set = func_fill_opacity_set_,
		.is_able_style = true,
	},
	{
		.name = "stroke-opacity",
		.pv_svg_attribute_func_set = func_stroke_opacity_set_,
		.is_able_style = true,
	},
	{
		.name = "groupmode",
		.pv_svg_attribute_func_set = func_groupmode_set_,
		.is_able_style = false,
	},
	{
		.name = "style",
		.pv_svg_attribute_func_set = func_style_set_,
		.is_able_style = false,
	},
	{
		.name = "version",
		.pv_svg_attribute_func_set = func_nop_set_,
		.is_able_style = false,
	},
	{
		.name = "label",
		.pv_svg_attribute_func_set = func_nop_set_,
		.is_able_style = false,
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

