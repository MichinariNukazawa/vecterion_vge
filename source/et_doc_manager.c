#include "et_doc_manager.h"

#include <stdlib.h>
#include "et_error.h"

EtDocManager *doc_manager = NULL;

EtDocManager *et_doc_manager_init()
{
	EtDocManager *this;
	this = (EtDocManager *)malloc(sizeof(EtDocManager));
	if(NULL == this){
		et_critical("");
		return NULL;
	}

	this->doc_nodes[0].doc = NULL;

	doc_manager = this;

	return this;
}

int _et_doc_manager_get_num_doc_node(EtDocNode *doc_nodes)
{
	int i = 0;
	while(NULL != doc_nodes[i].doc){
		i++;
	}

	// Todo: fix freescale
	if(14 < i){
		et_fixme("");
		i = 14;
	}

	return i;
}

bool et_doc_manager_add_doc(EtDoc *doc)
{
	EtDocManager *this = doc_manager;
	if(NULL == this){
		et_bug("");
		return false;
	}

	if(NULL == this->doc_nodes){
		et_bug("");
		return false;
	}

	int num = _et_doc_manager_get_num_doc_node(this->doc_nodes);
	this->doc_nodes[num].doc = doc;

	return true;
}

EtDoc *_et_doc_manager_get_doc_from_id(const EtDocId doc_id)
{
	EtDocManager *this = doc_manager;
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	int i = 0;
	while(NULL != this->doc_nodes[i].doc){
		if(doc_id == this->doc_nodes[i].doc->id){
			return this->doc_nodes[i].doc;
		}
		i++;
	}

	return NULL;
}


bool et_doc_manager_slot_mouse_action(EtDocId id_doc, EtMouseAction mouse_action)
{
	et_debug(" x:%d, y:%d,\n", (int)mouse_action.point.x, (int)mouse_action.point.y);
	EtDoc *doc = _et_doc_manager_get_doc_from_id(id_doc);
	if(NULL == doc){
		et_error("");
		return false;
	}

	et_doc_add_point(doc, mouse_action.point.x, mouse_action.point.y);
	et_doc_draw_canvas(doc);

	return true;
}
