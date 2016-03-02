#include "et_doc_manager.h"

#include <stdlib.h>
#include "et_error.h"

EtDocManager *doc_manager = NULL;

bool _et_doc_manager_add_doc(EtDoc *doc);

EtDocManager *et_doc_manager_init()
{
	if(NULL != doc_manager){
		et_bug("");
		exit(-1);
	}

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

EtDocId et_doc_manager_new_doc()
{
	EtDoc *doc = et_doc_new();
	if(NULL == doc){
		et_error("");
		return -1;
	}

	if(!_et_doc_manager_add_doc(doc)){
		et_error("");
		return -1;
	}

	return et_doc_get_id(doc);
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

bool _et_doc_manager_add_doc(EtDoc *doc)
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
	this->doc_nodes[num + 1].doc = NULL;
	this->doc_nodes[num].doc = doc;

	return true;
}

EtDoc *et_doc_manager_get_doc_from_id(const EtDocId doc_id)
{
	EtDocManager *this = doc_manager;
	if(NULL == this){
		et_bug("");
		return NULL;
	}

	int i = 0;
	while(NULL != this->doc_nodes[i].doc){
		if(doc_id == et_doc_get_id(this->doc_nodes[i].doc)){
			return this->doc_nodes[i].doc;
		}
		i++;
	}

	return NULL;
}
