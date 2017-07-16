#include <gtest/gtest.h>

extern "C"
{
#include "et_tool_info.h"
#include "et_tool_info_util.h"
#include "et_mouse_cursor_info.h"

#include "pv_vg.h"
#include "pv_element.h"
#include "pv_element_info.h"
#include "pv_focus.h"
#include "et_pointing_util.h"
#include "pv_document_preference.h"
}

#define NUM_CURVE		(7)
#define PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH		(16)

class Environment_EtToolInfo : public ::testing::Environment{
public:
	virtual ~Environment_EtToolInfo(){}

	virtual void SetUp()
	{
		bool res;
		res = et_tool_info_init_for_unittest(".");
		assert(res);

		res = et_mouse_cursor_info_init_for_unittest(".");
		assert(res);
	}
};

::testing::Environment* const foo_env = ::testing::AddGlobalTestEnvironment(new Environment_EtToolInfo);

#define SNAP_CONTEXT_POINTER (&(pv_document_preference.snap_context))

class TestEtToolInfo_Base_Base : public ::testing::Test{
protected:
	PvDocumentPreference pv_document_preference;

	PvPoint pointing_context_previous_mouse_point;
	PvPoint pointing_context_down_mouse_point;

	TestEtToolInfo_Base_Base()
		:
		pv_document_preference(PvDocumentPreference_Default)
	{}

	EtMouseAction mouse_action_(PvPoint event_point, EtMouseActionType mouse_action_kind)
	{
		int margin = 0;
		double scale = 1.0;
		EtMouseAction mouse_action = et_pointing_util_get_mouse_action(
				&pointing_context_previous_mouse_point,
				&pointing_context_down_mouse_point,
				event_point,
				(MOUSE_BUTTON_LEFT_MASK),
				margin,
				scale,
				EtMouseButton_Right,
				mouse_action_kind);

		fprintf(stdout,
				"%.1f, %.1f, "
				"%.1f, %.1f, "
				"%.1f, %.1f, \n",
				mouse_action.diff_down.x,
				mouse_action.diff_down.y,
				pointing_context_previous_mouse_point.x,
				pointing_context_previous_mouse_point.y,
				pointing_context_down_mouse_point.x,
				pointing_context_down_mouse_point.y
				);
		return mouse_action;
	}
};

class TestEtToolInfo_Base : public TestEtToolInfo_Base_Base{
protected:
	PvVg *vg;
	PvVg *vg_back;
	PvFocus *focus;
	PvElement *element_layer_top;

	virtual ~TestEtToolInfo_Base()
	{}

	virtual void internalSetUp() = 0;

	virtual void SetUp()
	{
		vg = pv_vg_new();
		assert(NULL != vg);

		element_layer_top = pv_vg_get_layer_top(vg);
		assert(NULL != element_layer_top);

		focus = pv_focus_new(vg);
		assert(NULL != focus);

		internalSetUp();

		vg_back = pv_vg_copy_new(vg);
		assert(vg);

		assert(NUM_CURVE == pv_general_get_parray_num((void **)element_layer_top->childs));
		assert(element_layer_top == focus->elements[0]);
	}

	virtual void TearDown()
	{
		if(NULL != focus){
			pv_focus_free(focus);
		}
		if(NULL != vg){
			pv_vg_free(vg);
		}
		if(NULL != vg_back){
			pv_vg_free(vg_back);
		}
	}

};
/*
class TestEtToolInfo_Element : public TestEtToolInfo_Base{
protected:
	PvElement *element_curves[NUM_CURVE];

	void internalSetUp()
	{
		for(int i = 0; i < NUM_CURVE; i++){
			element_curves[i] = pv_element_new(PvElementKind_Curve);
			assert(NULL != element_curves[i]);
			for(int t = 0; t < 2; t++){
				PvPoint point = {
					(double) i * 100,
					(double) t * 100
				};
				PvAnchorPoint ap = pv_anchor_point_from_point(point);
				assert(pv_element_curve_add_anchor_point(element_curves[i], ap));
			}
			assert(pv_element_append_child(element_layer_top, NULL, element_curves[i]));
		}
	}
};
*/

#define NUM_ANCHOR_POINT_PAR_CURVE (6)
class TestEtToolInfo_Element : public TestEtToolInfo_Base{
protected:
	PvElement *element_curves[NUM_CURVE];
	PvAnchorPoint *anchor_points[NUM_CURVE][NUM_ANCHOR_POINT_PAR_CURVE];

	void internalSetUp()
	{
		for(int i = 0; i < NUM_CURVE; i++){
			element_curves[i] = pv_element_new(PvElementKind_Curve);
			assert(NULL != element_curves[i]);
			for(int t = 0; t < NUM_ANCHOR_POINT_PAR_CURVE; t++){
				PvPoint point = {
					(double) i * 100,
					(double) t * 20
				};
				PvAnchorPoint *ap = pv_anchor_point_new_from_point(point);
				assert(ap);
				pv_element_curve_append_anchor_point(element_curves[i], ap, -1);
				anchor_points[i][t] = ap;
			}
			assert(pv_element_append_child(element_layer_top, NULL, element_curves[i]));
		}
	}
};


TEST_F(TestEtToolInfo_Element, Test){
	EXPECT_EQ(1,1);
}

TEST_F(TestEtToolInfo_Element, EtToolInfo_FocusingTouchedSingleElement){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	// # down
	event_point = (PvPoint){0, 0};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(element_curves[0], focus->elements[0]);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));

	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(0, position_rect.x);
	EXPECT_EQ(0, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);


	// # move (1)
	event_point = (PvPoint){550, 360};
	mouse_action = mouse_action_(event_point, EtMouseAction_Move);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(false == pv_focus_is_exist_element(focus, element_curves[1]));
	EXPECT_TRUE(false == pv_focus_is_exist_element(focus, element_curves[2]));
	EXPECT_TRUE(false == pv_focus_is_exist_element(focus, element_curves[3]));

//! @todo enable test.
/*
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(550, position_rect.x);
	EXPECT_EQ(360, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
*/

	// # move (2)
	event_point = (PvPoint){350, 120};
	mouse_action = mouse_action_(event_point, EtMouseAction_Move);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(false == pv_focus_is_exist_element(focus, element_curves[1]));
	EXPECT_TRUE(false == pv_focus_is_exist_element(focus, element_curves[2]));
	EXPECT_TRUE(false == pv_focus_is_exist_element(focus, element_curves[3]));

//! @todo enable test.
/*
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(350, position_rect.x);
	EXPECT_EQ(120, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
*/

	// # up
	event_point = (PvPoint){250, 180};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));

	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(250, position_rect.x);
	EXPECT_EQ(180, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);

}

TEST_F(TestEtToolInfo_Element, EtToolInfo_FocusingByArea){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;


	// # down
	event_point = (PvPoint){-1, -1};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(1, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(false == pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(element_layer_top, focus->elements[0]);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_layer_top));

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # move (1)
	event_point = (PvPoint){350, 120};
	mouse_action = mouse_action_(event_point, EtMouseAction_Move);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(5, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(4, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[2]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[3]));

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # move (2)
	event_point = (PvPoint){450, 120};
	mouse_action = mouse_action_(event_point, EtMouseAction_Move);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(5, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(5, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[2]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[3]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[4]));

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # move (3)
	event_point = (PvPoint){250, 120};
	mouse_action = mouse_action_(event_point, EtMouseAction_Move);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(5, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(3, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[2]));

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = (PvPoint){150, 220};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));

}

TEST_F(TestEtToolInfo_Element, EtToolInfo_Translate){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	// # setup focusing
	assert(pv_focus_add_element(focus, element_curves[0]));
	assert(pv_focus_add_element(focus, element_curves[1]));

	// # down
	event_point = (PvPoint){0, 50};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = (PvPoint){
		0 + 40,
		50 + 60
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(0 + 40, position_rect.x);
	EXPECT_EQ(0 + 60, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[1]);
	EXPECT_EQ(100 + 40, position_rect.x);
	EXPECT_EQ(0 + 60, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);

}


TEST_F(TestEtToolInfo_Element, EtToolInfo_Translate_minus){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;


	// # setup focusing
	assert(pv_focus_add_element(focus, element_curves[0]));
	assert(pv_focus_add_element(focus, element_curves[1]));

	// # down
	event_point = (PvPoint){0, 50};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = (PvPoint){
		0 - 200,
		50 - 500,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(0 - 200, position_rect.x);
	EXPECT_EQ(0 - 500, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[1]);
	EXPECT_EQ(100 - 200, position_rect.x);
	EXPECT_EQ(0 - 500, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);

}


TEST_F(TestEtToolInfo_Element, EtToolInfo_Resize){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;


	// # setup focusing
	assert(pv_focus_add_element(focus, element_curves[0]));
	assert(pv_focus_add_element(focus, element_curves[1]));

	// # down
	event_point = (PvPoint){
		100 + (PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH / 2) - 1,
		100 + (PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH / 2) - 1
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL != cursor);

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){50, 200});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(0 * 1.5, position_rect.x);
	EXPECT_EQ(0 * 3.0, position_rect.y);
	EXPECT_EQ(0 * 1.5, position_rect.w);
	EXPECT_EQ(100 * 3.0, position_rect.h);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[1]);
	EXPECT_EQ(100 * 1.5, position_rect.x);
	EXPECT_EQ(0 * 3.0, position_rect.y);
	EXPECT_EQ(0 * 1.5, position_rect.w);
	EXPECT_EQ(100 * 3.0, position_rect.h);

}


TEST_F(TestEtToolInfo_Element, EtToolInfo_Rotate){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;


	// # setup focusing
	assert(pv_focus_add_element(focus, element_curves[0]));
	assert(pv_focus_add_element(focus, element_curves[1]));

	// # down
	event_point = (PvPoint){
		100 + (PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH / 2) + 1,
		100 + (PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH / 2) + 1
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(5, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL != cursor);

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = (PvPoint){
		0 - ((PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH / 2) + 1),
		100 + (PX_SENSITIVE_RESIZE_EDGE_OF_TOUCH / 2) + 1,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL != cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(0, position_rect.x);
	EXPECT_EQ(0, position_rect.y);
	EXPECT_EQ(100, position_rect.w);
	EXPECT_EQ(0, position_rect.h);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[1]);
	EXPECT_EQ(0, position_rect.x);
	EXPECT_EQ(100, position_rect.y);
	EXPECT_EQ(100, position_rect.w);
	EXPECT_EQ(0, position_rect.h);

}

TEST_F(TestEtToolInfo_Element, EtToolInfo_Translate_with_SnapForGrid_plus){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvSnapContext snap_context = {
		.is_snap_for_grid = true,
		.grid = {50, 50,},
	};


	// # setup focusing
	assert(pv_focus_add_element(focus, element_curves[0]));
	assert(pv_focus_add_element(focus, element_curves[1]));

	// # down
	event_point = (PvPoint){0, 50};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = (PvPoint){
		0 + 140,
		50 + 360
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(150, position_rect.x);
	EXPECT_EQ(350, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[1]);
	EXPECT_EQ(250, position_rect.x);
	EXPECT_EQ(350, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);

}

TEST_F(TestEtToolInfo_Element, EtToolInfo_Translate_with_SnapForGrid_minus){
	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvSnapContext snap_context = {
		.is_snap_for_grid = true,
		.grid = {50, 50,},
	};


	// # setup focusing
	assert(pv_focus_add_element(focus, element_curves[0]));
	assert(pv_focus_add_element(focus, element_curves[1]));

	// # down
	event_point = (PvPoint){0, 50};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = (PvPoint){
		0 - 140,
		50 - 360,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(-150, position_rect.x);
	EXPECT_EQ(-350, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[1]);
	EXPECT_EQ(-50, position_rect.x);
	EXPECT_EQ(-350, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
}


TEST_F(TestEtToolInfo_Element, EtToolInfo_Resize_with_SnapForGrid_minus){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvSnapContext snap_context = {
		.is_snap_for_grid = true,
		.grid = {50, 50,},
	};


	// # setup focusing
	assert(pv_focus_add_element(focus, element_curves[0]));
	assert(pv_focus_add_element(focus, element_curves[1]));

	// # down
	event_point = (PvPoint){
		100,
		100,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL != cursor);

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){-260, -340});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_element_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(0 * -1.5, position_rect.x);
	EXPECT_EQ(-250, position_rect.y);
	EXPECT_EQ(0 * 1.5, position_rect.w);
	EXPECT_EQ(100 * 2.5, position_rect.h);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[1]);
	EXPECT_EQ(100 * -1.5, position_rect.x);
	EXPECT_EQ(-250, position_rect.y);
	EXPECT_EQ(0 * 1.5, position_rect.w);
	EXPECT_EQ(100 * 2.5, position_rect.h);

}


TEST_F(TestEtToolInfo_Element, EtToolInfo_TranslateAnchorPoint_with_SnapForGrid){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvSnapContext snap_context = {
		.is_snap_for_grid = true,
		.grid = {50, 50,},
	};


	// # setup focusing
	assert(pv_focus_add_anchor_point(focus, element_curves[0], anchor_points[0][0]));
	for(int t = 0; t < NUM_ANCHOR_POINT_PAR_CURVE; t++){
		assert(pv_focus_add_anchor_point(focus, element_curves[1], anchor_points[1][t]));
	}
	assert(pv_focus_add_anchor_point(focus, element_curves[2], anchor_points[2][0]));

	// # down
	event_point = (PvPoint){
		100,
		20,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_edit_anchor_point_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// document not change.
	EXPECT_TRUE(false == pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){-260, -350});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_edit_anchor_point_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL != edit_draw_element);
	if(NULL != edit_draw_element){
		EXPECT_EQ(4, pv_general_get_parray_num((void **)edit_draw_element->childs));
	}
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(3, pv_general_get_parray_num((void **)focus->elements))
		<< pv_general_get_parray_num((void **)focus->anchor_points);
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[0]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[1]));
	EXPECT_TRUE(pv_focus_is_exist_element(focus, element_curves[2]));

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[0]);
	EXPECT_EQ(0 -250, position_rect.x);
	EXPECT_EQ(0 + (-350 -20), position_rect.y);
	EXPECT_EQ(0 + 250, position_rect.w);
	EXPECT_EQ(100 - (-350 -20), position_rect.h);
	position_rect = element_info->func_get_rect_by_anchor_points(element_curves[1]);
	EXPECT_EQ(100 -250, position_rect.x);
	EXPECT_EQ(0 + (-350 - 20), position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(100, position_rect.h);

}

TEST_F(TestEtToolInfo_Element, EtToolInfo_AddAnchorPoint){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvColorPair color_pair = PvColorPair_None;
	PvStroke stroke = PvStroke_Default;

	// # setup focusing
	pv_focus_clear_to_first_layer(focus);



	// ** new AnchorPath and AnchorPoint
	// # down
	event_point = (PvPoint){
		-100,
		-20,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// document not change.
	EXPECT_TRUE(pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){-90, 90});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->anchor_points));

	for(int i = 0; i < NUM_CURVE; i++){
		EXPECT_NE(focus->elements[0], element_curves[i]);

		for(int t = 0; t < NUM_ANCHOR_POINT_PAR_CURVE; t++){
			EXPECT_NE(focus->anchor_points[0], anchor_points[i][t]);
		}
	}

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(focus->elements[0]);
	EXPECT_EQ(-100, position_rect.x);
	EXPECT_EQ(-20, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(0, position_rect.h);
	EXPECT_FALSE(pv_element_curve_get_close_anchor_point(focus->elements[0]));



	// ** AnchorPath add AnchorPoint
	is_save = false;

	// # down
	event_point = (PvPoint){
		-250,
		-120,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// document not change.
	EXPECT_TRUE(pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){-90, 90});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->anchor_points));

	for(int i = 0; i < NUM_CURVE; i++){
		EXPECT_NE(focus->elements[0], element_curves[i]);

		for(int t = 0; t < NUM_ANCHOR_POINT_PAR_CURVE; t++){
			EXPECT_NE(focus->anchor_points[0], anchor_points[i][t]);
		}
	}

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(focus->elements[0]);
	EXPECT_EQ(-250, position_rect.x);
	EXPECT_EQ(-120, position_rect.y);
	EXPECT_EQ(150, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
	EXPECT_FALSE(pv_element_curve_get_close_anchor_point(focus->elements[0]));



	// ** AnchorPath closed
	is_save = false;

	// # down
	event_point = (PvPoint){
		-100,
		-20,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// document not change.
	EXPECT_TRUE(pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){-90, 90});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->anchor_points));

	for(int i = 0; i < NUM_CURVE; i++){
		EXPECT_NE(focus->elements[0], element_curves[i]);

		for(int t = 0; t < NUM_ANCHOR_POINT_PAR_CURVE; t++){
			EXPECT_NE(focus->anchor_points[0], anchor_points[i][t]);
		}
	}

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(focus->elements[0]);
	EXPECT_EQ(-250, position_rect.x);
	EXPECT_EQ(-120, position_rect.y);
	EXPECT_EQ(150, position_rect.w);
	EXPECT_EQ(100, position_rect.h);
	EXPECT_TRUE(pv_element_curve_get_close_anchor_point(focus->elements[0]));



	// ** 2nd new AnchorPath and AnchorPoint
	is_save = false;
	PvElement *prev_element = focus->elements[0];

	// # down
	event_point = (PvPoint){
		-150,
		-100,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// document not change.
	EXPECT_TRUE(pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){-90, 90});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->anchor_points));

	EXPECT_NE(focus->elements[0], prev_element);
	for(int i = 0; i < NUM_CURVE; i++){
		EXPECT_NE(focus->elements[0], element_curves[i]);

		for(int t = 0; t < NUM_ANCHOR_POINT_PAR_CURVE; t++){
			EXPECT_NE(focus->anchor_points[0], anchor_points[i][t]);
		}
	}

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(focus->elements[0]);
	EXPECT_EQ(-150, position_rect.x);
	EXPECT_EQ(-100, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(0, position_rect.h);
	EXPECT_FALSE(pv_element_curve_get_close_anchor_point(focus->elements[0]));

}

TEST_F(TestEtToolInfo_Element, EtToolInfo_AddAnchorPoint_SnapForGrid){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvSnapContext snap_context = {
		.is_snap_for_grid = true,
		.grid = {50, 50,},
	};

	PvColorPair color_pair = PvColorPair_None;
	PvStroke stroke = PvStroke_Default;

	// # setup focusing
	pv_focus_clear_to_first_layer(focus);



	// ** new AnchorPath and AnchorPoint
	// # down
	event_point = (PvPoint){
		-240,
		-90,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// document not change.
	EXPECT_TRUE(pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){-90, 90});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_add_anchor_point_handle_mouse_action(
			vg,
			focus,
			&snap_context,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->anchor_points));

	for(int i = 0; i < NUM_CURVE; i++){
		EXPECT_NE(focus->elements[0], element_curves[i]);

		for(int t = 0; t < NUM_ANCHOR_POINT_PAR_CURVE; t++){
			EXPECT_NE(focus->anchor_points[0], anchor_points[i][t]);
		}
	}

	// document
	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(focus->elements[0]);
	EXPECT_EQ(-250, position_rect.x);
	EXPECT_EQ(-100, position_rect.y);
	EXPECT_EQ(0, position_rect.w);
	EXPECT_EQ(0, position_rect.h);
	EXPECT_FALSE(pv_element_curve_get_close_anchor_point(focus->elements[0]));

}

TEST_F(TestEtToolInfo_Element, EtToolInfo_AddBasicShape){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvColorPair color_pair = PvColorPair_None;
	PvStroke stroke = PvStroke_Default;

	// # setup focusing
	pv_focus_clear_to_first_layer(focus);



	// # down
	event_point = (PvPoint){
		100,
		50,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_add_basic_shape_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// document not change.
	EXPECT_TRUE(pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){100, 200});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_add_basic_shape_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(0, pv_general_get_parray_num((void **)focus->anchor_points));

	for(int i = 0; i < NUM_CURVE; i++){
		EXPECT_NE(focus->elements[0], element_curves[i]);
	}

	// document
	EXPECT_EQ(focus->elements[0]->kind, PvElementKind_BasicShape);
	element_info = pv_element_get_info_from_kind(PvElementKind_BasicShape);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(focus->elements[0]);
	/*
	EXPECT_EQ(100, position_rect.x);
	EXPECT_EQ(50, position_rect.y);
	EXPECT_EQ(100, position_rect.w);
	EXPECT_EQ(200, position_rect.h);
	*/

}

TEST_F(TestEtToolInfo_Element, EtToolInfo_AddBasicShape_minus){

	bool res;
	bool is_save = false;
	PvPoint event_point;
	EtMouseAction mouse_action;
	PvElement *edit_draw_element = NULL;
	GdkCursor *cursor = NULL;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvColorPair color_pair = PvColorPair_None;
	PvStroke stroke = PvStroke_Default;

	// # setup focusing
	pv_focus_clear_to_first_layer(focus);



	// # down
	event_point = (PvPoint){
		-100,
		-50,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	res = et_tool_info_util_func_add_basic_shape_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(false == is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// document not change.
	EXPECT_TRUE(pv_vg_is_diff(vg, vg_back));


	// # up
	event_point = pv_point_add(event_point, (PvPoint){-100, -200});
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	res = et_tool_info_util_func_add_basic_shape_element_mouse_action(
			vg,
			focus,
			SNAP_CONTEXT_POINTER,
			&is_save,
			mouse_action,
			&edit_draw_element,
			&cursor,
			color_pair,
			stroke);

	EXPECT_TRUE(res);
	EXPECT_TRUE(is_save);
	EXPECT_TRUE(NULL == edit_draw_element);
	EXPECT_TRUE(NULL == cursor);

	// focus
	EXPECT_TRUE(pv_focus_is_focused(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_EQ(0, pv_general_get_parray_num((void **)focus->anchor_points));

	for(int i = 0; i < NUM_CURVE; i++){
		EXPECT_NE(focus->elements[0], element_curves[i]);
	}

	// document
	EXPECT_EQ(focus->elements[0]->kind, PvElementKind_BasicShape);
	element_info = pv_element_get_info_from_kind(PvElementKind_BasicShape);
	assert(element_info);
	position_rect = element_info->func_get_rect_by_anchor_points(focus->elements[0]);
	/*
	EXPECT_EQ(-100 -100, position_rect.x);
	EXPECT_EQ(-50 -200, position_rect.y);
	EXPECT_EQ(100, position_rect.w);
	EXPECT_EQ(200, position_rect.h);
	*/

}

TEST_F(TestEtToolInfo_Base_Base, ResizeElements_00){

	PvPoint event_point;
	EtMouseAction mouse_action;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvElement **elements = (PvElement **)malloc(sizeof(PvElement *) * 2);
	elements[0] = pv_element_new(PvElementKind_Curve);
	elements[1] = NULL;
	pv_element_curve_add_anchor_point(elements[0], pv_anchor_point_from_point((PvPoint){100, 100}));
	pv_element_curve_add_anchor_point(elements[0], pv_anchor_point_from_point((PvPoint){200, 200}));

	// # down
	event_point = (PvPoint){
		200,
		200,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	// up
	event_point = (PvPoint){
		250,
		250,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	EdgeKind edge_kind = resize_elements_(
			elements,
			SNAP_CONTEXT_POINTER,
			mouse_action,
			EdgeKind_Resize_DownRight,
			(PvRect){100,100,100,100});


	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);

	element_info->func_apply_appearances(elements[0], elements[0]->etaion_work_appearances);
	elements[0]->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

	position_rect = element_info->func_get_rect_by_anchor_points(elements[0]);
	EXPECT_EQ(100, position_rect.x);
	EXPECT_EQ(100, position_rect.y);
	EXPECT_EQ(150, position_rect.w);
	EXPECT_EQ(150, position_rect.h);
}

TEST_F(TestEtToolInfo_Base_Base, ResizeElements_01){

	PvPoint event_point;
	EtMouseAction mouse_action;
	const PvElementInfo *element_info = NULL;
	PvRect position_rect;

	PvElement **elements = (PvElement **)malloc(sizeof(PvElement *) * 2);
	elements[0] = pv_element_new(PvElementKind_Curve);
	elements[1] = NULL;
	pv_element_curve_add_anchor_point(elements[0], pv_anchor_point_from_point((PvPoint){100, 100}));
	pv_element_curve_add_anchor_point(elements[0], pv_anchor_point_from_point((PvPoint){101, 101}));

	// # down
	event_point = (PvPoint){
		101,
		101,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Down);

	// up
	event_point = (PvPoint){
		250,
		250,
	};
	mouse_action = mouse_action_(event_point, EtMouseAction_Up);

	EdgeKind edge_kind = resize_elements_(
			elements,
			SNAP_CONTEXT_POINTER,
			mouse_action,
			EdgeKind_Resize_DownRight,
			(PvRect){100,100,1,1});


	element_info = pv_element_get_info_from_kind(PvElementKind_Curve);
	assert(element_info);

	element_info->func_apply_appearances(elements[0], elements[0]->etaion_work_appearances);
	elements[0]->etaion_work_appearances[0]->kind = PvAppearanceKind_None;

	position_rect = element_info->func_get_rect_by_anchor_points(elements[0]);
	EXPECT_EQ(100, position_rect.x);
	EXPECT_EQ(100, position_rect.y);
	EXPECT_EQ(150, position_rect.w);
	EXPECT_EQ(150, position_rect.h);
}

