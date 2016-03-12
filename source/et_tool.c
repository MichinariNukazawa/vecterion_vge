#include "et_tool.h"

#include "et_error.h"

bool _et_tool_is_init = false; // check initialized tools

EtToolInfo *_et_tool_get_info_from_id(EtToolId tool_id)
{
	// TODO: implement to infos.
	int num_tool = et_tool_get_num();
	if(tool_id < 0 || num_tool <= tool_id){
		et_error("");
		return false;
	}

	return &_et_tool_infos[tool_id];
}

GdkPixbuf *_et_tool_conv_pixbuf(GdkPixbuf *pb_src)
{

	double w = (double)gdk_pixbuf_get_width(pb_src);
	double h = (double)gdk_pixbuf_get_height(pb_src);
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
	if(NULL == surface){
		et_bug("");
		return NULL;
	}
	cairo_t *cr = cairo_create (surface);

	gdk_cairo_set_source_pixbuf (cr, pb_src, 0, 0);
	cairo_paint (cr);

	int T = 3;
	cairo_rectangle (cr, T, T, w- T, h - T);
	cairo_set_source_rgba (cr, 0.7, 0, 0, 0.3);
	cairo_fill (cr);

	GdkPixbuf *pb = gdk_pixbuf_get_from_surface(surface, 0, 0, w, h);
	if(NULL == pb){
		et_error("");
		return NULL;
	}

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}

bool et_tool_init()
{
	if(_et_tool_is_init){
		et_bug("");
		return false;
	}

	int num_tool = et_tool_get_num();
	et_debug("%d\n", num_tool);
	for(int tool_id = 0; tool_id < num_tool; tool_id++){
		EtToolInfo *info = _et_tool_get_info_from_id(tool_id);
		if(NULL == info){
			et_bug("");
			return false;
		}

		// ** make image(cursor,icons)
		GError *error = NULL;
		info->cursor = gdk_pixbuf_new_from_file(info->filepath_cursor, &error);
		if(NULL == info->cursor){
			et_error("'%s'\n", error->message);
			return false;
		}
		info->icon = info->cursor;
		info->icon_focus = _et_tool_conv_pixbuf(info->icon);
	}

	_et_tool_is_init = true;

	return true;
}

const EtToolInfo *et_tool_get_info_from_id(EtToolId tool_id)
{
	if(!_et_tool_is_init){
		et_bug("");
		return NULL;
	}

	const EtToolInfo *info = _et_tool_get_info_from_id(tool_id);
	return info;
}

