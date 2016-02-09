#include "pv_renderer.h"

#include "pv_error.h"

bool _pv_renderer_is_group_kind(PvElement * const element)
{
	switch(element->kind){
		case PvElementKind_Layer:
		case PvElementKind_Group:
			return true;
			break;
		default:
			return false;
	}
}

bool _pv_renderer_cairo_anchor_points(
		cairo_t *cr, const PvRenderContext render_context,
		int anchor_points_num, PvAnchorPoint * const anchor_points
		)
{
	for(int i = 0; i < anchor_points_num; i++){
		// Todo: ポイントを繋げていない
		// Todo: ハンドルを見ていない
		double x = anchor_points[i].points[PvAnchorPointIndex_Point].x;
		double y = anchor_points[i].points[PvAnchorPointIndex_Point].y;
		x *= render_context.scale;
		y *= render_context.scale;
		cairo_set_source_rgb (cr, 0.1, 0.1, 0.1);
		if(0 == i){
			if(1 == anchor_points_num){
				cairo_rectangle (cr, x, y, 2, 2);
				cairo_fill (cr);
			}else{
				cairo_set_line_width(cr, 2.0);
				cairo_move_to(cr, x, y);
			}
		}else{
			cairo_line_to(cr, x, y);
		}
	}
	cairo_stroke(cr);

	return true;
}

bool _pv_renderer_cairo_recersive(
		cairo_t *cr, const PvRenderContext render_context, 
		PvElement * const element, int *level)
{
	bool ret = true;
	(*level)++;

	if(_pv_renderer_is_group_kind(element)){
		int num = pv_general_get_parray_num((void **)element->childs);
		for(int i = 0; i < num; i++){
			if(!_pv_renderer_cairo_recersive(cr, render_context,
						element->childs[i], level)){
				pv_error("%d\n", i);
				ret = false;
				goto end;
			}
		}
	}else{
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
					PvElementBezierData *data = element->data;
					if(!_pv_renderer_cairo_anchor_points(
								cr,
								render_context,
								data->anchor_points_num,
								data->anchor_points)){
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

GdkPixbuf *pv_renderer_pixbuf_from_vg(PvVg * const vg,
		const PvRenderContext render_context)
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

	int level = 0;
	if(!_pv_renderer_cairo_recersive(cr, render_context, vg->element_root, &level)){
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

