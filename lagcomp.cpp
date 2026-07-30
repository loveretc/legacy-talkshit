#include "includes.h"

Extrapolation g_extrapolation{};;

bool Extrapolation::HandleLagCompensation(AimPlayer* data) {

	// reset delayed state
	data->m_delayed = false;


	// we have no data to work with.
	// this should never happen if we call this
	if (data->m_records.empty() || data->m_records.size() <= 2)
		return false;

	// meme.
	if (data->m_player->dormant())
		return false;

	// get first record.
	LagRecord* record = data->m_records[0].get();



	// reset all prediction related variables.
	// this has been a recurring problem in all my hacks lmfao.
	// causes the prediction to stack on eachother.
	record->predict();
	record->m_broke_lc = record->broke_lc();

	// we are not breaking lagcomp at this point.
	// return false so it can aim at all the records at once since server-sided lagcomp is still active and we can abuse that.
	if (!record->m_broke_lc)
		return false;


	if (g_menu.main.aimbot.fakelag_correction.get() == 0)
		return true;

	const int sv_ticks_since_upd = g_cl.m_server_tick - record->m_tick;

	// hardcoded for now
	if (sv_ticks_since_upd <= 1)
		return true;

	if (sv_ticks_since_upd >= g_cl.m_latency_ticks) {
		data->m_delayed = true;
		return true;
	}

	if (record->m_lag > 16) {
		data->m_delayed = true;
		return true;
	}

	const float outgoing = g_csgo.m_engine->GetNetChannelInfo()->GetLatency(INetChannel::FLOW_OUTGOING);
	const int receive_tick = std::abs((g_csgo.m_cl->m_server_tick + (game::TIME_TO_TICKS(outgoing))) - game::TIME_TO_TICKS(record->m_sim_time));

	const float delta = static_cast<float>(receive_tick) / static_cast<float>(record->m_lag);
	const int adjusted_arrive_tick = std::clamp(game::TIME_TO_TICKS(((outgoing)+g_csgo.m_globals->m_realtime) - record->m_realtime), 0, 100);

	if (g_menu.main.aimbot.fakelag_correction.get() == 1) {
		data->m_delayed = true;
		return true;
	}

	float total_latency = g_csgo.m_net->GetLatency(0) + g_csgo.m_net->GetLatency(1);
	const float time_delta = record->get_time_delta();

	// if there will be no delay if we reached max lag
	// however it will be delayed by 1tick if there is remaining lag to choke
	const int delay = g_cl.m_lag >= g_cl.m_max_lag ? 0 : 1;
	const int latency_ticks = game::TIME_TO_TICKS(g_csgo.m_net->GetLatency(0) + g_csgo.m_net->GetLatency(1));
	const float delta2 = static_cast<float>(delay + g_csgo.m_cl->m_server_tick + latency_ticks - record->m_tick) / static_cast<float>(record->m_lag);
	const float max = static_cast<float>(game::TIME_TO_TICKS(time_delta - 0.2f)) / static_cast<float>(record->m_lag);
	const auto clamped_delta = std::min(delta2, max);

	extrapolation_data_t pred_data{ data->m_player, record };

	for (int i{}; i < record->m_lag; ++i) {
		pred_data.m_sim_time += g_csgo.m_globals->m_interval;
		SimulateMovement(pred_data);
	}

	record->m_extrapolated = true;
	record->m_pred_time = pred_data.m_sim_time;
	record->m_pred_flags = pred_data.m_flags;
	record->m_pred_origin = pred_data.m_origin;
	record->m_pred_velocity = pred_data.m_velocity;

	const vec3_t origin_delta = pred_data.m_origin - record->m_origin;

	// shift the matrix over by however much he moved.
	// note; m_bone_count, not 128. the arrays are exactly 128 entries long and the
	//       old loop ran to 128 inclusive, writing one matrix past the end of them.
	for (int i{ }; i < record->m_bone_count; ++i) {
		record->m_extrap_bones[i][0][3] += origin_delta.x;
		record->m_extrap_bones[i][1][3] += origin_delta.y;
		record->m_extrap_bones[i][2][3] += origin_delta.z;
	}

	return true;
}

// function: slides along whatever we bump into, same idea as CGameMovement::TryPlayerMove.
static vec3_t ClipMove(const extrapolation_data_t& data, const vec3_t& start, vec3_t velocity, float time_left) {
	CGameTrace            trace{ };
	CTraceFilterWorldOnly filter{ };

	vec3_t pos = start;

	// the game allows up to 4 bumps per move.
	for (int bump{ }; bump < 4; ++bump) {

		if (time_left <= 0.f || velocity.length_sqr() <= 0.f)
			break;

		g_csgo.m_engine_trace->TraceRay(
			{ pos, pos + velocity * time_left, data.m_obb_min, data.m_obb_max },
			CONTENTS_SOLID, &filter, &trace
		);

		pos = trace.m_endpos;

		// made it all the way, we're done.
		if (trace.m_fraction == 1.f)
			break;

		time_left -= time_left * trace.m_fraction;

		// clip our velocity to the plane we ran into.
		velocity -= trace.m_plane.m_normal * velocity.dot(trace.m_plane.m_normal);

		const float adjust = velocity.dot(trace.m_plane.m_normal);
		if (adjust < 0.f)
			velocity -= trace.m_plane.m_normal * adjust;
	}

	return pos;
}

void Extrapolation::SimulateMovement(extrapolation_data_t& data) {

	static ConVar* sv_maxvelocity = g_csgo.m_cvar->FindVar(HASH("sv_maxvelocity"));

	const float interval = g_csgo.m_globals->m_interval;
	const float gravity = g_csgo.sv_gravity->GetFloat();
	const bool  on_ground = (data.m_flags & FL_ONGROUND) != 0;

	if (on_ground) {
		// he is standing on something. if he was airborne the tick before, assume he
		// keeps holding jump like every single player in this game does.
		data.m_velocity.z = data.m_was_in_air ? g_csgo.sv_jump_impulse->GetFloat() : 0.f;
	}
	else {
		// note; gravity is applied in halves around the move ( StartGravity / FinishGravity ),
		//       the old code applied a full tick of it and only while he was ON the ground.
		data.m_velocity.z -= gravity * interval * 0.5f;

		// the game clamps air speed when bunnyhopping is disabled.
		if (!g_csgo.sv_enablebunnyhopping->GetInt()) {
			const float speed = data.m_velocity.length();
			const float max_speed = data.m_player->m_flMaxspeed() * 1.1f;

			if (max_speed > 0.f && speed > max_speed)
				data.m_velocity *= (max_speed / speed);
		}
	}

	// remember what we entered this tick as, the flags get recomputed further down.
	const bool was_airborne = !on_ground;
	data.m_was_in_air = was_airborne;

	// CGameMovement::CheckVelocity, clamps every axis on its own.
	if (sv_maxvelocity) {
		const float max_velocity = sv_maxvelocity->GetFloat();

		math::clamp(data.m_velocity.x, -max_velocity, max_velocity);
		math::clamp(data.m_velocity.y, -max_velocity, max_velocity);
		math::clamp(data.m_velocity.z, -max_velocity, max_velocity);
	}

	// regular move.
	vec3_t end = ClipMove(data, data.m_origin, data.m_velocity, interval);

	// if we are walking and got stopped by something, try to step over it.
	// without this everyone extrapolates straight into the first stair they walk up.
	if (on_ground) {
		const vec3_t flat_delta = end - data.m_origin;

		if (flat_delta.length_2d_sqr() < (data.m_velocity.length_2d() * interval) * (data.m_velocity.length_2d() * interval) * 0.99f) {
			CGameTrace            trace{ };
			CTraceFilterWorldOnly filter{ };

			constexpr float step_size = 18.f;

			// move up, forward, then back down. ( CGameMovement::StepMove )
			g_csgo.m_engine_trace->TraceRay(
				{ data.m_origin, data.m_origin + vec3_t(0.f, 0.f, step_size), data.m_obb_min, data.m_obb_max },
				CONTENTS_SOLID, &filter, &trace
			);

			const vec3_t stepped = ClipMove(data, trace.m_endpos, data.m_velocity, interval);

			g_csgo.m_engine_trace->TraceRay(
				{ stepped, stepped - vec3_t(0.f, 0.f, step_size), data.m_obb_min, data.m_obb_max },
				CONTENTS_SOLID, &filter, &trace
			);

			// only keep the stepped move if it actually got us further and landed on
			// something walkable.
			if (trace.m_plane.m_normal.z > 0.7f
				&& (trace.m_endpos - data.m_origin).length_2d_sqr() > flat_delta.length_2d_sqr())
				end = trace.m_endpos;
		}
	}

	data.m_origin = end;

	// second half of the gravity for this tick.
	if (was_airborne)
		data.m_velocity.z -= gravity * interval * 0.5f;

	// figure out if he is standing on anything now. ( CategorizePosition )
	{
		CGameTrace            trace{ };
		CTraceFilterWorldOnly filter{ };

		g_csgo.m_engine_trace->TraceRay(
			{ data.m_origin, { data.m_origin.x, data.m_origin.y, data.m_origin.z - 2.f }, data.m_obb_min, data.m_obb_max },
			CONTENTS_SOLID, &filter, &trace
		);

		data.m_flags &= ~FL_ONGROUND;

		// moving up fast enough means we just jumped, we are not on the ground.
		if (trace.m_fraction != 1.f && trace.m_plane.m_normal.z > 0.7f && data.m_velocity.z <= 140.f)
			data.m_flags |= FL_ONGROUND;
	}
}

