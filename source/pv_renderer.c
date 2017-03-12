#include "pv_renderer.h"

#include <math.h>
#include "pv_error.h"
#include "pv_render_option.h"
#include "pv_element_info.h"
#include "pv_cairo.h"


static bool _pv_renderer_is_group_kind(const PvElement *element)
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

static bool _pv_renderer_cairo_recursive(
		cairo_t *cr,
		const PvElement *element,
		const PvRenderOption render_option,
		int *level)
{
	bool ret = true;
	(*level)++;

	if(_pv_renderer_is_group_kind(element)){
		int num = pv_general_get_parray_num((void **)element->childs);
		for(int i = 0; i < num; i++){
			if(!_pv_renderer_cairo_recursive(cr,
						element->childs[i],
						render_option,
						level)){
				pv_error("%d", i);
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
		if(render_option.render_context.is_focus){
			if(NULL == info->func_draw_focusing
					|| !info->func_draw_focusing(cr, render_option, element)){
				pv_bug("");
				ret = false;
				goto end;
			}
		}else{
			if(NULL == info->func_draw || !info->func_draw(cr, render_option, element)){
				pv_bug("");
				ret = false;
				goto end;
			}
		}
	}

end:
	(*level)--;
	return ret;
}

static bool _pv_renderer_cairo_background(cairo_t *cr,
		const PvVg * const vg,
		const PvRenderContext render_context)
{
	// pv_debug("%f, %f, %f, %f, ", (vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);

	//! @todo checkboard drawing use to start offset (x,y)

	int w_size = (vg->rect).w * render_context.scale;
	int h_size = (vg->rect).h * render_context.scale;

	// clipping
	cairo_save(cr);
	cairo_rectangle(cr, 0, 0, w_size, h_size);
	cairo_clip(cr);

	switch(render_context.background_kind){
		case PvBackgroundKind_Transparent:
			break;
		case PvBackgroundKind_White:
			{
				// ** fill background white
				cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
				cairo_rectangle (cr, 0, 0, w_size, h_size);
				cairo_fill (cr);
			}
			break;
		case PvBackgroundKind_Checkboard:
			{
				pv_cairo_fill_checkboard(cr, (PvRect){0, 0, w_size, h_size});
			}
			break;
		default:
			pv_bug("%d", render_context.background_kind);
	}

	// frame
	if(render_context.is_frame_line){
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
		cairo_rectangle(cr, 0, 0, w_size, h_size);
		cairo_set_line_width(cr, 1 * 2);
		cairo_stroke(cr);
	}

	cairo_restore(cr); // clear clipping

	return true;
}

GdkPixbuf *pv_renderer_pixbuf_from_vg(
		const PvVg *vg,
		const PvRenderContext render_context,
		const PvFocus *focus,
		const PvElement *element_overwrite)
{
	if(NULL == vg){
		pv_bug("");
		return NULL;
	}

	const int width  = ceil(vg->rect.w * render_context.scale) + (render_context.margin * 2);
	const int height = ceil(vg->rect.h * render_context.scale) + (render_context.margin * 2);
	if(width <= 0 || height <= 0){
		pv_bug("");
		return NULL;
	}

	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	if(NULL == surface){
		pv_bug("");
		return NULL;
	}

	cairo_t *cr = cairo_create (surface);
	cairo_translate(cr, render_context.margin, render_context.margin);

	if(!_pv_renderer_cairo_background(cr, vg, render_context)){
		pv_error("");
		return NULL;
	}

		PvRenderOption render_option = {
			.render_context = render_context,
			.focus = focus,
		};
		int level = 0;
		if(!_pv_renderer_cairo_recursive(cr, vg->element_root, render_option, &level)){
			pv_error("");
			return NULL;
		}

	if(NULL != focus){
		PvRect rect_extent = PvRect_Default;
		int num = pv_general_get_parray_num((void **)focus->elements);
		for(int i = 0; i < num; i++){
			const PvElement *focus_element = focus->elements[i];
			if(NULL != focus_element && !_pv_renderer_is_group_kind(focus_element)){
				render_option.render_context.is_focus = true;
				if(!_pv_renderer_cairo_recursive(cr, focus_element, render_option, &level)){
					pv_error("");
					return NULL;
				}
			}

			const PvElementInfo *info = pv_element_get_info_from_kind(focus_element->kind);
			if(0 == i){
				rect_extent = info->func_get_rect_by_anchor_points(focus_element);
			}else{
				PvRect r = info->func_get_rect_by_anchor_points(focus_element);
				rect_extent = pv_rect_expand(rect_extent, r);
			}
		}
		if(0 != num){
			rect_extent = pv_rect_mul_value(rect_extent, render_context.scale);
			pv_cairo_set_source_rgba_workingcolor(cr);
			cairo_rectangle (cr, rect_extent.x, rect_extent.y, rect_extent.w, rect_extent.h);
			cairo_stroke(cr);
		}
	}

	if(NULL != element_overwrite){
		render_option.focus = NULL;
		render_option.render_context.is_focus = false;
		level = 0;
		if(!_pv_renderer_cairo_recursive(cr, element_overwrite, render_option, &level)){
			pv_error("");
			return NULL;
		}
	}

	GdkPixbuf *pb = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
	if(NULL == pb){
		pv_error("%d, %d, %f", width, height, render_context.scale);
		return NULL;
	}

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}

