#include "et_doc_id.h"

#include "et_error.h"

static int et_doc_id_num = 0; // 割り当て済みIDの数(=未割り当てIDの先頭)

EtDocId et_doc_id_new()
{
	et_debug("%d", et_doc_id_num);

	return et_doc_id_num++;
}

