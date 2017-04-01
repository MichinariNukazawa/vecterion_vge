#include "pv_basic_shape_info.h"

#include <string.h>
#include "pv_error.h"
#include "pv_general.h"
#include "pv_urischeme.h"



// ******** ********
// Raster
// ******** ********

static gpointer func_raster_new_data_()
{
	PvBasicShapeRasterData *data = malloc(sizeof(PvBasicShapeRasterData));
	pv_assert(data);

	data->path = NULL;
	data->pixbuf = NULL;
	data->urischeme_byte_array = NULL;

	return data;
}

static void func_raster_free_data_(
		void *data_)
{
	PvBasicShapeRasterData *data = data_;
	pv_assert(data);

	if(NULL != data->path){
		free(data->path);
	}

	if(NULL != data->pixbuf){
		g_object_unref(G_OBJECT(data->pixbuf));
	}

	if(NULL != data->urischeme_byte_array){
		g_byte_array_unref(data->urischeme_byte_array);
	}

	free(data);
}

static gpointer func_raster_copy_new_data_(
		const void *data_)
{
	const PvBasicShapeRasterData *data = data_;
	pv_assert(data);

	PvBasicShapeRasterData *new_data = func_raster_new_data_();
	pv_assert(new_data);

	*new_data = *data;

	new_data->path = g_strdup(data->path);

	if(NULL != new_data->pixbuf){
		g_object_ref(G_OBJECT(new_data->pixbuf));
	}

	if(NULL != new_data->urischeme_byte_array){
		g_byte_array_ref(new_data->urischeme_byte_array);
	}

	return new_data;
}

static bool func_raster_write_svg_(
		xmlNodePtr *node_,
		InfoTargetSvg *target,
		const PvElement *element,
		const ConfWriteSvg *conf)
{
	pv_assert(element);
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)element->data;
	pv_assert(element_data);

	PvBasicShapeRasterData *data = element_data->data;
	pv_assert(data);

	if(NULL == data->urischeme_byte_array){
		if(NULL != data->path){
			char *urischeme_str = pv_urischeme_get_from_image_filepath(data->path);
			pv_assert(urischeme_str);
			data->urischeme_byte_array = g_byte_array_new();
			pv_assert(data->urischeme_byte_array);
			g_byte_array_append(data->urischeme_byte_array, (guint8 *)urischeme_str, strlen(urischeme_str) + 1);
			g_free(urischeme_str);
		}
	}
	if(NULL == data->urischeme_byte_array){
		if(NULL != data->pixbuf){
			// not implement.
		}
	}

	if(NULL == data->urischeme_byte_array){
		pv_error("");
		return false;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assert(info);
	PvRect rect = info->func_get_rect_by_anchor_points(element);
	char *x_str = g_strdup_printf("%.6f", rect.x);
	char *y_str = g_strdup_printf("%.6f", rect.y);
	char *w_str = g_strdup_printf("%.6f", rect.w);
	char *h_str = g_strdup_printf("%.6f", rect.h);

	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "image");
	pv_assert(node);

	xmlNewProp(node, BAD_CAST "xlink:href", BAD_CAST data->urischeme_byte_array->data);
	xmlNewProp(node, BAD_CAST "x", BAD_CAST x_str);
	xmlNewProp(node, BAD_CAST "y", BAD_CAST y_str);
	xmlNewProp(node, BAD_CAST "width", BAD_CAST w_str);
	xmlNewProp(node, BAD_CAST "height", BAD_CAST h_str);
	xmlAddChild(target->xml_parent_node, node);

	g_free(x_str);
	g_free(y_str);
	g_free(w_str);
	g_free(h_str);

	*node_ = node;

	return true;
}


static PvPoint get_pixbuf_size_(const GdkPixbuf *pb)
{
	PvPoint ret = {
		.x = gdk_pixbuf_get_width(pb),
		.y = gdk_pixbuf_get_height(pb),
	};

	return ret;
}

static PvPoint func_raster_get_size_(const void *data_)
{
	const PvBasicShapeRasterData *data = data_;

	return get_pixbuf_size_(data->pixbuf);
}

static void func_raster_draw_(
		cairo_t *cr,
		const PvElement *element,
		PvPoint resize)
{
	pv_assert(element);
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)element->data;
	pv_assert(element_data);
	PvBasicShapeRasterData *data = element_data->data;
	pv_assert(data);

	pv_assert(data->pixbuf);

	cairo_scale(cr, resize.x, resize.y);
	gdk_cairo_set_source_pixbuf (cr, data->pixbuf, 0, 0);
	cairo_paint (cr);
}

static bool func_raster_is_diff_one_(
		const void *data0_,
		const void *data1_)
{
	const PvBasicShapeRasterData *data0 = data0_;
	pv_assert(data0);
	const PvBasicShapeRasterData *data1 = data1_;
	pv_assert(data1);

	// ** dataX->path is not check.

	//! @todo check basic_shape pixbuf.
	if(data0->pixbuf != data1->pixbuf){
		pv_debug("####");
		return true;
	}

	return false;
}



// ******** ********
// Rect
// ******** ********

static gpointer func_rect_new_data_()
{
	PvBasicShapeRectData *data = malloc(sizeof(PvBasicShapeRectData));
	pv_assert(data);

	return data;
}

static void func_rect_free_data_(
		void *data_)
{
	PvBasicShapeRectData *data = data_;
	pv_assert(data);

	free(data);
}

static gpointer func_rect_copy_new_data_(
		const void *data_)
{
	const PvBasicShapeRectData *data = data_;
	pv_assert(data);

	PvBasicShapeRectData *new_data = func_rect_new_data_();
	pv_assert(new_data);

	*new_data = *data;

	return new_data;
}

static bool func_rect_write_svg_(
		xmlNodePtr *node_,
		InfoTargetSvg *target,
		const PvElement *element,
		const ConfWriteSvg *conf)
{
	pv_assert(element);
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)element->data;
	pv_assert(element_data);

	PvBasicShapeRectData *data = element_data->data;
	pv_assert(data);

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assert(info);
	PvRect rect = info->func_get_rect_by_anchor_points(element);
	char *x_str = g_strdup_printf("%.6f", rect.x);
	char *y_str = g_strdup_printf("%.6f", rect.y);
	char *w_str = g_strdup_printf("%.6f", rect.w);
	char *h_str = g_strdup_printf("%.6f", rect.h);

	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "rect");
	pv_assert(node);

	xmlNewProp(node, BAD_CAST "x", BAD_CAST x_str);
	xmlNewProp(node, BAD_CAST "y", BAD_CAST y_str);
	xmlNewProp(node, BAD_CAST "width", BAD_CAST w_str);
	xmlNewProp(node, BAD_CAST "height", BAD_CAST h_str);
	xmlAddChild(target->xml_parent_node, node);

	g_free(x_str);
	g_free(y_str);
	g_free(w_str);
	g_free(h_str);

	*node_ = node;

	return true;
}

static PvPoint func_rect_get_size_(const void *data_)
{
	return (PvPoint){1, 1};
}

static void func_rect_draw_(
		cairo_t *cr,
		const PvElement *element,
		PvPoint resize)
{
	pv_assert(element);
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)element->data;
	pv_assert(element_data);
	PvBasicShapeRasterData *data = element_data->data;
	pv_assert(data);

	PvRect rect = {0, 0, 1, 1};
	cairo_rectangle(cr, rect.x, rect.y, rect.w * resize.x, rect.h * resize.y);

	cairo_set_line_width(cr, element->stroke.width);

	PvCairoRgbaColor cc_f = pv_color_get_cairo_rgba(element->color_pair.colors[PvColorPairGround_BackGround]);
	cairo_set_source_rgba (cr, cc_f.r, cc_f.g, cc_f.b, cc_f.a);
	cairo_fill_preserve(cr);

	PvCairoRgbaColor cc_s = pv_color_get_cairo_rgba(element->color_pair.colors[PvColorPairGround_ForGround]);
	cairo_set_source_rgba (cr, cc_s.r, cc_s.g, cc_s.b, cc_s.a);
	cairo_stroke(cr);
}

static bool func_rect_is_diff_one_(
		const void *data0_,
		const void *data1_)
{
	const PvBasicShapeRectData *data0 = data0_;
	pv_assert(data0);
	const PvBasicShapeRectData *data1 = data1_;
	pv_assert(data1);

	return false;
}




const PvBasicShapeInfo pv_basic_shape_infos_[] = {
	{PvBasicShapeKind_Rect, "Rect",
		.func_new_data			= func_rect_new_data_,
		.func_free_data			= func_rect_free_data_,
		.func_copy_new_data		= func_rect_copy_new_data_,
		.func_write_svg			= func_rect_write_svg_,
		.func_draw			= func_rect_draw_,
		.func_get_size			= func_rect_get_size_,
		.func_is_diff_one		= func_rect_is_diff_one_,
	},
	{PvBasicShapeKind_Raster, "Raster",
		.func_new_data			= func_raster_new_data_,
		.func_free_data			= func_raster_free_data_,
		.func_copy_new_data		= func_raster_copy_new_data_,
		.func_write_svg			= func_raster_write_svg_,
		.func_draw			= func_raster_draw_,
		.func_get_size			= func_raster_get_size_,
		.func_is_diff_one		= func_raster_is_diff_one_,
	},
};



const PvBasicShapeInfo *pv_basic_shape_info_get_from_kind(PvBasicShapeKind kind)
{
	size_t num = sizeof(pv_basic_shape_infos_) / sizeof(PvBasicShapeInfo);
	for(int i = 0; i < (int)num; i++){
		if(kind == pv_basic_shape_infos_[i].kind){
			return &pv_basic_shape_infos_[i];
		}
	}

	pv_bug("%d", kind);
	return NULL;
}

