#include "pv_renderer.h"

#include <math.h>
#include "pv_error.h"
#include "pv_render_option.h"
#include "pv_element_info.h"
#include "pv_cairo.h"


static bool pv_element_is_group_kind_(const PvElement *element)
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

static bool pv_render_cairo_recursive_(
		cairo_t *cr,
		const PvElement *element,
		const PvRenderOption render_option,
		int *level)
{
	bool ret = true;
	(*level)++;

	if(pv_element_is_group_kind_(element)){
		size_t num = pv_general_get_parray_num((void **)element->childs);
		for(int i = 0; i < (int)num; i++){
			if(!pv_render_cairo_recursive_(
						cr,
						element->childs[i],
						render_option,
						level)){
				pv_error("%d", i);
				ret = false;
				goto failed;
			}
		}
	}else{
		const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
		pv_assertf(info, "%d", element->kind);
		if(render_option.render_context.is_focus){
			info->func_draw_focusing(cr, render_option, element);
		}else{
			info->func_draw(cr, render_option, element);
		}
	}

failed:
	(*level)--;
	return ret;
}

static void pv_rendererd_cairo_background_(
		cairo_t *cr,
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
				if(render_context.is_transparent_grid){
					pv_cairo_fill_checkboard(cr, (PvRect){0, 0, w_size, h_size});
				}else{
					cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
					cairo_rectangle (cr, 0, 0, w_size, h_size);
					cairo_fill (cr);
				}
			}
			break;
		default:
			pv_abortf("%d", render_context.background_kind);
	}

	// frame
	if(render_context.is_frame_line){
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
		cairo_rectangle(cr, 0, 0, w_size, h_size);
		cairo_set_line_width(cr, 1 * 2);
		cairo_stroke(cr);
	}

	cairo_restore(cr); // clear clipping
}

GdkPixbuf *pv_renderer_pixbuf_from_vg(
		const PvVg *vg,
		const PvRenderContext render_context,
		const PvFocus *focus,
		const PvElement *element_overwrite)
{
	pv_assert(vg);
	GdkPixbuf *pb = NULL;

	const int width  = ceil(vg->rect.w * render_context.scale) + (render_context.margin * 2);
	const int height = ceil(vg->rect.h * render_context.scale) + (render_context.margin * 2);
	if(width <= 0 || height <= 0){
		pv_bug("%dx%d, %fx%f, %f", width, height, vg->rect.w, vg->rect.h, render_context.scale);
		return NULL;
	}

	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	pv_assert(surface);

	cairo_t *cr = cairo_create (surface);
	pv_assert(cr);
	cairo_translate(cr, render_context.margin, render_context.margin);

	pv_rendererd_cairo_background_(cr, vg, render_context);

	PvRenderOption render_option = {
		.render_context = render_context,
		.focus = focus,
	};
	int level = 0;
	if(!pv_render_cairo_recursive_(cr, vg->element_root, render_option, &level)){
		pv_error("");
		goto failed;
	}

	if(NULL != focus){
		PvRect rect_extent = PvRect_Default;
		size_t num = pv_general_get_parray_num((void **)focus->elements);
		for(int i = 0; i < (int)num; i++){
			const PvElement *focus_element = focus->elements[i];
			if(!pv_element_is_group_kind_(focus_element)){
				render_option.render_context.is_focus = true;
				if(!pv_render_cairo_recursive_(cr, focus_element, render_option, &level)){
					pv_error("");
					goto failed;
				}
			}

			const PvElementInfo *info = pv_element_get_info_from_kind(focus_element->kind);
			pv_assertf(info, "%d", focus_element->kind);
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
		if(!pv_render_cairo_recursive_(cr, element_overwrite, render_option, &level)){
			pv_error("");
			return NULL;
		}
	}

	pb = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
	pv_assertf(pb, "%dx%d, %fx%f, %f", width, height, vg->rect.w, vg->rect.h, render_context.scale);

failed:

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}

