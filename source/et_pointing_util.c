#include "et_pointing_util.h"

#include "et_error.h"

static PvPoint _et_canvas_dp_from_cwp(PvPoint cwp, int margin, double scale)
{
	PvPoint cp = pv_point_add_value(cwp, -1 * margin);
	PvPoint dp = pv_point_div_value(cp, scale);

	return dp;
}

EtMouseAction et_pointing_util_get_mouse_action(
		PvPoint *pointing_context_previous_mouse_point,
		PvPoint *pointing_context_down_mouse_point,
		PvPoint event_point,
		GdkModifierType event_state,
		int margin,
		double scale,
		EtMouseButtonType mouse_button,
		EtMouseActionType mouse_action)
{
	PvPoint dp = {0, 0};
	PvPoint dp_move = {0, 0};
	PvPoint cwp_diff_down = {0, 0};

	switch(mouse_action){
		case EtMouseAction_Down:
			{
				*pointing_context_previous_mouse_point = event_point;
				*pointing_context_down_mouse_point = event_point;
				PvPoint cwp = event_point;
				dp = _et_canvas_dp_from_cwp(cwp, margin, scale);

				dp_move = (PvPoint){.x = 0, .y = 0};
				cwp_diff_down = (PvPoint){.x = 0, .y = 0};
				/*

				   dp,
				   dp_move,
				   cwp_diff_down,
				   self->render_context.scale,
				   event->state,
				   EtMouseButton_Right, EtMouseAction_Down))
				 */
			}
			break;
		case EtMouseAction_Move:
			{
				PvPoint cwp = event_point;
				dp = _et_canvas_dp_from_cwp(cwp, margin, scale);

				PvPoint cwp_prev = *pointing_context_previous_mouse_point;
				PvPoint dp_prev = _et_canvas_dp_from_cwp(cwp_prev, margin, scale);
				dp_move = pv_point_sub(dp, dp_prev);

				*pointing_context_previous_mouse_point = event_point;

				PvPoint ep_diff_down = pv_point_sub(event_point, *pointing_context_down_mouse_point);
				cwp_diff_down = ep_diff_down;
				/*
				   dp,
				   dp_move,
				   cwp_diff_down,
				   self->render_context.scale,
				   event->state,
				   EtMouseButton_Right, EtMouseAction_Move))

				 */
			}
			break;
		case EtMouseAction_Up:
			{
				PvPoint cwp = event_point;
				dp = _et_canvas_dp_from_cwp(cwp, margin, scale);

				PvPoint cwp_prev = *pointing_context_previous_mouse_point;
				PvPoint dp_prev = _et_canvas_dp_from_cwp(cwp_prev, margin, scale);
				dp_move = pv_point_sub(dp, dp_prev);

				*pointing_context_previous_mouse_point = event_point;

				PvPoint ep_diff_down = pv_point_sub(event_point, *pointing_context_down_mouse_point);
				cwp_diff_down = ep_diff_down;
				/*

				   dp,
				   dp_move,
				   cwp_diff_down,
				   self->render_context.scale,
				   event->state,
				   EtMouseButton_Right, EtMouseAction_Up))
				 */
			}
			break;
		default:
			et_abortf("%d", mouse_action);
	}

	/*

	   PvPoint dp,
	   PvPoint dp_move,
	   PvPoint cwp_diff_down,
	   double scale,
	   GdkModifierType state,
	   EtMouseButtonType mouse_button, EtMouseActionType mouse_action)

	   EtMouseAction _mouse_action = {
	   .button = mouse_button,
	   .action = mouse_action,
	   .point = dp,
	   .move = dp_move,
	   .diff_down = cwp_diff_down,
	   .state = state,
	   .scale = scale,
	   };

	 */

	EtMouseAction mouse_action_ = {
		.button = mouse_button,
		.action = mouse_action,
		.point = dp,
		.move = dp_move,
		.diff_down = cwp_diff_down,
		.state = event_state,
		.scale = scale,
	};

	return mouse_action_;
}

