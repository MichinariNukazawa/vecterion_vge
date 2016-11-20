#include "et_mouse_cursor.h"

#include "et_error.h"

static bool _is_init_mouse_cursor_infos = false;

EtMouseCursorInfo _mouse_cursor_infos[] = {
	{
		EtMouseCursorId_Resize_UpRight,
		"resource/cursor/resize_upright_24x24.svg",
		NULL, NULL,
		0, 24,
	},
	{
		EtMouseCursorId_Resize_UpLeft,
		NULL,
		NULL, NULL,
		24, 24,
	},
	{
		EtMouseCursorId_Resize_DownRight,
		NULL,
		NULL, NULL,
		0, 0,
	},
	{
		EtMouseCursorId_Resize_DownLeft,
		NULL,
		NULL, NULL,
		24, 0,
	},
};

static size_t _et_mouse_cursor_get_num()
{
	return sizeof(_mouse_cursor_infos) / sizeof(_mouse_cursor_infos[0]);
}

static GdkPixbuf *_copy_new_pixbuf_reverse_horizontal(const GdkPixbuf *pb_src)
{
	double w = gdk_pixbuf_get_width(pb_src);
	double h = gdk_pixbuf_get_height(pb_src);
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
	et_assert(surface);
	cairo_t *cr = cairo_create (surface);
	et_assert(cr);

	cairo_matrix_t rev_h = {
		-1, 0,
		0, 1,
		w, 0,
	};
	cairo_set_matrix(cr, &rev_h);

	gdk_cairo_set_source_pixbuf (cr, pb_src, 0, 0);
	cairo_paint (cr);

	GdkPixbuf *pb = gdk_pixbuf_get_from_surface(surface, 0, 0, w, h);
	et_assert(pb);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}

bool et_mouse_cursor_info_init()
{
	et_assert(false == _is_init_mouse_cursor_infos);

	et_assertf(gdk_display_get_default(), "GdkDisplay not grub");

	size_t num = _et_mouse_cursor_get_num();
	for(int i = 0; i < (int)num; i++){
		EtMouseCursorInfo *info = &(_mouse_cursor_infos[i]);

		switch(info->id){
			case EtMouseCursorId_Resize_UpRight:
				{
					et_assertf(info->filepath, "%d", info->id);
					GError *error = NULL;
					info->pixbuf = gdk_pixbuf_new_from_file(info->filepath, &error);
					et_assertf(info->pixbuf, "%s, %s",info->filepath, error->message);
				}
				break;
			case EtMouseCursorId_Resize_UpLeft:
			case EtMouseCursorId_Resize_DownRight:
				{
					const EtMouseCursorInfo *info_upright =
						&(_mouse_cursor_infos[EtMouseCursorId_Resize_UpRight]);
					et_assert(info_upright && info_upright->cursor);

					info->pixbuf = _copy_new_pixbuf_reverse_horizontal(info_upright->pixbuf);
					et_assert(info->pixbuf);
				}
				break;
			case EtMouseCursorId_Resize_DownLeft:
				{
					const EtMouseCursorInfo *info_upright =
						&(_mouse_cursor_infos[EtMouseCursorId_Resize_UpRight]);
					et_assert(info_upright && info_upright->cursor);

					info->pixbuf = gdk_pixbuf_copy(info_upright->pixbuf);
					et_assert(info->pixbuf);
				}
				break;
			default:
				{
					et_assertf(false, "%d", info->id);
				}
		}

		info->cursor = gdk_cursor_new_from_pixbuf(
				gdk_display_get_default(),
				info->pixbuf,
				info->center_x, info->center_y);
		et_assert(info->cursor);
	}

	_is_init_mouse_cursor_infos = true;

	return true;
}

const EtMouseCursorInfo *et_mouse_cursor_get_info_from_id(EtMouseCursorId id)
{
	et_assert(_is_init_mouse_cursor_infos);

	size_t num = _et_mouse_cursor_get_num();
	et_assertf((0 <= id && id < num), "%u, %lu", id, num);

	return &(_mouse_cursor_infos[id]);
}

