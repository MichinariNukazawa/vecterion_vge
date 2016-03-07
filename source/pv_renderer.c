#include "pv_renderer.h"

// http://stackoverflow.com/questions/7356523/linking-math-library-to-a-c90-code-using-gcc
// http://www.sbin.org/doc/glibc/libc_19.html
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

#include "pv_error.h"

typedef struct PvRenderOption{
	PvRenderContext render_context;
	PvFocus focus;
}PvRenderOption;

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

void _pv_render_workingcolor_cairo_set_source_rgb(cairo_t *cr)
{
	cairo_set_source_rgb (cr, 0.2, 0.4, 0.9);
}

bool _pv_renderer_draw_element_beziers(
		cairo_t *cr,
		const PvRenderOption render_option,
		const PvElement *element
		)
{
	pv_element_debug_print(element);

	PvElementBezierData *data = element->data;
	if(NULL == data){
		pv_error("");
		return false;
	}
	int anchor_points_num = data->anchor_points_num;
	PvAnchorPoint * const anchor_points = data->anchor_points;

	if(anchor_points_num <= 0){
		return true;
	}

	const PvRenderContext render_context = render_option.render_context;

	// ** stroke
	double first_ap_x = 0, first_ap_y = 0, second_ap_x = 0, second_ap_y = 0;
	for(int i = 0; i < anchor_points_num; i++){
		double x = anchor_points[i].points[PvAnchorPointIndex_Point].x;
		double y = anchor_points[i].points[PvAnchorPointIndex_Point].y;

		// cairo_set_source_rgb (cr, 0.1, 0.1, 0.1);
		if(0 == i){
			x *= render_context.scale;
			y *= render_context.scale;

			if(1 == anchor_points_num){
				cairo_rectangle (cr, x, y, 2, 2);
				cairo_fill (cr);
			}else{
				cairo_set_line_width(cr, 2.0);
				cairo_move_to(cr, x, y);
			}
		}else{
			const PvAnchorPoint *ap_prev = &anchor_points[i - 1];
			first_ap_x = ap_prev->points[PvAnchorPointIndex_HandleNext].x
				+ ap_prev->points[PvAnchorPointIndex_Point].x;
			first_ap_y = ap_prev->points[PvAnchorPointIndex_HandleNext].y
				+ ap_prev->points[PvAnchorPointIndex_Point].y;

			second_ap_x = anchor_points[i].points[PvAnchorPointIndex_HandlePrev].x;
			second_ap_y = anchor_points[i].points[PvAnchorPointIndex_HandlePrev].y;
			second_ap_x += x;
			second_ap_y += y;

			x *= render_context.scale;
			y *= render_context.scale;
			first_ap_x *= render_context.scale;
			first_ap_y *= render_context.scale;
			second_ap_x *= render_context.scale;
			second_ap_y *= render_context.scale;
			cairo_curve_to(cr, first_ap_x, first_ap_y, second_ap_x, second_ap_y, x, y);
		/*
			cairo_curve_to(cr, first_ap_x, first_ap_y,
					second_ap_x, second_ap_y, x, y);
			*/
		}

	}
	if(data->is_close){
		double x = anchor_points[0].points[PvAnchorPointIndex_Point].x;
		double y = anchor_points[0].points[PvAnchorPointIndex_Point].y;

		const PvAnchorPoint *ap_last = &anchor_points[anchor_points_num - 1];
		first_ap_x = ap_last->points[PvAnchorPointIndex_HandleNext].x
			+ ap_last->points[PvAnchorPointIndex_Point].x;
		first_ap_y = ap_last->points[PvAnchorPointIndex_HandleNext].y
			+ ap_last->points[PvAnchorPointIndex_Point].y;

		const PvAnchorPoint *ap_first = &anchor_points[0];
		second_ap_x = ap_first->points[PvAnchorPointIndex_HandlePrev].x
			+ ap_first->points[PvAnchorPointIndex_Point].x;
		second_ap_y = ap_first->points[PvAnchorPointIndex_HandlePrev].y
			+ ap_first->points[PvAnchorPointIndex_Point].y;

		first_ap_x *= render_context.scale;
		first_ap_y *= render_context.scale;
		second_ap_x *= render_context.scale;
		second_ap_y *= render_context.scale;
		x *= render_context.scale;
		y *= render_context.scale;
		cairo_curve_to(cr, first_ap_x, first_ap_y, second_ap_x, second_ap_y, x, y);
		cairo_close_path (cr);
	}
	cairo_stroke(cr);

	// ** anchor point
	int ix = ((data->is_close)? 0 : anchor_points_num - 1);
	double x = anchor_points[ix].points[PvAnchorPointIndex_Point].x;
	double y = anchor_points[ix].points[PvAnchorPointIndex_Point].y;
	double prev_ap_x = anchor_points[ix]
				.points[PvAnchorPointIndex_HandlePrev].x;
	double prev_ap_y = anchor_points[ix]
				.points[PvAnchorPointIndex_HandlePrev].y;
	double next_ap_x = anchor_points[ix]
				.points[PvAnchorPointIndex_HandleNext].x;
	double next_ap_y = anchor_points[ix]
				.points[PvAnchorPointIndex_HandleNext].y;
	prev_ap_x += x;
	prev_ap_y += y;
	next_ap_x += x;
	next_ap_y += y;
	x *= render_context.scale;
	y *= render_context.scale;
	prev_ap_x *= render_context.scale;
	prev_ap_y *= render_context.scale;
	next_ap_x *= render_context.scale;
	next_ap_y *= render_context.scale;

	cairo_set_line_width(cr, 1.0);
	_pv_render_workingcolor_cairo_set_source_rgb(cr);
	cairo_move_to(cr, x, y);
	cairo_line_to(cr, prev_ap_x, prev_ap_y);
	cairo_stroke(cr);
	cairo_move_to(cr, x, y);
	cairo_line_to(cr, next_ap_x, next_ap_y);
	cairo_stroke(cr);

	_pv_render_workingcolor_cairo_set_source_rgb(cr);
	cairo_arc (cr, prev_ap_x, prev_ap_y, 2.0, 0., 2 * M_PI);
	cairo_arc (cr, next_ap_x, next_ap_y, 2.0, 0., 2 * M_PI);
	//cairo_rectangle (cr, prev_ap_x, prev_ap_y, 4, 4);
	//cairo_rectangle (cr, next_ap_x, next_ap_y, 4, 4);
	cairo_fill (cr);

	return true;
}

bool _pv_renderer_cairo_recersive(
		cairo_t *cr,
		PvElement * const element,
		const PvRenderOption render_option,
		int *level)
{
	const PvRenderContext render_context = render_option.render_context;

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
		if(render_option.focus.element == element){
			_pv_render_workingcolor_cairo_set_source_rgb(cr);
		}else{
			cairo_set_source_rgb (cr, 0.1, 0.1, 0.1);
		}
		switch(element->kind){
			case PvElementKind_Raster:
				{
					PvElementRasterData *data = element->data;
					GdkPixbuf *pixbuf = data->pixbuf;
					double w = (double)gdk_pixbuf_get_width(pixbuf);
					double h = (double)gdk_pixbuf_get_height(pixbuf);
					w *= render_context.scale;
					h *= render_context.scale;
					GdkPixbuf *pb = gdk_pixbuf_scale_simple(
							pixbuf,
							(int)w, (int)h,
							GDK_INTERP_HYPER);
					if(NULL == pb){
						pv_error("");
						ret = false;
						goto end;
					}else{
						gdk_cairo_set_source_pixbuf (cr, pb, 0, 0);
						cairo_paint (cr);
						g_object_unref(G_OBJECT(pb));
					}
				}
				break;
			case PvElementKind_Bezier:
				{
					if(!_pv_renderer_draw_element_beziers(
								cr,
								render_option,
								element
								)){
						pv_error("");
						ret = false;
						goto end;
					}
				}
				break;
			default:
				pv_bug("%d\n", element->kind);
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
	pv_debug("%f, %f, %f, %f, \n", (vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);
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

	GdkPixbuf *pb = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
	if(NULL == pb){
		pv_error("");
		return NULL;
	}

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}
