#include "includes.h"
#include "pred.h"

InputPrediction g_inputpred{ };

void InputPrediction::Initialize() {

	// unlock the engines choked command clamp so we can fake lag past its limit.
	// note; this used to live inside UpdateGamePrediction, meaning it ran its checks
	//       on every single command, and it wrote to the patch address without ever
	//       verifying the pattern actually resolved. a failed scan gave us address 1.
	// note; only ever attempt this once. a pattern scan over the whole engine module
	//       is not something we want to run per command, and if it fails now it will
	//       keep failing.
	if (m_init_attempted)
		return;

	m_init_attempted = true;

	Address cl_move_clamp = pattern::find(g_csgo.m_engine_dll, XOR("B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC"));
	if (!cl_move_clamp)
		return;

	const auto patch_at = cl_move_clamp.add(1).as< std::uint32_t* >();
	unsigned long protect = 0;

	if (!VirtualProtect((void*)patch_at, sizeof(std::uint32_t), PAGE_EXECUTE_READWRITE, &protect))
		return;

	// keep the original so we can put it back when we unload.
	m_move_clamp = patch_at;
	m_move_clamp_original = *patch_at;

	*patch_at = 62;

	VirtualProtect((void*)patch_at, sizeof(std::uint32_t), protect, &protect);

	m_unlocked_fakelag = true;
}

void InputPrediction::Restore() {

	if (!m_unlocked_fakelag || !m_move_clamp)
		return;

	unsigned long protect = 0;

	if (!VirtualProtect((void*)m_move_clamp, sizeof(std::uint32_t), PAGE_EXECUTE_READWRITE, &protect))
		return;

	*m_move_clamp = m_move_clamp_original;

	VirtualProtect((void*)m_move_clamp, sizeof(std::uint32_t), protect, &protect);

	m_move_clamp = nullptr;
	m_unlocked_fakelag = false;
}

void InputPrediction::ApplyPredictedNetvars(CMoveData& data) {
	// restore move data.
	std::memcpy(&data, &m_data.data, sizeof(CMoveData));

	g_cl.m_local->m_aimPunchAngleVel() = m_data.m_aim_punch_vel;
	g_cl.m_local->m_aimPunchAngle() = m_data.m_aim_punch;
	g_cl.m_local->m_viewPunchAngle() = m_data.m_view_punch;
	g_cl.m_local->m_nTickBase() = m_data.m_tickbase;
	g_cl.m_local->m_fFlags() = m_data.m_flags;
	g_cl.m_local->m_MoveType() = m_data.m_move_type;
	g_cl.m_local->m_vecOrigin() = m_data.m_origin;
	g_cl.m_local->m_vecVelocity() = m_data.m_velocity;
	g_cl.m_local->m_vecViewOffset() = m_data.m_view_offset;
	g_cl.m_local->m_flStamina() = m_data.m_stamina;
	g_cl.m_local->m_flVelocityModifier() = m_data.m_velocity_modifier;
	g_cl.m_local->m_flDuckAmount() = m_data.m_duck_amount;
	g_cl.m_local->m_flFallVelocity() = m_data.m_fall_velocity;
	g_cl.m_local->m_surfaceFriction() = m_data.m_surface_friction;
}

void InputPrediction::StorePredictedNetvars(const CMoveData& data) {
	// store move data.
	std::memcpy(&m_data.data, &data, sizeof(CMoveData));

	m_data.m_aim_punch_vel = g_cl.m_local->m_aimPunchAngleVel();
	m_data.m_aim_punch = g_cl.m_local->m_aimPunchAngle();
	m_data.m_view_punch = g_cl.m_local->m_viewPunchAngle();
	m_data.m_tickbase = g_cl.m_local->m_nTickBase();
	m_data.m_flags = g_cl.m_local->m_fFlags();
	m_data.m_move_type = g_cl.m_local->m_MoveType();
	m_data.m_origin = g_cl.m_local->m_vecOrigin();
	m_data.m_velocity = g_cl.m_local->m_vecVelocity();
	m_data.m_view_offset = g_cl.m_local->m_vecViewOffset();
	m_data.m_stamina = g_cl.m_local->m_flStamina();
	m_data.m_velocity_modifier = g_cl.m_local->m_flVelocityModifier();
	m_data.m_duck_amount = g_cl.m_local->m_flDuckAmount();
	m_data.m_fall_velocity = g_cl.m_local->m_flFallVelocity();
	m_data.m_surface_friction = g_cl.m_local->m_surfaceFriction();
}

void InputPrediction::UpdateGamePrediction(CUserCmd* cmd) {
	// make sure the game predicted every command we sent.
	// prediction runs on frames, so on low fps it can fall behind.
	const int m_tick = g_csgo.m_cl->m_delta_tick;
	if (m_tick > 0) {
		g_csgo.m_prediction->Update(m_tick, true, g_csgo.m_cl->m_last_command_ack,
			g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands);
	}

	// one time engine patch, cheap no-op once it succeeded.
	Initialize();
}

void InputPrediction::RunGamePrediction(CUserCmd* cmd) {
	// backup data.
	m_curtime = g_csgo.m_globals->m_curtime;
	m_frametime = g_csgo.m_globals->m_frametime;
	m_first_time_predicted = g_csgo.m_prediction->m_first_time_predicted;
	m_in_prediction = g_csgo.m_prediction->m_in_prediction;

	*g_csgo.m_nPredictionRandomSeed = cmd->m_random_seed;
	g_csgo.m_pPredictionPlayer = g_cl.m_local;

	g_csgo.m_globals->m_curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	CMoveData move_data{ };
	std::memset(&move_data, 0, sizeof(CMoveData));

	// note; this pass only exists to capture the state we are starting from.
	//       it has to be wrapped in its own Start/Finish pair, the old code opened
	//       a StartTrackPredictionErrors here that nothing ever closed, so the
	//       engines prediction error accumulated across commands.
	g_csgo.m_game_movement->StartTrackPredictionErrors(g_cl.m_local);
	{
		g_csgo.m_prediction->SetupMove(g_cl.m_local, cmd, g_csgo.m_move_helper, &move_data);

		// store predicted netvars.
		StorePredictedNetvars(move_data);
	}
	g_csgo.m_game_movement->FinishTrackPredictionErrors(g_cl.m_local);

	// lets re-predict them.
	PredictGamePrediction(move_data, cmd);
}

void InputPrediction::PredictGamePrediction(CMoveData& data, CUserCmd* cmd) {
	// apply predicted netvars.
	ApplyPredictedNetvars(data);

	// feed the command we are about to send into the move data.
	data.m_flForwardMove = cmd->m_forward_move;
	data.m_flSideMove = cmd->m_side_move;
	data.m_flUpMove = cmd->m_up_move;
	data.m_nButtons = cmd->m_buttons;
	data.m_vecViewAngles = cmd->m_view_angles;
	data.m_vecAbsViewAngles = cmd->m_view_angles;
	data.m_nImpulseCommand = cmd->m_impulse;

	g_csgo.m_prediction->m_first_time_predicted = false;
	g_csgo.m_prediction->m_in_prediction = true;

	*g_csgo.m_nPredictionRandomSeed = cmd->m_random_seed;
	g_csgo.m_pPredictionPlayer = g_cl.m_local;

	g_csgo.m_globals->m_curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
	g_csgo.m_globals->m_frametime = g_cl.m_local->m_fFlags() & FL_ATCONTROLS ? 0.f : g_csgo.m_globals->m_interval;

	g_csgo.m_move_helper->SetHost(g_cl.m_local);
	{
		g_csgo.m_game_movement->StartTrackPredictionErrors(g_cl.m_local);
		{
			// note; a single SetupMove for the command we actually run.
			//       there used to be a second one here on a different command than
			//       the one the caller built, which made the copy pointless.
			g_csgo.m_prediction->SetupMove(g_cl.m_local, cmd, g_csgo.m_move_helper, &data);

			g_csgo.m_game_movement->ProcessMovement(g_cl.m_local, &data);

			g_cl.m_local->SetAbsOrigin(g_cl.m_local->m_vecOrigin());

			g_csgo.m_prediction->FinishMove(g_cl.m_local, cmd, &data);
		}
		g_csgo.m_game_movement->FinishTrackPredictionErrors(g_cl.m_local);
	}
	g_csgo.m_move_helper->SetHost(nullptr);

	auto weapon = g_cl.m_local->GetActiveWeapon();
	if (weapon) {
		weapon->GetSpread();
		weapon->UpdateAccuracyPenalty();
	}
}

void InputPrediction::RestoreGamePrediction(CUserCmd* cmd) {
	g_csgo.m_prediction->m_in_prediction = false;

	*g_csgo.m_nPredictionRandomSeed = -1;
	g_csgo.m_pPredictionPlayer = nullptr;

	// restore curtime/frametime
	// and prediction seed/player.
	g_csgo.m_globals->m_curtime = g_inputpred.m_curtime;
	g_csgo.m_globals->m_frametime = g_inputpred.m_frametime;
	g_csgo.m_prediction->m_first_time_predicted = g_inputpred.m_first_time_predicted;
	g_csgo.m_prediction->m_in_prediction = g_inputpred.m_in_prediction;
}
