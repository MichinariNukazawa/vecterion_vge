#include "pv_urischeme.h"

#include <errno.h>
#include <string.h>
#include <glib.h>
#include "pv_error.h"
#include "pv_file_format.h"

char *pv_urischeme_get_from_image_filepath(const char *filepath)
{
	const PvFileFormat *format = get_file_format_from_filepath(filepath);
	if(!format){
		pv_error("'%s'", filepath);
		return NULL;
	}

	char *urischeme_str = NULL;
	unsigned char *data = NULL;
	char *base64_str = NULL;
	FILE *fp = NULL;
	if (NULL == (fp = fopen(filepath, "r"))){
		pv_error("'%s'", filepath);
		return NULL;
	}

	errno = 0;
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if(0 >= filesize){
		pv_error("%s %d '%s'", strerror(errno), filesize, filepath);
		goto failed1;
	}

	data = calloc(filesize, sizeof(unsigned char));
	if(filesize != (int)fread(data, sizeof(unsigned char), filesize, fp)){
		pv_error("'%s'", filepath);
		goto failed1;
	}

	base64_str = g_base64_encode(data, filesize);
	pv_assert(base64_str);

	urischeme_str = g_strdup_printf("data:image/%s;base64;,%s", format->extension, base64_str);
	pv_assert(urischeme_str);

failed1:
	if(NULL != base64_str){
		g_free(base64_str);
	}
	fclose(fp);
	free(data);

	return urischeme_str;
}

