#include "et_doc.h"

#include <stdlib.h>
#include "et_error.h"

EtDoc *et_doc_new()
{
	EtDoc *this = (EtDoc *)malloc(sizeof(EtDoc));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	for(int i = 0; i < 10; i++){
		this->points[i].x = -1.0;
		this->points[i].y = -1.0;
	}

	this->callback_draws =
		(EtDocCallback *)malloc(sizeof(EtDocCallback) * 1);
	if(NULL == this->callback_draws){
		et_error("");
		return NULL;
	}
	this->callback_draws[0].id = -1;

	this->pixbuf = NULL;

	return this;
}

int _et_doc_get_num_callback_draws(EtDocCallback *callback_draws){
	int i = 0;
	while(0 <= callback_draws[i].id){
		i++;
	}

	return i;
}

bool et_doc_set_image_from_file(EtDoc *this, const char *filepath)
{
	this->pixbuf = gdk_pixbuf_new_from_file(filepath, NULL);
	if(NULL == this->pixbuf){
		et_error("");
		return false;
	}

	int num = _et_doc_get_num_callback_draws(this->callback_draws);
	for(int i = 0; i < num; i++){
		this->callback_draws[i].func(this, this->callback_draws[i].data);
	}

	return true;
}

EtCallbackId et_doc_add_draw_callback(EtDoc *this, EtDocDrawCallback func, gpointer data)
{
	int num = _et_doc_get_num_callback_draws(this->callback_draws);
	EtDocCallback *new = realloc(this->callback_draws,
					sizeof(EtDocCallback) * (num + 2));
	if(NULL == new){
		return -1;
	}
	new[num + 1].id = -1;
	new[num].id = 1; // Todo: identific number.
	new[num].func = func;
	new[num].data = data;

	this->callback_draws = new;

	// Todo: create EtCallbackId
	return new[num].id;
}

GdkPixbuf *et_doc_get_pixbuf(EtDoc *this){
	return this->pixbuf;
}
