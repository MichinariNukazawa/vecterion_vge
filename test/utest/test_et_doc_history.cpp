#include <gtest/gtest.h>
#include <stdio.h>

extern "C"
{
#include "et_doc_history_hive.h"
}

TEST(Test, Test){
	ASSERT_EQ(1,1);
}

TEST(Test, History){
	PvVg *vg = pv_vg_new();
	if(NULL == vg){
		FAIL();
	}
	EtDocHistoryHive *hist;
	hist = et_doc_history_hive_new(NULL);
	ASSERT_TRUE(NULL == hist);
	hist = et_doc_history_hive_new(vg);
	ASSERT_TRUE(NULL != hist);
}

TEST(test, hist_start){
	PvVg *vg = pv_vg_new();
	if(NULL == vg){
		FAIL();
	}
	EtDocHistoryHive *hist;
	hist = et_doc_history_hive_new(vg);
	if(NULL == hist){
		FAIL();
	}

	// ** 初期状態のチェック
	int num;
	num = et_doc_history_hive_get_num_undo(hist);
	ASSERT_EQ(0, num);
	num = et_doc_history_hive_get_num_redo(hist);
	ASSERT_EQ(0, num);

	EtDocHistory *item = NULL;
	item = et_doc_history_get_from_relative(hist, 0);
	ASSERT_TRUE(NULL != item);
	ASSERT_TRUE(!pv_vg_is_diff(vg, item->vg)); // check equal

	item = et_doc_history_get_from_relative(hist, -1);
	ASSERT_TRUE(NULL == item);
	item = et_doc_history_get_from_relative(hist, 1);
	ASSERT_TRUE(NULL == item);

	bool ret;
	// Nothing history undo, redo
	ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
	ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
	ret = et_doc_history_hive_undo(hist);
	ASSERT_TRUE(ret);
	ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
	ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
	ret = et_doc_history_hive_redo(hist);
	ASSERT_TRUE(ret);
	ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
	ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));

	item = et_doc_history_get_from_relative(hist, 0);
	ASSERT_TRUE(NULL != item);
	ASSERT_TRUE(!pv_vg_is_diff(vg, item->vg)); // check equal

	num = et_doc_history_hive_get_num_undo(hist);
	ASSERT_EQ(0, num);
	num = et_doc_history_hive_get_num_redo(hist);
	ASSERT_EQ(0, num);
}

class MyTest : public ::testing::Test {
	protected:
		void make_hist_from_vg_src(EtDocHistoryHive **hist_ret, const PvVg *vg_src)
		{
			*hist_ret = NULL;

			PvVg *vg = pv_vg_copy_new(vg_src);
			if(NULL == vg){
				FAIL();
			}
			if(FALSE != pv_vg_is_diff(vg_src, vg)){
				FAIL();
			}
			EtDocHistoryHive *hist = NULL;
			hist = et_doc_history_hive_new(vg);
			if(NULL == hist){
				FAIL();
			}

			*hist_ret = hist;
			return;
		}

		void make_hist_with_diff_from_vg_src(
				EtDocHistoryHive **hist_ret, const PvVg *vg_src)
		{
			*hist_ret = NULL;

			EtDocHistory *item = NULL;
			EtDocHistoryHive *hist = NULL;

			make_hist_from_vg_src(&hist, vg_src);
			if(NULL == hist){
				FAIL();
			}

			item = et_doc_history_get_from_relative(hist, 0);
			if(NULL == item || pv_vg_is_diff(vg_src, item->vg)){
				FAIL();
			}
			PvElement *elem = pv_vg_get_layer_top(item->vg);
			PvElement *elem_new = pv_element_new(PvElementKind_Group);
			if(!pv_element_append_child(elem, NULL, elem_new)){
				FAIL();
			}
			item = et_doc_history_get_from_relative(hist, 0);
			ASSERT_TRUE(NULL != item);
			ASSERT_TRUE(NULL != item->vg);
			ASSERT_TRUE(pv_vg_is_diff(vg_src, item->vg));

			*hist_ret = hist;
			return;
		}

		void make_hist_with_redo(EtDocHistoryHive **hist_ret, const PvVg *vg_src)
		{
			EtDocHistory *item = NULL;

			EtDocHistoryHive *hist = NULL;
			make_hist_with_diff_from_vg_src(&hist, vg_src);
			if(NULL == hist){
				FAIL();
			}
			// ** save
			bool ret;
			ret = et_doc_history_hive_save_with_focus(hist);
			if(TRUE != ret){
				FAIL();
			}
			item = et_doc_history_get_from_relative(hist, 0);
			PvVg *vg_at_save = pv_vg_copy_new(item->vg);
			if(NULL == vg_at_save){
				FAIL();
			}
			// ** undo
			ret = et_doc_history_hive_undo(hist);
			item = et_doc_history_get_from_relative(hist, 0);
			if(NULL == item){
				FAIL();
			}

			*hist_ret = hist;
			return;
		}

		void do_hist_save_change(bool *ret, EtDocHistoryHive *hist)
		{
			*ret = false;

			EtDocHistory *item = NULL;
			item = et_doc_history_get_from_relative(hist, 0);
			if(NULL == item){
				FAIL();
			}
			PvElement *elem = pv_vg_get_layer_top(item->vg);
			PvElement *elem_new = pv_element_new(PvElementKind_Group);
			if(!pv_element_append_child(elem, NULL, elem_new)){
				FAIL();
			}
			ASSERT_TRUE(et_doc_history_hive_save_with_focus(hist));

			*ret = true;
			return;
		}

		void do_hist_save_changed_loop(bool *ret, EtDocHistoryHive *hist, int num_limit)
		{
			*ret = false;

			// ** save history full
			for(int i = 0; i < num_limit; i++){
				*ret = false;
				do_hist_save_change(ret, hist);
				if(FALSE == ret){
					FAIL();
				}
			}

			*ret = true;
			return;
		}
};

TEST_F(MyTest, hist_change){
	PvVg *vg_src = pv_vg_new();
	if(NULL == vg_src){
		FAIL();
	}

	PvVg *vg = pv_vg_copy_new(vg_src);
	if(FALSE != pv_vg_is_diff(vg_src, vg)){
		FAIL();
	}

	// ** change
	{
		EtDocHistoryHive *hist = NULL;
		make_hist_with_diff_from_vg_src(&hist, vg_src);
		if(NULL == hist){
			FAIL();
		}
	}

	// ** change (undo)
	{
		EtDocHistory *item = NULL;

		EtDocHistoryHive *hist = NULL;
		make_hist_from_vg_src(&hist, vg_src);
		if(NULL == hist){
			FAIL();
		}
		// ** undo
		// Nothing history undo.
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_TRUE(et_doc_history_hive_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_FALSE(pv_vg_is_diff(vg_src, item->vg));
		et_doc_history_hive_debug_print(hist);
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
	}

	// ** change (undo+diff)
	// ** difference stacking to history when undo.
	{
		EtDocHistory *item = NULL;

		EtDocHistoryHive *hist = NULL;
		make_hist_with_diff_from_vg_src(&hist, vg_src);
		if(NULL == hist){
			FAIL();
		}
		item = et_doc_history_get_from_relative(hist, 0);
		PvVg *vg1 = NULL;
		if(NULL == (vg1 = pv_vg_copy_new(item->vg))){
			FAIL();
		}
		// ** undo
		ASSERT_TRUE(et_doc_history_hive_undo(hist));
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(1, et_doc_history_hive_get_num_redo(hist));
		ASSERT_TRUE(pv_vg_is_diff(vg1, item->vg));
		ASSERT_FALSE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_TRUE(et_doc_history_hive_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		// ** redo
		ASSERT_TRUE(et_doc_history_hive_redo(hist));
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_EQ(1, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
		ASSERT_FALSE(pv_vg_is_diff(vg1, item->vg));
		ASSERT_TRUE(pv_vg_is_diff(vg_src, item->vg));
	}

	// ** change (save)
	{
		EtDocHistory *item = NULL;

		EtDocHistoryHive *hist = NULL;
		make_hist_with_diff_from_vg_src(&hist, vg_src);
		if(NULL == hist){
			FAIL();
		}
		// ** save
		bool ret;
		ret = et_doc_history_hive_save_with_focus(hist);
		ASSERT_TRUE(ret);
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_TRUE(pv_vg_is_diff(vg_src, item->vg));
		item = et_doc_history_get_from_relative(hist, -1);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_FALSE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_EQ(1, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
		// ** save skip because nothing change
		ret = et_doc_history_hive_save_with_focus(hist);
		ASSERT_TRUE(ret);
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_TRUE(pv_vg_is_diff(vg_src, item->vg));
		item = et_doc_history_get_from_relative(hist, -1);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_FALSE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_EQ(1, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
	}

	// ** change (save -> undo -> redo)
	{
		EtDocHistory *item = NULL;

		EtDocHistoryHive *hist = NULL;
		make_hist_with_diff_from_vg_src(&hist, vg_src);
		if(NULL == hist){
			FAIL();
		}
		// ** save
		bool ret;
		ret = et_doc_history_hive_save_with_focus(hist);
		if(TRUE != ret){
			FAIL();
		}
		item = et_doc_history_get_from_relative(hist, 0);
		PvVg *vg_at_save = pv_vg_copy_new(item->vg);
		if(NULL == vg_at_save){
			FAIL();
		}
		// ** undo
		ret = et_doc_history_hive_undo(hist);
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_FALSE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(1, et_doc_history_hive_get_num_redo(hist));
		// ** redo
		ret = et_doc_history_hive_redo(hist);
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_TRUE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_FALSE(pv_vg_is_diff(vg_at_save, item->vg));
		ASSERT_EQ(1, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
		// ** re process
		// ** undo
		ret = et_doc_history_hive_undo(hist);
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_FALSE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(1, et_doc_history_hive_get_num_redo(hist));
		// ** redo
		ret = et_doc_history_hive_redo(hist);
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(NULL != item->vg);
		ASSERT_TRUE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_FALSE(pv_vg_is_diff(vg_at_save, item->vg));
		ASSERT_EQ(1, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
	}

	// ** save skip because no change.
	{
		EtDocHistory *item = NULL;
		EtDocHistoryHive *hist = et_doc_history_hive_new(vg_src);
		if(NULL == hist){
			FAIL();
		}

		ASSERT_TRUE(et_doc_history_hive_save_with_focus(hist));
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_FALSE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_redo(hist));
	}

	// ** save when drop redo.
	{
		// ** stacked redo history.
		EtDocHistoryHive *hist = NULL;
		make_hist_with_redo(&hist, vg_src);
		if(NULL == hist){
			FAIL();
		}
		if(1 != et_doc_history_hive_get_num_redo(hist)){
			FAIL();
		}
		// ** change (drop redo)
		EtDocHistory *item = NULL;
		item = et_doc_history_get_from_relative(hist, 0);
		if(NULL == item || pv_vg_is_diff(vg_src, item->vg)){
			FAIL();
		}
		PvElement *elem = pv_vg_get_layer_top(item->vg);
		PvElement *elem_new = pv_element_new(PvElementKind_Group);
		if(!pv_element_append_child(elem, NULL, elem_new)){
			FAIL();
		}
		ASSERT_EQ(0,et_doc_history_hive_get_num_redo(hist));
	}

	// ** undo limit and limit over.
	const int num_history = 128;
	const int num_limit = num_history - 1;
	{
		EtDocHistory *item = NULL;

		EtDocHistoryHive *hist = NULL;
		make_hist_with_diff_from_vg_src(&hist, vg_src);
		if(NULL == hist){
			FAIL();
		}
		// ** limit full
		bool ret;
		ret = false;
		do_hist_save_changed_loop(&ret, hist, num_limit);
		if(FALSE == ret){
			FAIL();
		}
		// ** undo history full
		for(int i = 0; i < num_limit; i++){
			ASSERT_TRUE(et_doc_history_hive_undo(hist));
		}

		item = et_doc_history_get_from_relative(hist, 0);
		if(NULL == item || NULL == item->vg){
			FAIL();
		}
		ASSERT_FALSE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_EQ(num_limit, et_doc_history_hive_get_num_redo(hist));
	}

	// ** limit over
	{
		EtDocHistory *item = NULL;

		EtDocHistoryHive *hist = NULL;
		make_hist_with_diff_from_vg_src(&hist, vg_src);
		if(NULL == hist){
			FAIL();
		}

		bool ret;
		ret = false;
		do_hist_save_change(&ret, hist);
		if(FALSE == ret){
			FAIL();
		}
		item = et_doc_history_get_from_relative(hist, 0);
		PvVg *vg1 = NULL;
		if(NULL == (vg1 = pv_vg_copy_new(item->vg))){
			FAIL();
		}
		// ** save full + 1 = over
		do_hist_save_changed_loop(&ret, hist, num_limit);
		if(FALSE == ret){
			FAIL();
		}
		// ** undo history full
		for(int i = 0; i < num_limit; i++){
			ASSERT_TRUE(et_doc_history_hive_undo(hist));
		}
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_FALSE(pv_vg_is_diff(vg1, item->vg));
		printf("**\n");
		// ** over undo
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		ASSERT_TRUE(et_doc_history_hive_undo(hist));
		ASSERT_EQ(0, et_doc_history_hive_get_num_undo(hist));
		item = et_doc_history_get_from_relative(hist, 0);
		ASSERT_TRUE(NULL != item);
		ASSERT_TRUE(pv_vg_is_diff(vg_src, item->vg));
		ASSERT_FALSE(pv_vg_is_diff(vg1, item->vg));
	}
}

