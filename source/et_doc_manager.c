#include "et_doc_manager.h"

#include <stdlib.h>
#include "et_error.h"
#include "pv_general.h"


struct EtDocNode;
typedef struct EtDocNode EtDocNode;

struct EtDocNode{
	EtDoc *doc;
};

struct EtDocManager{
	EtDocNode **doc_nodes;
};


EtDocManager *doc_manager = NULL;

static bool _et_doc_manager_add_doc(EtDoc *doc);

EtDocManager *et_doc_manager_init()
{
	if(NULL != doc_manager){
		et_bug("");
		exit(-1);
	}

	EtDocManager *self = (EtDocManager *)malloc(sizeof(EtDocManager));
	if(NULL == self){
		et_critical("");
		return NULL;
	}

	self->doc_nodes = NULL;

	doc_manager = self;

	return self;
}

EtDocId et_doc_manager_new_doc_from_vg(const PvVg *vg)
{
	EtDoc *doc = et_doc_new_from_vg(vg);
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

static bool _et_doc_manager_add_doc(EtDoc *doc)
{
	EtDocManager *self = doc_manager;
	et_assert(self);

	EtDocNode *node = malloc(sizeof(EtDocNode));
	et_assert(node);
	node->doc = doc;

	int num = pv_general_get_parray_num((void **)self->doc_nodes);
	EtDocNode **new_nodes = realloc(self->doc_nodes, sizeof(EtDocNode *) * (num + 2));
	et_assert(new_nodes);
	new_nodes[num + 1] = NULL;
	new_nodes[num + 0] = node;
	self->doc_nodes = new_nodes;

	return true;
}

EtDoc *et_doc_manager_get_doc_from_id(const EtDocId doc_id)
{
	EtDocManager *self = doc_manager;
	if(NULL == self){
		et_bug("");
		return NULL;
	}

	size_t num = pv_general_get_parray_num((void **)self->doc_nodes);
	for(int i = 0; i < (int)num; i++){
		if(doc_id == et_doc_get_id(self->doc_nodes[i]->doc)){
			return self->doc_nodes[i]->doc;
		}
	}

	return NULL;
}

