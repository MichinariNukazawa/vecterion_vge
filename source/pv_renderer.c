#include "pv_renderer.h"

#include "pv_error.h"
#include "pv_render_option.h"
#include "pv_element_infos.h"

bool _pv_renderer_is_group_kind(PvElement * const element)
{
	switch(element->kind){
		case PvElementKind_Root:
		case PvElementKind_Layer:
		case PvElementKind_Group:
			return true;
			break;
		default:
			return false;
	}
}

bool _pv_renderer_cairo_recersive(
		cairo_t *cr,
		PvElement * const element,
		const PvRenderOption render_option,
		int *level)
{
	bool ret = true;
	(*level)++;

	if(_pv_renderer_is_group_kind(element)){
		int num = pv_general_get_parray_num((void **)element->childs);
		for(int i = 0; i < num; i++){
			if(!_pv_renderer_cairo_recersive(cr,
						element->childs[i],
						render_option,
						level)){
				pv_error("%d\n", i);
				ret = false;
				goto end;
			}
		}
	}else{
		const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
		if(NULL == info){
			pv_bug("");
			ret = false;
			goto end;
		}
		if(!info->func_draw(cr, render_option, element)){
			pv_bug("");
			ret = false;
			goto end;
		}
	}

end:
	(*level)--;
	return ret;
}

bool _pv_renderer_cairo_background(cairo_t *cr,
		const PvVg * const vg,
		const PvRenderContext render_context)
{
	// pv_debug("%f, %f, %f, %f, \n", (vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);
	cairo_set_source_rgb (cr, 0.6, 0.6, 0.6);

	int unit = 16;
	for(int y = 0; y < (vg->rect).h; y += unit){
		for(int x = 0 + (((y/unit) % 2) * unit); x < (vg->rect).w; x += (unit * 2)){
			cairo_rectangle (cr, x, y, unit, unit);
		}
	}
	cairo_fill (cr);

	return true;
}

GdkPixbuf *pv_renderer_pixbuf_from_vg(PvVg * const vg,
		const PvRenderContext render_context,
		const PvFocus focus)
{
	int width = (int)vg->rect.w;
	int height = (int)vg->rect.h;
	width *= render_context.scale;
	height *= render_context.scale;
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
			width, height);
	if(NULL == surface){
		pv_bug("");
		return NULL;
	}

	cairo_t *cr = cairo_create (surface);

	if(!_pv_renderer_cairo_background(cr, vg, render_context)){
		pv_error("");
		return NULL;
	}

	PvRenderOption render_option = {
		.render_context = render_context,
		.focus = focus,
	};
	int level = 0;
	if(!_pv_renderer_cairo_recersive(cr, vg->element_root, render_option, &level)){
		pv_error("");
		return NULL;
	}

	if(NULL != focus.element && !_pv_renderer_is_group_kind(focus.element)){
		render_option.render_context.is_focus = true;
		if(!_pv_renderer_cairo_recersive(cr, focus.element, render_option, &level)){
			pv_error("");
			return NULL;
		}
	}

	GdkPixbuf *pb = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
	if(NULL == pb){
		pv_error("");
		return NULL;
	}

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}
