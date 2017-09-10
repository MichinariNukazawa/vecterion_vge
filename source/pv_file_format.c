#include "pv_file_format.h"

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>

#include <string.h>
#include <strings.h>
#include "pv_error.h"

static const PvFileFormat file_formats[] = {
	{
		PvFormatKind_JPEG,
		"jpg", "jpeg",
		false, false,
	},
	{
		PvFormatKind_JPEG,
		"jpeg", "jpeg",
		false, false,
	},
	{
		PvFormatKind_PNG,
		"png", "png",
		true, false
		},
	{
		PvFormatKind_BMP,
		"bmp", "bmp",
		false, false,
	},
	{
		PvFormatKind_SVG,
		"svg", "svg",
		true,	true,
	},		//!< @todo implement native file format pvvg.
};

size_t get_num_file_formats()
{
	return sizeof(file_formats) / sizeof(file_formats[0]);
}

const PvFileFormat *get_file_format_from_index(int index)
{
	if(index < 0 || (int)get_num_file_formats() <= index){
		return NULL;
	}

	return &file_formats[index];
}
const PvFileFormat *get_file_format_from_extension(const char *extension)
{
	if(NULL == extension){
		return NULL;
	}

	for(int i = 0; i < (int)get_num_file_formats(); i++){
		if(0 == strcasecmp(extension, file_formats[i].extension)){
			return &(file_formats[i]);
		}
	}

	return NULL;
}

const PvFileFormat *get_file_format_from_filepath(const char *filepath)
{
	if(NULL == filepath){
		return NULL;
	}

	char *ext;
	if(NULL == (ext = strrchr(filepath, '.'))){
		pv_error("");
		return NULL;
	}
	ext++;

	return get_file_format_from_extension(ext);
}

char *pv_file_format_change_new_extension_from_filepath(const char *prev_filepath, const char *extension)
{
	if(NULL == prev_filepath){
		prev_filepath = _("untitled_document");
	}

	if(NULL == extension){
		pv_error("");
		extension = "";
	}

	char *tmp_filepath = g_strdup(prev_filepath);
	pv_assert(tmp_filepath);

	char *filename = strrchr(tmp_filepath, G_DIR_SEPARATOR);
	if(NULL == filename){
		filename = tmp_filepath;
	}

	char *ext = strrchr(filename, '.');
	if(NULL != ext){
		*ext = '\0';
	}

	char *next_filepath = g_strdup_printf("%s.%s", tmp_filepath, extension);
	g_free(tmp_filepath);

	return next_filepath;
}

