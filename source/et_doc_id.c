#include "et_doc_id.h"

static int et_doc_id_num = 0; // 割り当て済みIDの数(=未割り当てIDの先頭)

EtDocId et_doc_id_new()
{
	return et_doc_id_num++;
}


