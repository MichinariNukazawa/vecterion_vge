#include "pv_renderer.h"

#include <math.h>
#include "pv_error.h"
#include "pv_render_option.h"
#include "pv_element_info.h"
#include "pv_element_render_context.h"
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
		PvElementRenderContext *element_render_context,
		const PvRenderOption render_option,
		int *level)
{
	bool ret = true;
	(*level)++;

	if(element->is_invisible){
		goto finally;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assertf(info, "%d", element->kind);
	PvElementDrawRecursive recursive;
	if(render_option.render_context.is_focus){
		recursive = info->func_draw_focusing(
				cr,
				element_render_context,
				render_option,
				element);
		recursive = PvElementDrawRecursive_Continues;
	}else{
		recursive = info->func_draw(
				cr,
				element_render_context,
				render_option,
				element);
	}

	switch(recursive){
		case PvElementDrawRecursive_Continues:
			{
				size_t num = pv_general_get_parray_num((void **)element->childs);
				for(int i = 0; i < (int)num; i++){
					if(!pv_render_cairo_recursive_(
								cr,
								element->childs[i],
								element_render_context,
								render_option,
								level)){
						pv_error("%d", i);
						ret = false;
						goto failed;
					}
				}
			}
			break;
		case PvElementDrawRecursive_End:
			break;
		default:
			pv_abortf("%d", recursive);
	}

	info->func_draw_after(
			cr,
			element_render_context,
			render_option,
			element);

finally:
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
		const PvDocumentPreference *document_preference,
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

	PvElementRenderContext element_render_context_ = PvElementRenderContext_Default;
	PvElementRenderContext *element_render_context = &element_render_context_;

	// ** document
	PvRenderOption render_option = {
		.render_context = render_context,
		.focus = focus,
	};
	int level = 0;
	if(!pv_render_cairo_recursive_(
				cr,
				vg->element_root,
				element_render_context,
				render_option,
				&level)){
		pv_error("");
		goto failed;
	}

	// ** document preference
	if(NULL != document_preference){
		if(document_preference->snap_context.is_snap_for_grid){
			cairo_set_line_width (cr, 1.0);
			pv_cairo_set_source_rgba_workingcolor_with_opacity(cr, 0.5);

			//! @todo rendering area
			PvRect area = {
				-100,
				-100,
				+100 + (vg->rect.w * render_context.scale),
				+100 + (vg->rect.h * render_context.scale),
			};

			//! @todo margin area
			for(int x = 0; x < vg->rect.w; x += document_preference->snap_context.grid.x){
				double x_pos = (x * render_context.scale);
				cairo_move_to (cr, x_pos, area.y);
				cairo_line_to (cr, x_pos, area.h);
			}

			for(int y = 0; y < vg->rect.h; y += document_preference->snap_context.grid.y){
				double y_pos = (y * render_context.scale);
				cairo_move_to (cr, area.x, y_pos);
				cairo_line_to (cr, area.w, y_pos);
			}

			cairo_stroke(cr);
		}
	}

	// ** focus
	if(NULL != focus){
		PvRect rect_extent_b = PvRect_Default;
		PvRect rect_extent_a = PvRect_Default;
		PvRect rb, ra;
		size_t num = pv_general_get_parray_num((void **)focus->elements);
		for(int i = 0; i < (int)num; i++){
			const PvElement *focus_element = focus->elements[i];
			if(!pv_element_is_group_kind_(focus_element)){
				render_option.render_context.is_focus = true;
				if(!pv_render_cairo_recursive_(
							cr,
							focus_element,
							element_render_context,
							render_option,
							&level)){
					pv_error("");
					goto failed;
				}
			}

			const PvElementInfo *info = pv_element_get_info_from_kind(focus_element->kind);
			pv_assertf(info, "%d", focus_element->kind);

			rb = info->func_get_rect_by_anchor_points(focus_element);
			{
				PvElement *simplify = pv_element_copy_recursive(focus_element);
				pv_assert(simplify);
				info->func_apply_appearances(simplify, focus_element->etaion_work_appearances);
				ra = info->func_get_rect_by_anchor_points(simplify);
				pv_element_free(simplify);
			}
			if(0 == i){
				rect_extent_b = rb;
				rect_extent_a = ra;
			}else{
				rect_extent_b = pv_rect_expand(rect_extent_b, rb);
				rect_extent_a = pv_rect_expand(rect_extent_a, ra);
			}
		}
		if(0 != num){
			rect_extent_b = pv_rect_mul_value(rect_extent_b, render_context.scale);
			rect_extent_a = pv_rect_mul_value(rect_extent_a, render_context.scale);
			pv_cairo_set_source_rgba_workingcolor(cr);
			cairo_rectangle (cr, rect_extent_b.x, rect_extent_b.y, rect_extent_b.w, rect_extent_b.h);
			cairo_rectangle (cr, rect_extent_a.x, rect_extent_a.y, rect_extent_a.w, rect_extent_a.h);
			cairo_stroke(cr);
		}
	}

	// ** tool
	if(NULL != element_overwrite){
		render_option.focus = NULL;
		render_option.render_context.is_focus = false;
		level = 0;
		if(!pv_render_cairo_recursive_(
					cr,
					element_overwrite,
					element_render_context,
					render_option,
					&level)){
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

