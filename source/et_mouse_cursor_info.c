#include "et_mouse_cursor_info.h"

#include "et_error.h"

// http://stackoverflow.com/questions/7356523/linking-math-library-to-a-c90-code-using-gcc
// http://www.sbin.org/doc/glibc/libc_19.html
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

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
	{
		EtMouseCursorId_Rotate_UpRight,
		"resource/cursor/rotate_upright_24x24.svg",
		NULL, NULL,
		0, 24,
	},
	{
		EtMouseCursorId_Rotate_UpLeft,
		NULL,
		NULL, NULL,
		24, 24,
	},
	{
		EtMouseCursorId_Rotate_DownRight,
		NULL,
		NULL, NULL,
		0, 0,
	},
	{
		EtMouseCursorId_Rotate_DownLeft,
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

static GdkPixbuf *_copy_new_pixbuf_rotate(const GdkPixbuf *pb_src, int rotate)
{
	double w = gdk_pixbuf_get_width(pb_src);
	double h = gdk_pixbuf_get_height(pb_src);
	cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, w, h);
	et_assert(surface);
	cairo_t *cr = cairo_create (surface);
	et_assert(cr);

	cairo_translate(cr, w/2, h/2);
	cairo_rotate(cr, rotate * (M_PI/180));
	cairo_translate(cr, -w/2, -h/2);

	gdk_cairo_set_source_pixbuf (cr, pb_src, 0, 0);
	cairo_paint (cr);

	GdkPixbuf *pb = gdk_pixbuf_get_from_surface(surface, 0, 0, w, h);
	et_assert(pb);

	cairo_surface_destroy (surface);
	cairo_destroy (cr);

	return pb;
}

static bool et_mouse_cursor_info_init_inline_(
		const char *dirpath_application_base,
		bool is_test)
{
	et_assert(!_is_init_mouse_cursor_infos);

	size_t num = _et_mouse_cursor_get_num();
	for(int i = 0; i < (int)num; i++){
		EtMouseCursorInfo *info = &(_mouse_cursor_infos[i]);

		switch(info->id){
			case EtMouseCursorId_Resize_UpRight:
			case EtMouseCursorId_Rotate_UpRight:
				{
					et_assertf(info->filepath, "%d", info->id);
					char *path = g_strdup_printf("%s/%s", dirpath_application_base, info->filepath);
					et_assertf(path, "'%s'", info->filepath);
					GError *error = NULL;
					info->pixbuf = gdk_pixbuf_new_from_file(path, &error);
					et_assertf(info->pixbuf, "%s, %s", info->filepath, error->message);
					free(path);
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
			case EtMouseCursorId_Rotate_UpLeft:
				{
					const EtMouseCursorInfo *info_upright =
						&(_mouse_cursor_infos[EtMouseCursorId_Rotate_UpRight]);
					et_assert(info_upright && info_upright->cursor);

					info->pixbuf = _copy_new_pixbuf_reverse_horizontal(info_upright->pixbuf);
					et_assert(info->pixbuf);
				}
				break;
			case EtMouseCursorId_Rotate_DownRight:
				{
					const EtMouseCursorInfo *info_upright =
						&(_mouse_cursor_infos[EtMouseCursorId_Rotate_UpRight]);
					et_assert(info_upright && info_upright->cursor);

					info->pixbuf = _copy_new_pixbuf_rotate(info_upright->pixbuf, 90);
					et_assert(info->pixbuf);
				}
				break;
			case EtMouseCursorId_Rotate_DownLeft:
				{
					const EtMouseCursorInfo *info_upright =
						&(_mouse_cursor_infos[EtMouseCursorId_Rotate_UpRight]);
					et_assert(info_upright && info_upright->cursor);

					info->pixbuf = _copy_new_pixbuf_rotate(info_upright->pixbuf, 180);
					et_assert(info->pixbuf);
				}
				break;
			default:
				{
					et_abortf("%d", info->id);
				}
		}

		if(is_test){
			info->cursor = (GdkCursor *)(1);
		}else{
			et_assertf(gdk_display_get_default(), "GdkDisplay not grub");
#ifndef TARGET_OS_WIN
			info->cursor = gdk_cursor_new_from_pixbuf(
					gdk_display_get_default(),
					info->pixbuf,
					info->center_x, info->center_y);
#else
			info->cursor = gdk_cursor_new_from_pixbuf(
					gdk_display_get_default(),
					info->pixbuf,
					0, 0);
#endif
		}
		et_assert(info->cursor);
	}

	_is_init_mouse_cursor_infos = true;

	return true;
}

bool et_mouse_cursor_info_init(const char *dirpath_application_base)
{
	return et_mouse_cursor_info_init_inline_(dirpath_application_base, false);
}

bool et_mouse_cursor_info_init_for_unittest(const char *dirpath_application_base)
{
	return et_mouse_cursor_info_init_inline_(dirpath_application_base, true);
}

const EtMouseCursorInfo *et_mouse_cursor_get_info_from_id(EtMouseCursorId id)
{
	et_assert(_is_init_mouse_cursor_infos);

	size_t num = _et_mouse_cursor_get_num();
	et_assertf((0 <= id && id < num), "%u, %zu", id, num);

	return &(_mouse_cursor_infos[id]);
}

