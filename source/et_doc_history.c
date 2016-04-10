#include "et_doc_history.h"

#include <stdlib.h>
#include <string.h>
#include "et_error.h"

const int _et_doc_history_num_history = 128;

EtDocHistory *et_doc_history_new(const PvVg *vg)
{
	if(NULL == vg){
		et_bug("");
		return NULL;
	}

	EtDocHistory *this = (EtDocHistory *)malloc(sizeof(EtDocHistory));
	if(NULL == this){
		et_critical("");
		return NULL;
	}

	this->ix_current = 0;
	this->ix_redo = 0;
	this->ix_undo = 0;
	this->num_history = _et_doc_history_num_history;

	this->hists = (EtDocHistoryItem *)malloc(sizeof(EtDocHistoryItem) * this->num_history);
	if(NULL == this->hists){
		et_critical("");
		return NULL;
	}
	for(int ix = 0; ix < this->num_history; ix++){
		this->hists[ix].vg = NULL;
		this->hists[ix].focus = NULL;
	}

	PvVg *vg_new0 = pv_vg_copy_new(vg);
	if(NULL == vg_new0){
		et_error("");
		return NULL;
	}
	PvVg *vg_new1 = pv_vg_copy_new(vg);
	if(NULL == vg_new1){
		et_error("");
		return NULL;
	}
	PvFocus *focus_new0 = pv_focus_new(vg_new0);
	if(NULL == focus_new0){
		et_error("");
		return NULL;
	}
	PvFocus *focus_new1 = pv_focus_new(vg_new1);
	if(NULL == focus_new1){
		et_error("");
		return NULL;
	}
	this->hists[0].vg = vg_new0;
	this->hists[0].focus = focus_new0;
	this->hist_work.vg = vg_new1;
	this->hist_work.focus = focus_new1;

	return this;
}

/** @brief
 * @return is nusefe (index history enable is not check.)
 */
int _et_doc_history_get_index_from_relative(
		const EtDocHistory *hist,
		int ix_base,
		int relative,
		int max)
{
	if(0 == max){
		et_bug("");
		return 0;
	}

	if((NULL != hist) && (relative < 0)){
		if(pv_vg_is_diff(hist->hist_work.vg, hist->hists[hist->ix_current].vg))
		{
			relative += 1;
		}
	}

	int ix = ix_base + relative;
	while(max <= ix){
		ix -= max;
	}
	while(ix < 0){
		ix += max;
	}

	return ix;
}

bool _et_doc_history_is_extent_enable(EtDocHistory *hist, int ix)
{
	if(!(0 <= ix && ix < hist->num_history)){
		return false;
	}

	if(hist->ix_undo <= hist->ix_redo){
		if(!(hist->ix_undo <= ix && ix <= hist->ix_redo)){
			return false;
		}
	}else{
		if(!(ix <= hist->ix_redo || hist->ix_undo <= ix)){
			return false;
		}
	}

	return true;
}

EtDocHistoryItem *et_doc_history_get_from_relative(EtDocHistory *hist, int relative)
{
	// et_debug("curr:%d rel:%d", hist->ix_current, relative);

	if(0 == relative){
		return &(hist->hist_work);
	}

	int ix = _et_doc_history_get_index_from_relative(
			hist,
			hist->ix_current,
			relative,
			hist->num_history);

	if(! _et_doc_history_is_extent_enable(hist, ix)){
		goto error;
	}

	return &(hist->hists[ix]);
error:
	et_error("ix:%d, rel:%d undo:%d, curr:%d, redo:%d, num:%d",
			ix, relative,
			hist->ix_undo, hist->ix_current, hist->ix_redo,
			hist->num_history);
	return NULL;
}

int et_doc_history_element_get_index(const PvElement *element)
{
	PvElement *parent = element->parent;
	if(NULL == parent){
		et_error("");
		return -1;
	}
	for(int i = 0; NULL != parent->childs[i]; i++){
		if(element == parent->childs[i]){
			return i;
		}
	}

	return -1;
}

int *et_doc_history_element_get_indexes(const PvElement *element)
{
	if(NULL == element){
		// et_warning("");
		return NULL;
	}

	int level = 0;
	int *indexes = NULL;
	const PvElement *_element = element;
	while(NULL != _element->parent){
		int ix = et_doc_history_element_get_index(_element);
		if(ix < 0){
			et_error("");
			goto error;
		}
		int *new = realloc(indexes, sizeof(int) * (level + 2));
		if(NULL == new){
			et_error("");
			goto error;
		}else{
			memmove(&new[1], &new[0], sizeof(int) * (level + 1));
			indexes = new;
		}

		indexes[level + 1] = -1;
		indexes[0] = ix;
		level++;

		_element = _element->parent;
	}

	return indexes;
error:
	free(indexes);
	return NULL;
}

PvElement *et_doc_element_get_tree_from_indexes(
		PvElement *element_root,
		const int *indexes)
{

	if(NULL == indexes){
		// NULL indexes is fine.
		return NULL;
	}

	for(int level = 0; 0 <= indexes[level]; level++){
		printf("%d,", indexes[level]);
	}
	printf("\n");

	PvElement *_element = element_root;
	for(int level = 0; 0 <= indexes[level]; level++){
		int num = pv_general_get_parray_num((void **)_element->childs);
		if(!(indexes[level] < num)){
			et_warning("level:%d ix:%d num:%d",
					level, indexes[level], num);
			return NULL;
		}

		_element = _element->childs[indexes[level]];
	}

	return _element;
}

PvFocus *_et_doc_history_copy_focus_by_vg(const PvFocus *focus_src, const PvVg *vg_dst)
{
	PvFocus *focus_dst = pv_focus_new(vg_dst);
	if(NULL == focus_dst){
		et_error("");
		return NULL;
	}

	// search focus element.
	int *indexes = NULL;
	const PvElement *focus_element = pv_focus_get_first_element(focus_src);
	if(NULL != focus_element){
		// ** get focus path(index's)
		indexes = et_doc_history_element_get_indexes(
				focus_element);
		if(NULL == indexes){
			et_error("");
			goto error;
		}

		PvElement *elem = et_doc_element_get_tree_from_indexes(
				vg_dst->element_root,
				indexes);
		pv_focus_clear_set_element(focus_dst, elem);
	}

	free(indexes);
	return focus_dst;
error:
	pv_focus_free(focus_dst);
	free(indexes);
	return NULL;
}

void _et_doc_hist_free_history(EtDocHistory *hist, int ix)
{
	pv_vg_free(hist->hists[ix].vg);
	pv_focus_free(hist->hists[ix].focus);
	hist->hists[ix].vg = NULL;
	hist->hists[ix].focus = NULL;
}

bool _et_doc_hist_free_redo_all(EtDocHistory *hist)
{
	if(hist->ix_redo < hist->ix_current){
		for(; 0 <= hist->ix_redo; (hist->ix_redo)--){
			_et_doc_hist_free_history(hist, hist->ix_redo);
		}
		hist->ix_redo = hist->num_history - 1;
	}
	for(;hist->ix_current < hist->ix_redo; (hist->ix_redo)--){
		_et_doc_hist_free_history(hist, hist->ix_redo);
	}

	return true;
}

bool _et_doc_hist_free_history_next(EtDocHistory *hist)
{
	// ** このあと、current == redo 前提で記述する
	if(hist->ix_current != hist->ix_redo){
		// reodがあるならば、削除して続行
		et_bug("");
		if(!_et_doc_hist_free_redo_all(hist)){
			et_error("");
			return false;
		}
	}

	int ix = _et_doc_history_get_index_from_relative(
			hist,
			hist->ix_current,
			1,
			hist->num_history);

	if(! _et_doc_history_is_extent_enable(hist, ix)){
		return true;
	}

	_et_doc_hist_free_history(hist, ix);
	int ix2 = _et_doc_history_get_index_from_relative(
			hist,
			ix,
			1,
			hist->num_history);
	hist->ix_undo = ix2;

	return true;


	// TODO: not implement.
	return true;
}

bool _et_doc_history_copy_hist(
		EtDocHistoryItem *item_dst,
		const EtDocHistoryItem *item_src)
{
	if(NULL == item_dst){
		et_bug("");
		return false;
	}
	if(NULL == item_src){
		et_bug("");
		return false;
	}
	if(NULL == item_src->vg){
		et_bug("");
		return false;
	}

	// ** copy vg
	PvVg *vg_new = pv_vg_copy_new(item_src->vg);
	if(NULL == vg_new){
		et_error("");
		return false;
	}
	// ** restructed focus.
	PvFocus *focus_new = _et_doc_history_copy_focus_by_vg(
				item_src->focus,
				vg_new);
	if(NULL == focus_new){
		et_error("");
		return false;
	}

	if(NULL != item_dst->vg){
		pv_vg_free(item_dst->vg);
		pv_focus_free(item_dst->focus);
	}
	item_dst->vg = vg_new;
	item_dst->focus = focus_new;

	return true;
}

bool _et_doc_history_copy_hist_by_index(EtDocHistory *hist, int ix_dst, int ix_src)
{
	return _et_doc_history_copy_hist(
			&(hist->hists[ix_dst]),
			&(hist->hists[ix_src]));
}

bool et_doc_history_save_with_focus(EtDocHistory *hist)
{
	et_debug("undo:%3d, curr:%3d, redo:%3d, num:%3d",
			hist->ix_undo, hist->ix_current, hist->ix_redo,
			hist->num_history);

	int ix = _et_doc_history_get_index_from_relative(
			NULL,
			hist->ix_current,
			1,
			hist->num_history);

	// ** save skip because no change.
	if(FALSE == pv_vg_is_diff(hist->hist_work.vg, hist->hists[hist->ix_current].vg)){
		return true;
	}

	// ** currentより先のredoをすべて削除
	if(!_et_doc_hist_free_redo_all(hist)){
		et_error("");
		return false;
	}
	// ** 次のUndoを置く領域を(無ければ)一件削除して開ける
	if(!_et_doc_hist_free_history_next(hist)){
		et_error("");
		return false;
	}

	if(!_et_doc_history_copy_hist(&(hist->hists[ix]), &(hist->hist_work))){
		et_debug("");
		return false;
	}

	// **
	hist->ix_current = ix;
	hist->ix_redo = ix;

	return true;
}

/* @brief (存在するならば)currentを1つ過去にずらす */
bool et_doc_history_undo(EtDocHistory *hist)
{
	et_debug("undo:%3d, curr:%3d, redo:%3d, num:%3d",
			hist->ix_undo, hist->ix_current, hist->ix_redo,
			hist->num_history);

	if(0 >= et_doc_history_get_num_undo(hist)){
		et_debug("undo history is empty.");
		return false;
	}

	// ** 変更があればそれをsaveしておく
	if(TRUE == pv_vg_is_diff(hist->hist_work.vg, hist->hists[hist->ix_current].vg)){
		if(!et_doc_history_save_with_focus(hist)){
			et_debug("");
			return false;
		}
	}

	int ix = _et_doc_history_get_index_from_relative(
			hist,
			hist->ix_current,
			-1,
			hist->num_history);
	if(!_et_doc_history_is_extent_enable(hist, ix)){
		et_error("%d", ix);
		return false;
	}
	hist->ix_current = ix;

	if(!_et_doc_history_copy_hist(&(hist->hist_work), &(hist->hists[ix]))){
		et_error("%d", ix);
		return false;
	}

	return true;
}

/* @brief (存在するならば)currentを1つ未来にずらす */
bool et_doc_history_redo(EtDocHistory *hist)
{
	et_debug("undo:%3d, curr:%3d, redo:%3d, num:%3d",
			hist->ix_undo, hist->ix_current, hist->ix_redo,
			hist->num_history);

	if(et_doc_history_get_num_redo(hist) <= 0){
		et_debug("redo history is empty.");
		return false;
	}

	int ix = _et_doc_history_get_index_from_relative(
			hist,
			hist->ix_current,
			1,
			hist->num_history);
	if(!_et_doc_history_is_extent_enable(hist, ix)){
		et_error("%d", ix);
		return false;
	}
	hist->ix_current = ix;

	if(!_et_doc_history_copy_hist(&(hist->hist_work), &(hist->hists[ix]))){
		et_error("%d", ix);
		return false;
	}

	return true;
}

int et_doc_history_get_num_undo(EtDocHistory *hist)
{
	int num = hist->ix_current - hist->ix_undo;
	if(num < 0){
		num += hist->num_history;
	}

	if(pv_vg_is_diff(hist->hist_work.vg, hist->hists[hist->ix_current].vg)){
		num += 1;
	}

	return num;
}

int et_doc_history_get_num_redo(EtDocHistory *hist)
{
	// ** check hist_work change.
	if(pv_vg_is_diff(hist->hist_work.vg, hist->hists[hist->ix_current].vg)){
		// droping redo history when save.
		return 0;
	}
	int num = hist->ix_redo - hist->ix_current;
	if(num < 0){
		num += hist->num_history;
	}

	return num;
}

void et_doc_history_debug_print(const EtDocHistory *hist){
	et_debug("undo:%3d, curr:%3d, redo:%3d, num:%3d",
			hist->ix_undo, hist->ix_current, hist->ix_redo,
			hist->num_history);
}

