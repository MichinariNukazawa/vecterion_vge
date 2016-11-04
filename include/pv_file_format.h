#ifndef include_PV_FILE_FORMAT_H
#define include_PV_FILE_FORMAT_H

#include <stdbool.h>
#include <stddef.h>

typedef enum{
	PvFormatKind_JPEG,
	PvFormatKind_PNG,
	PvFormatKind_BMP,
	PvFormatKind_SVG,
	PvFormatKind_OTHER,
}PvFormatKind;

typedef struct{
	PvFormatKind kind;
	const char *extension;
	const char *gdk_file_type;	//!< gdk_pixbuf_save_*() type(ex."jpeg") or other(ex. "svg")
	bool has_alpha;
	bool is_native;			//!< pvvg(etaion) native file format @fixme need implement!
}PvFileFormat;


size_t get_num_file_formats();
const PvFileFormat *get_file_format_from_index(int index);
const PvFileFormat *get_file_format_from_extension(const char *extension);
const PvFileFormat *get_file_format_from_filepath(const char *filepath);
char *pv_file_format_change_new_extension_from_filepath(const char *prev_filepath, const char *extension);


#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_FILE_FORMAT_H

