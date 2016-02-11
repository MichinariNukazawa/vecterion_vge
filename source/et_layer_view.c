#include "et_layer_view.h"

#include <stdlib.h>
#include <string.h>
#include "et_error.h"
#include "et_doc_manager.h"
#include "pv_element.h"


struct _EtLayerView{
	EtDocId doc_id;
};

typedef struct _EtLayerViewRltData{
	int level;
	char *name;
	PvElementKind kind;
	unsigned long debug_pointer_value;
}EtLayerViewRltData;

typedef struct _EtLayerViewRltDataPack{
	EtLayerViewRltData **datas;
}EtLayerViewRltDataPack;


static EtLayerView *layer_view = NULL;

EtLayerView *et_layer_view_init()
{
	if(NULL != layer_view){
		et_bug("");
		exit(-1);
	}

	EtLayerView *this = (EtLayerView *)malloc(sizeof(EtLayerView));
	if(NULL == this){
		et_error("");
		return NULL;
	}

	this->doc_id = -1;

	layer_view = this;

	return this;
}

bool _et_layer_view_read_layer_tree(PvElement *element, gpointer data, int level)
{
	EtLayerViewRltDataPack *func_rlt_data_pack = data;
	EtLayerViewRltData ***datas = &(func_rlt_data_pack->datas);

	EtLayerViewRltData *rlt = malloc(sizeof(EtLayerViewRltData));
	if(NULL == rlt){
		et_error("");
		exit(-1);
	}
	rlt->level = level;
	rlt->name = "";
	rlt->kind = element->kind;
	memcpy(&(rlt->debug_pointer_value), &element, sizeof(unsigned long));

	int num = pv_general_get_parray_num((void **)*datas);
	*datas = (EtLayerViewRltData **)
			realloc(*datas, sizeof(EtLayerViewRltData *) * (num + 2));
	if(NULL == *datas){
		et_critical("");
		exit(-1);
	}

	(*datas)[num] = rlt;
	(*datas)[num + 1] = NULL;
	
	return true;
}

bool _et_layer_view_slot_update_doc()
{
	EtLayerView *this = layer_view;
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	EtDoc *doc = et_doc_manager_get_doc_from_id(this->doc_id);
	if(NULL == doc){
		et_error("");
		return false;
	}

	PvVg *vg = et_doc_get_vg_ref(doc);
	if(NULL == vg){
		et_error("");
		return false;
	}

	EtLayerViewRltDataPack func_rlt_data_pack = {
		.datas = NULL
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive(vg->element_root,
				_et_layer_view_read_layer_tree, &func_rlt_data_pack,
				&error)){
		et_error("level:%d", error.level);
		return false;
	}

	// Todo: show window.
	et_debug("\n");
	int num = pv_general_get_parray_num((void **)func_rlt_data_pack.datas);
	for(int i = 0; i < num; i++){
		EtLayerViewRltData *data = func_rlt_data_pack.datas[i];
		printf("%03d:%03d:%08x '%s'\n",
				data->level, data->kind,
				data->debug_pointer_value,
				((data->name)?"":data->name));
		free(data);
	}

	return true;
}

bool et_layer_view_set_doc_id(EtLayerView *this, EtDocId doc_id)
{
	if(NULL == this){
		et_bug("");
		exit(-1);
	}

	this->doc_id = doc_id;

	if(!_et_layer_view_slot_update_doc()){
		et_error("");
		return false;
	}

	return true;
}

void et_layer_view_slot_from_doc_change(EtDoc *doc, gpointer data)
{
	if(!_et_layer_view_slot_update_doc()){
		et_error("");
	}
}

