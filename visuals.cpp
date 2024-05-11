#include "includes.h"

Visuals g_visuals{ };;

void Visuals::ModulateWorld() {
	std::vector< IMaterial* > world, props;

	// iterate material handles.
	for (uint16_t h{ g_csgo.m_material_system->FirstMaterial() }; h != g_csgo.m_material_system->InvalidMaterial(); h = g_csgo.m_material_system->NextMaterial(h)) {
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial(h);
		if (!mat)
			continue;

		// store world materials.
		if (FNV1a::get(mat->GetTextureGroupName()) == HASH("World textures"))
			world.push_back(mat);

		// store props.
		else if (FNV1a::get(mat->GetTextureGroupName()) == HASH("StaticProp textures"))
			props.push_back(mat);
	}

	// night
	if (g_menu.main.visuals.world.get(0)) {
		for (const auto& w : world)

			w->ColorModulate(0.17f, 0.16f, 0.18f);

		// IsUsingStaticPropDebugModes my nigga
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(0);
		}

		for (const auto& p : props)
			p->ColorModulate(0.5f, 0.5f, 0.5f);

		//g_csgo.LoadNamedSky(XOR("sky_csgo_night02"));
	}

	// disable night.
	else {
		for (const auto& w : world)
			w->ColorModulate(1.f, 1.f, 1.f);

		// restore r_DrawSpecificStaticProp.
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != -1) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(-1);
		}

		for (const auto& p : props)
			p->ColorModulate(1.f, 1.f, 1.f);
	}

	// transparent props.
		float alpha2 = g_menu.main.visuals.walls_amount.get() / 100;
		for (const auto& w : world)
			w->AlphaModulate(alpha2);

		float alpha = g_menu.main.visuals.transparent_props_amount.get() / 100;
		for (const auto& p : props)
			p->AlphaModulate(alpha);

		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(0);
		}
}

auto clean_item_name = [](const char* name) -> const char*
{
	if (name[0] == 'C')
		name++;

	auto start = strstr(name, "Weapon");
	if (start != nullptr)
		name = start + 6;

	return name;
};

void Visuals::ThirdpersonThink( ) {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	// for whatever reason overrideview also gets called from the main menu.
	if( !g_csgo.m_engine->IsInGame( ) )
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_local && g_cl.m_local->alive( );

	// camera should be in thirdperson.
	if( m_thirdperson ) {

		// if alive and not in thirdperson already switch to thirdperson.
		if( alive && !g_csgo.m_input->CAM_IsThirdPerson( ) )
			g_csgo.m_input->CAM_ToThirdPerson( );

		// if dead and spectating in firstperson switch to thirdperson.
		else if( g_cl.m_local->m_iObserverMode( ) == 4 ) {

			// if in thirdperson, switch to firstperson.
			// we need to disable thirdperson to spectate properly.
			if( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
				g_csgo.m_input->CAM_ToFirstPerson( );
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_cl.m_local->m_iObserverMode( ) = 5;
		}
	}

	// camera should be in firstperson.
	else if( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
		g_csgo.m_input->CAM_ToFirstPerson( );
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
		// get camera angles.
		g_csgo.m_engine->GetViewAngles( offset );

		// get our viewangle's forward directional vector.
		math::AngleVectors( offset, &forward );

		static auto camdist = g_csgo.m_cvar->FindVar(HASH("cam_idealdist"));
		camdist->SetValue(g_menu.main.visuals.thirdperson_distance.get());

		// cam_idealdist convar.
		offset.z = camdist->GetFloat();

		// start pos.
		origin = g_cl.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity( g_cl.m_local );

		g_csgo.m_engine_trace->TraceRay(
			Ray( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ),
			MASK_NPCWORLDSTATIC,
			( ITraceFilter* ) &filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp( tr.m_fraction, 0.f, 1.f );
		offset.z *= tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

constexpr int linesize = 8, linedec = 4;
void Visuals::Hitmarker() {
	static auto cross = g_csgo.m_cvar->FindVar(HASH("weapon_debug_spread_show"));
	cross->SetValue(g_menu.main.visuals.force_xhair.get() && !g_cl.m_local->m_bIsScoped() ? 3 : 0);

	if (!(g_menu.main.players.hitmarker_select.get() == 1))
		return;

	if (g_csgo.m_globals->m_curtime > m_hit_end)
		return;

	if (m_hit_duration <= 0.f)
		return;

	float complete = (g_csgo.m_globals->m_curtime - m_hit_start) / m_hit_duration;
	int x = g_cl.m_width,
		y = g_cl.m_height,
		alpha = (1.f - complete) * 240;

	constexpr int a{ 4 };
	constexpr int b{ a + 5 };
	auto color = Color(240, 240, 240, alpha);

	constexpr int line{ 6 };

	render::line(x / 2 - b, y / 2 - b, x / 2 - a, y / 2 - a, color); // left upper
	render::line(x / 2 - b, y / 2 + b, x / 2 - a, y / 2 + a, color); // left down
	render::line(x / 2 + b, y / 2 + b, x / 2 + a, y / 2 + a, color); // right down
	render::line(x / 2 + b, y / 2 - b, x / 2 + a, y / 2 - a, color); // right upper
}

void Visuals::hitmarker_world() {
	if (!(g_menu.main.players.hitmarker_select.get() == 2))
		return;

	if (hitmarkers.size() == 0)
		return;

	if (!g_cl.m_processing || !g_csgo.m_engine->IsInGame()) {
		if (!m_impacts.empty())
			m_impacts.clear();

		if (!hitmarkers.empty())
			hitmarkers.clear();

		return;
	}

	// draw
	for (int i = 0; i < hitmarkers.size(); i++) {
		vec3_t pos3D = hitmarkers[i].impact.m_impact_pos;
		vec2_t pos2D;

		if (!render::WorldToScreen(pos3D, pos2D))
			continue;

		if (hitmarkers[i].impact.m_impact_pos.IsZero())
			continue;

		float complete = (g_csgo.m_globals->m_curtime - m_hit_start) / m_hit_duration;
		int x = g_cl.m_width,
			y = g_cl.m_height,
			alpha = (1.f - complete) * 240;

		auto color = Color(240, 240, 240, hitmarkers[i].alpha);

		render::line(pos2D.x + linesize, pos2D.y + linesize, pos2D.x + linedec, pos2D.y + linedec, color);
		render::line(pos2D.x - linesize, pos2D.y - linesize, pos2D.x - linedec, pos2D.y - linedec, color);
		render::line(pos2D.x + linesize, pos2D.y - linesize, pos2D.x + linedec, pos2D.y - linedec, color);
		render::line(pos2D.x - linesize, pos2D.y + linesize, pos2D.x - linedec, pos2D.y + linedec, color);
	}

	// proceeed
	for (int i = 0; i < hitmarkers.size(); i++) {
		if (hitmarkers[i].time + 0.1f <= g_csgo.m_globals->m_curtime) {
			hitmarkers[i].alpha -= 1;
		}

		if (hitmarkers[i].alpha <= 0)
			hitmarkers.erase(hitmarkers.begin() + i);
	}

}

void Visuals::NoSmoke() {

	if (!smoke1)
		smoke1 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_fire"), XOR("Other textures"));

	if (!smoke2)
		smoke2 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_smokegrenade"), XOR("Other textures"));

	if (!smoke3)
		smoke3 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_emods"), XOR("Other textures"));

	if (!smoke4)
		smoke4 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_emods_impactdust"), XOR("Other textures"));

	if (g_menu.main.visuals.removals.get(0)) {
		if (!smoke1->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke1->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke2->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke2->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke3->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke3->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke4->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke4->SetFlag(MATERIAL_VAR_NO_DRAW, true);
	}

	else {
		if (smoke1->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke1->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke2->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke2->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke3->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke3->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke4->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke4->SetFlag(MATERIAL_VAR_NO_DRAW, false);
	}

	// godbless alpha and led for adding post process removal to RemoveSmoke.
	// 
	// nitro code (alt to forcing cvar etc)
	static auto DisablePostProcess = g_csgo.postproc;

	// get post process address
	static bool* disable_post_process = *reinterpret_cast<bool**>(DisablePostProcess);

	// set it.
	if (*disable_post_process != g_menu.main.visuals.postprocess.get())
		*disable_post_process = g_menu.main.visuals.postprocess.get();
}

void Visuals::think( ) {
	// don't run anything if our local player isn't valid.
	if( !g_cl.m_local )
		return;

	if (g_menu.main.visuals.removals.get(3)
		&& g_cl.m_local->alive( )
		&& g_cl.m_local->GetActiveWeapon( )
		&& g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_cl.m_local->m_bIsScoped( ) ) {

		// rebuild the original scope lines.
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt( );

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if( size > 1 ) {
			x -= ( size / 2 );
			y -= ( size / 2 );
		}

		// draw our lines.
		render::rect_filled( 0, y, w, size, colors::black );
		render::rect_filled( x, 0, size, h, colors::black );
	}

	auto& predicted_nades = g_grenades_pred.get_list();

	static auto last_server_tick = g_csgo.m_cl->m_server_tick;
	if (g_csgo.m_cl->m_server_tick != last_server_tick) {
		predicted_nades.clear();

		last_server_tick = g_csgo.m_cl->m_server_tick;
	}

	// draw esp on ents.
	for (int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex(); ++i) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity(i);
		if (!ent)
			continue;

		draw(ent);

		if (ent->dormant())
			continue;

		if (!ent->is(HASH("CMolotovProjectile"))
			&& !ent->is(HASH("CBaseCSGrenadeProjectile")))
			continue;

		if (ent->is(HASH("CBaseCSGrenadeProjectile"))) {
			const auto studio_model = ent->GetModel();
			if (!studio_model
				|| std::string_view(studio_model->m_name).find("fraggrenade") == std::string::npos)
				continue;
		}

		const auto handle = reinterpret_cast<Player*>(ent)->GetRefEHandle();

		if (ent->m_fEffects() & EF_NODRAW) {
			predicted_nades.erase(handle);

			continue;
		}

		if (predicted_nades.find(handle) == predicted_nades.end()) {
			predicted_nades.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(handle),
				std::forward_as_tuple(
					reinterpret_cast<Player*>(g_csgo.m_entlist->GetClientEntityFromHandle(ent->m_hThrower())),
					ent->is(HASH("CMolotovProjectile")) ? MOLOTOV : HEGRENADE,
					ent->m_vecOrigin(), ent->m_vecVelocity(), ent->m_flSpawnTime_Grenade(),
					game::TIME_TO_TICKS(reinterpret_cast<Player*>(ent)->m_flSimulationTime() - ent->m_flSpawnTime_Grenade())
				)
			);
		}

		if (predicted_nades.at(handle).draw())
			continue;

		predicted_nades.erase(handle);
	}

	g_grenades_pred.get_local_data().draw();

	// draw everything else.
	StatusIndicators( );
	Spectators( );
	ManualAntiAim();
	PenetrationCrosshair( );
	Hitmarker( );
	hitmarker_world( );
	DrawPlantedC4( );
	AutopeekIndicator();
	ImpactData();
}

void Visuals::Spectators( ) {
	if( !g_menu.main.visuals.spectators.get( ) )
		return;

	std::vector< std::string > spectators{ XOR("") };
	int h = render::esp.m_size.m_height;

	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player)
			continue;

		if (!player->IsPlayer())
			continue;

		if (player->dormant())
			continue;

		//if (player->alive())
			//continue;

		if (player->GetObserverTarget() != g_cl.m_local)
			continue;

		player_info_t info;
		if (!g_csgo.m_engine->GetPlayerInfo(i, &info))
			continue;

		spectators.push_back(std::string(info.m_name).substr(0, 24));
	}

	size_t total_size = spectators.size() * (h - 1);

	for (size_t i{ }; i < spectators.size(); ++i) {
		const std::string& name = spectators[i];

		render::esp.string(g_cl.m_width - 15, 30 - (total_size / 2) + (i * (h + 2)),
			{ 255, 255, 255, 255 }, name, render::ALIGN_RIGHT);
	}
}

Color LerpRGB(Color a, Color b, float t)
{
	return Color
	(
		a.r() + (b.r() - a.r()) * t,
		a.g() + (b.g() - a.g()) * t,
		a.b() + (b.b() - a.b()) * t,
		a.a() + (b.a() - a.a()) * t
	);
}

float GetPingSpikeAmmount() {
	const auto channel_info = g_csgo.m_engine->GetNetChannelInfo();

	if (channel_info == nullptr)
		return 0;

	auto average_latency = channel_info->GetAvgLatency(1);

	static auto cl_updaterate = g_csgo.m_cvar->FindVar(HASH("cl_updaterate"));

	if (cl_updaterate->GetFloat() > 0.001f)
		average_latency += -0.5f / cl_updaterate->GetFloat();

	return std::fabs(average_latency) * 230.0f;
}


void Visuals::StatusIndicators() {
	// dont do if dead.
	if (!g_cl.m_processing)
		return;

	// compute hud size.
	// int size = ( int )std::round( ( g_cl.m_height / 17.5f ) * g_csgo.hud_scaling->GetFloat( ) );

	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };

	// LBY
	if (g_menu.main.antiaim.enable.get()) {
		// get the absolute change between current lby and animated angle.
		float change = std::abs(math::NormalizedAngle(g_cl.m_body - g_cl.m_angle.y));

		Indicator_t ind{ };
		ind.color = change > 35.f ? Color(150, 200, 60) : Color(255, 0, 0);
		ind.text = XOR("LBY");
		indicators.push_back(ind);
	}

	// LC
	if (g_cl.m_local->m_vecVelocity().length_2d() > 270.f || g_cl.m_lagcomp) {
		Indicator_t ind{ };
		ind.color = g_cl.m_lagcomp ? Color(150, 200, 60) : Color(255, 0, 0);
		ind.text = XOR("LC");

		indicators.push_back(ind);
	}

	// DMG
	if (g_aimbot.m_damage_toggle) {
		Indicator_t ind{ };
		ind.color = Color(255, 255, 255);
		int dmg = g_menu.main.aimbot.override_dmg_value.get();
		std::string damage_string;
		if (dmg > 100)
			ind.text = tfm::format(XOR("DMG: HP+%i"), (g_menu.main.aimbot.override_dmg_value.get() - 100));
		else if (dmg == 0)
			ind.text = tfm::format(XOR("DMG: Auto"));
		else
			ind.text = tfm::format(XOR("DMG: %i"), g_menu.main.aimbot.override_dmg_value.get());
		indicators.push_back(ind);
	}

	// BAIM
	if (g_aimbot.m_force_body) {
		Indicator_t ind{ };
		ind.color = Color(150, 200, 60);
		ind.text = XOR("BAIM");
		indicators.push_back(ind);
	}

	// PING
	if (g_aimbot.m_fake_latency || g_aimbot.m_fake_latency2) {
		//for color
		float p_r = 255 - (GetPingSpikeAmmount() * 2.29824561404);
		float p_g = GetPingSpikeAmmount() * 3.42105263158;
		float p_b = GetPingSpikeAmmount() * 0.22807017543;

		std::max(0, (int)std::round(g_cl.m_latency * 1000.f));

		int r = std::min((int)p_r, 255);
		int g = std::min((int)p_g, 255);
		int b = std::min((int)p_b, 255);

		Color col = { r, g, b };
		//if (g > 255)
			//col = { r, 255, b };


		Indicator_t ind{ };
		//ind.color = 0xff15c27b;
		ind.color = col;
		ind.text = XOR("PING");
		indicators.push_back(ind);
	}
	
	// DT
	if (g_aimbot.m_double_tap) {
		Indicator_t ind{ };
		ind.color = g_aimbot.m_double_tap ? 0xffffffff : 0xff0000ff;
		ind.text = XOR("DT");

		indicators.push_back(ind);
	}

	//LOL im high 24/7 keep crying youre so mad and I think I know why ahHHAhahaha
	if (indicators.empty())
		return;

	// iterate and draw indicators.
	for (size_t i{ }; i < indicators.size(); ++i) {
		auto& indicator = indicators[i];

		render::indicator.string(12, g_cl.m_height - 64 - (30 * i), indicator.color, indicator.text);
	}

	auto local_player = g_cl.m_local;
	int screen_width, screen_height;
	g_csgo.m_engine->GetScreenSize(screen_width, screen_height);
	static float next_lby_update[65];
	const float curtime = g_csgo.m_globals->m_curtime;

	if (local_player->m_vecVelocity().length_2d() > 0.1f && !g_input.GetKeyState(g_menu.main.misc.fakewalk.get()))
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	static float last_lby[65];
	if (last_lby[local_player->index()] != local_player->m_flLowerBodyYawTarget())
	{
		last_lby[local_player->index()] = local_player->m_flLowerBodyYawTarget();
		next_lby_update[local_player->index()] = curtime + 1.125f + g_csgo.m_globals->m_interval;
	}

	if (next_lby_update[local_player->index()] < curtime)
		next_lby_update[local_player->index()] = curtime + 1.125f;

	float time_remain_to_update = next_lby_update[local_player->index()] - local_player->m_flSimulationTime();
	float time_update = next_lby_update[local_player->index()];
	float fill = 0;
	fill = (((time_remain_to_update)));
	static float add = 0.000f;
	add = 1.125f - fill;

	float change1337 = std::abs(math::NormalizedAngle(g_cl.m_body - g_cl.m_angle.y));
	Color color1337 = { 255,0,0,255 };

	if (change1337 > 35.f)
		color1337 = { 124,195,13,255 };

	if (!((g_cl.m_buttons & IN_JUMP) || !(g_cl.m_flags & FL_ONGROUND)) && g_menu.main.antiaim.enable.get()) {
		render::draw_arc(12 + 60, g_cl.m_height - 64 + 23 - 9, 8, 0, 360, 5, { 0,0,0,125 });
		render::draw_arc(12 + 60, g_cl.m_height - 64 + 23 - 9, 7, 0, 340 * add, 3, color1337);
	}
}

void Visuals::ManualAntiAim() {

	// dont do if dead.
	if (!g_menu.main.visuals.manual_anti_aim_indic.get() || !g_cl.m_processing)
		return;

	int x, y;

	// calculate screen center
	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;
	
	// manage arrow colors, dont let user modify alpha (dont like full-bright arrows :v)
	Color color = g_menu.main.visuals.manual_anti_aim_col.get();
	Color fc = (g_hvh.m_left ? Color(color.r(), color.g(), color.b(), 180) : Color(0, 0, 0, 130));
	Color sc = (g_hvh.m_right ? Color(color.r(), color.g(), color.b(), 180) : Color(0, 0, 0, 130));
	Color tc = (g_hvh.m_back ? Color(color.r(), color.g(), color.b(), 180) : Color(0, 0, 0, 130));

	// left arrow
	render::triangle(vec2_t(x - 45, y + 10), vec2_t(x - 65, y), vec2_t(x - 45, y - 10), fc);

	// right arrow
	render::triangle(vec2_t(x + 45, y - 10), vec2_t(x + 65, y), vec2_t(x + 45, y + 10), sc);

	// back arrow
	render::triangle(vec2_t(x, y + 70), vec2_t(x - 10, y + 50), vec2_t(x + 10, y + 50), tc);

}

void Visuals::PenetrationCrosshair() {
	int   x, y;
	bool  valid_player_hit;
	Color final_color;

	if (!g_menu.main.visuals.pen_crosshair.get() || !g_cl.m_processing)
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;


	valid_player_hit = (g_cl.m_pen_data.m_target && g_cl.m_pen_data.m_target->enemy(g_cl.m_local));
	if (valid_player_hit)
		final_color = colors::light_blue;

	else if (g_cl.m_pen_data.m_pen)
		final_color = colors::transparent_green;

	else
		final_color = colors::transparent_red;

	// todo - dex; use fmt library to get damage string here?
	//             draw damage string?

	// draw small square in center of screen.
	int damage1337 = g_cl.m_pen_data.m_damage;

	if (g_cl.m_pen_data.m_pen || valid_player_hit)
		render::esp_small.string(x + 3, y + 2, { final_color }, std::to_string(damage1337).c_str(), render::ALIGN_LEFT);
	if (g_cl.m_pen_data.m_damage > 1) {
		render::rect_filled(x - 1, y, 1, 1, { final_color });
		render::rect_filled(x, y, 1, 1, { final_color });
		render::rect_filled(x + 1, y, 1, 1, { final_color });
		render::rect_filled(x, y + 1, 1, 1, { final_color });
		render::rect_filled(x, y - 1, 1, 1, { final_color });
		////shadow
		//render::rect_filled(x - 2, y, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x + 1, y - 1, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x + 2, y, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x, y + 2, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x, y - 2, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x + 1, y - 2, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x + 1, y + 1, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x + 2, y + 1, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x - 1, y + 1, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x - 1, y + 2, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x - 2, y + 1, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x + 1, y + 2, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x - 1, y - 1, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x - 1, y - 2, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x - 2, y - 1, 1, 1, { 0,0, 0, 125 });
		//render::rect_filled(x + 2, y - 1, 1, 1, { 0,0, 0, 125 });
	}
}

void Visuals::draw( Entity* ent ) {

	if ( !g_cl.m_local || !g_csgo.m_engine->IsInGame()) {
		g_cl.kaaba.clear();
		g_cl.cheese.clear();
		g_cl.dopium.clear();
		g_cl.same_hack.clear();
		g_cl.fade.clear();
		g_cl.roberthook.clear();
		return;
	}

	if( ent->IsPlayer( ) ) {
		Player* player = ent->as< Player* >( );

		if( player->m_bIsLocalPlayer( ) )
			return;

		// draw player esp.
		DrawPlayer( player );
	}

	 if (ent->IsBaseCombatWeapon() && !ent->dormant())
		DrawItem(ent->as< Weapon* >());

	 if( g_menu.main.visuals.proj.get( ) )
		DrawProjectile( ent->as< Weapon* >( ) );
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 3)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return out.str();
}

void Visuals::DrawProjectile(Weapon* ent) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	Color col = g_menu.main.visuals.proj_color.get();
	Color moly_color = g_menu.main.visuals.molotov_radius.get();
	Color smoke_color = g_menu.main.visuals.smoke_radius.get();
	Color text_col = { 240, 240, 240, 200 };
	col.a() = 0xb4;
	auto dist_world = g_cl.m_local->m_vecOrigin().dist_to(origin);
	if (dist_world > 150.f) {
		col.a() *= std::clamp((750.f - (dist_world - 200.f)) / 750.f, 0.f, 1.f);
	}

	// premium animation here
	// fade elements depending on your distance, this way you dont have your entire screen filled with indicators
	// use linear decrease
	int dist = (((ent->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;
	int fading_alpha, warning_alpha, warning_red, warning_icon;
	float pulse_value = 0.5f * (1 + sin(3 * M_PI * g_csgo.m_globals->m_curtime));
	
	// World circle
	if (dist <= 17) {
		fading_alpha = 255;
	}
	else if (dist > 17 && dist <= 30) {
		fading_alpha = 255 - ((255 / (30 - 17)) * (dist - 17));
	}
	else {
		fading_alpha = 0;
	}

	// Warning circle
	if (dist <= 30) {
		warning_alpha = 230;
		warning_icon = 255;
	}
	else if (dist > 30 && dist <= 35) {
		warning_alpha = 230 - ((230 / (35 - 30)) * (dist - 30));
		warning_icon = 255 - ((255 / (35 - 30)) * (dist - 30));
	}
	else {
		warning_alpha = 0;
		warning_icon = 0;
	}

	// Red channel for grenade warning
	if (dist <= 3) {
		// apply a pulse effect when you are near the molotov
		warning_red = static_cast<int>(150 + pulse_value * 30); // range: [120, 180]
	}
	else if (dist > 3 && dist <= 8) {
		warning_red = 120 - ((120 / (8 - 3)) * (dist - 3));
	}
	else {
		warning_red = 26;
	}

	// draw decoy.
	if (ent->is(HASH("CDecoyProjectile")))
		render::esp_small.string(screen.x, screen.y, text_col, XOR("DECOY"), render::ALIGN_CENTER);

	// draw molotov.
	else if (ent->is(HASH("CMolotovProjectile")))
		render::esp_small.string(screen.x, screen.y, text_col, XOR("MOLLY"), render::ALIGN_CENTER);

	else if (ent->is(HASH("CBaseCSGrenadeProjectile"))) {
		const model_t* model = ent->GetModel();

		if (model) {
			// grab modelname.
			std::string name{ ent->GetModel()->m_name };

			if (name.find(XOR("flashbang")) != std::string::npos)
				render::esp_small.string(screen.x, screen.y, text_col, XOR("FLASH"), render::ALIGN_CENTER);

			else if (name.find(XOR("fraggrenade")) != std::string::npos)
				render::esp_small.string(screen.x, screen.y, text_col, XOR("FRAG"), render::ALIGN_CENTER);
		}
	}
	// find classes.
	else if (ent->is(HASH("CInferno"))) {

		const double spawn_time = *(float*)(uintptr_t(ent) + 0x20);
		const double factor = ((spawn_time + 7.031) - g_csgo.m_globals->m_curtime) / 7.031;
		Color col_timer = g_menu.main.visuals.proj_color.get();
		if (spawn_time > 0.f && g_menu.main.visuals.proj.get()) {
			// render our bg then timer colored bar
			float radius = 144.f;
			

			render::WorldCircleOutline(origin, radius, 1.f, Color(moly_color.r(), moly_color.g(), moly_color.b(), fading_alpha));

			if (g_menu.main.visuals.grenade_warning.get()) {
				render::circle(screen.x, screen.y - 10, 20, 360, Color(warning_red, 26, 30, warning_alpha));
				render::draw_arc(screen.x, screen.y - 10, 20, 0, 360 * factor, 1.5, Color(moly_color.r(), moly_color.g(), moly_color.b(), warning_alpha));
				render::undefeated.string(screen.x + 7, screen.y - 22, { 255,255,255,warning_icon }, XOR("n"), render::ALIGN_RIGHT);
			}

			// render our timer in seconds and our title text
			if (dist <= 50) {
				render::round_rect(screen.x - 13 + 1, screen.y + 9, 26, 4, 2, Color(0, 0, 0, 200));
				render::round_rect(screen.x - 13 + 2, screen.y + 9 + 1, 24 * factor, 2, 2, Color(col_timer.r(), col_timer.g(), col_timer.b(), 200));

				render::esp_small.string(screen.x - 13 + 26 * factor, screen.y + 7, text_col, tfm::format(XOR("%.1f"), (spawn_time + 7.031) - g_csgo.m_globals->m_curtime), render::ALIGN_CENTER);
				render::esp_small.string(screen.x, screen.y, text_col, XOR("FIRE"), render::ALIGN_CENTER);
			}
		}
	}

	else if (ent->is(HASH("CSmokeGrenadeProjectile"))) {
		float radius = 144.f;	
		const float spawn_time = game::TICKS_TO_TIME(ent->m_nSmokeEffectTickBegin());
		const double factor = ((spawn_time + 18.041) - g_csgo.m_globals->m_curtime) / 18.041;
		Color col_timer = g_menu.main.visuals.proj_color.get();
		if (spawn_time > 0.f && g_menu.main.visuals.proj.get()) {
			

			int starting_alpha = 5;
			
			if (g_menu.main.visuals.grenade_warning.get()) {
				render::circle(screen.x, screen.y - 10, 20, 360, Color(26, 26, 30, warning_alpha));
				render::draw_arc(screen.x, screen.y - 10, 20, 0, 360 * factor, 1.5, Color(smoke_color.r(), smoke_color.g(), smoke_color.b(), warning_alpha));
				render::undefeated.string(screen.x + 7, screen.y - 22, { 255,255,255,warning_icon }, XOR("k"), render::ALIGN_RIGHT);
			}

			render::WorldCircleOutline(origin, radius, 1.f, Color(smoke_color.r(), smoke_color.g(), smoke_color.b(), fading_alpha));

			// render our timer in seconds and our title text
			if (dist <= 50) {
				// render our bg then timer colored bar
				render::round_rect(screen.x - 13 + 1, screen.y + 9, 26, 4, 2, Color(0, 0, 0, 200));
				render::round_rect(screen.x - 13 + 2, screen.y + 9 + 1, 24 * factor, 2, 2, Color(col_timer.r(), col_timer.g(), col_timer.b(), 200));

				render::esp_small.string(screen.x - 13 + 26 * factor, screen.y + 7, text_col, tfm::format(XOR("%.1f"), (spawn_time + 18.04125) - g_csgo.m_globals->m_curtime), render::ALIGN_CENTER);
				render::esp_small.string(screen.x, screen.y, text_col, XOR("SMOKE"), render::ALIGN_CENTER);
			}
		}
	}
}

void Visuals::AutopeekIndicator() {
	// dont do if dead.
	if (!g_cl.m_processing)
		return;

	auto weapon = g_cl.m_local->GetActiveWeapon();

	if (!weapon)
		return;

	static auto position = vec3_t(0.f, 0.f, 0.f);

	if (!g_movement.start_position.IsZero())
		position = g_movement.start_position;

	if (position.IsZero())
		return;

	static auto alpha = 0.0f;

	if (g_input.GetKeyState(g_menu.main.aimbot.quickpeekassist.get()) || alpha) {

		if (g_input.GetKeyState(g_menu.main.aimbot.quickpeekassist.get()))
			alpha += 85.0f * g_csgo.m_globals->m_frametime;
		else
			alpha -= 85.0f * g_csgo.m_globals->m_frametime;

		alpha = math::dont_break(alpha, 0.0f, 15.0f);
		render::Draw3DFilledCircle(position, alpha, g_menu.main.aimbot.autopeek_active.get());
		//render::Draw3DCircle(position, 15.0f, outer_color);
	}
}

void Visuals::DrawItem(Weapon* item) {
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle(item->m_hOwnerEntity());
	if (owner)
		return;

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	WeaponInfo* data = item->GetWpnData();
	if (!data)
		return;

	Color col = g_menu.main.visuals.item_color.get();
	int alpha1 = g_menu.main.visuals.item_color_alpha.get();

	std::string distance;
	int dist = (((item->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;
	//if (dist > 0)
	//distance = tfm::format(XOR("%i FT"), dist);
	if (dist > 0) {
		if (dist > 5) {
			while (!(dist % 5 == 0)) {
				dist = dist - 1;
			}

			if (dist % 5 == 0)
				distance = tfm::format(XOR("%i FT"), dist);
		}
		else
			distance = tfm::format(XOR("%i FT"), dist);
	}

	// render bomb in green.
	if (item->is(HASH("CC4")))
		render::esp_small.string(screen.x, screen.y, {150, 200, 60, 0xb4 }, XOR("BOMB"), render::ALIGN_CENTER);

	if (item->is(HASH("CC4")))
		return;

	if (dist > 25)
		return;

	std::string name{ clean_item_name(item->GetClientClass()->m_pNetworkName) };
	std::transform(std::execution::par, name.begin(), name.end(), name.begin(), ::toupper);

	if (g_menu.main.visuals.dropped_weapons.get(0))
		render::esp_small.string(screen.x, screen.y, Color(col.r(), col.g(), col.b(), alpha1), name, render::ALIGN_CENTER);

	if (g_menu.main.visuals.dropped_weapons.get(1))
		render::esp_small.string(screen.x, screen.y - 10, Color(col.r(), col.g(), col.b(), alpha1), distance, render::ALIGN_CENTER);

	if (g_menu.main.visuals.dropped_weapons.get(2)){

		// nades do not have ammo.
		if (data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE)
			return;

		if (item->m_iItemDefinitionIndex() == 0 || item->m_iItemDefinitionIndex() == C4)
			return;

		std::string ammo = tfm::format(XOR("[ %i/%i ]"), item->m_iClip1(), item->m_iPrimaryReserveAmmoCount());
		std::string icon = tfm::format(XOR("%c"), m_weapon_icons[item->m_iItemDefinitionIndex()]);
	
		const int current = item->m_iClip1();
		const int max = data->m_max_clip1;
		const float scale = (float)current / max;
		const int width_ = render::esp_small.size(name.c_str()).m_width;
	
		int bar = (int)std::round( ( width_ - 1 ) * scale);
		render::rect_filled(screen.x - int( width_ / 2.f ), screen.y + 12, render::esp_small.size(name.c_str()).m_width + 1, 4, Color(0, 0, 0, alpha1));
		render::rect_filled(screen.x - int( width_ / 2.f ) + 1, screen.y + 1 + 12, bar, 2, Color(col.r(), col.g(), col.b(), alpha1));
	}
}

void Visuals::OffScreen(Player* player, int alpha) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[3], verts_outline[3];
	Color  color;

	// todo - dex; move this?
	static auto get_offscreen_data = [](const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation) {
		ang_t  view_angles(g_csgo.m_view_render->m_view.m_angles);
		vec3_t fwd, right, up(0.f, 0.f, 1.f);
		float  front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::AngleVectors(view_angles, &fwd);

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize();

		// calculate front / side positions.
		right = up.cross(fwd);
		front = delta.dot(fwd);
		side = delta.dot(right);

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation ( yaw, 0 - 360 ).
		out_rotation = math::rad_to_deg(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + math::pi);

		// get needed sine / cosine values.
		yaw_rad = math::deg_to_rad(-out_rotation);
		sa = std::sin(yaw_rad);
		ca = std::cos(yaw_rad);

		// rotate offscreen position around.
		out_offscreen_pos.x = (int)((g_cl.m_width / 2.f) + (radius * sa));
		out_offscreen_pos.y = (int)((g_cl.m_height / 2.f) - (radius * ca));
	};

	if (!g_menu.main.players.offscreen.get())
		return;

	if (!g_cl.m_processing || !g_cl.m_local->enemy(player))
		return;

	// get the player's center screen position.
	target_pos = player->WorldSpaceCenter();
	is_on_screen = render::WorldToScreen(target_pos, screen_pos);

	// give some extra room for screen position to be off screen.
	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if (!is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >(g_cl.m_width + leeway_x)
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >(g_cl.m_height + leeway_y)) {

		float size = g_menu.main.players.offscreen_size.get() / 16.f;
		float pos = g_menu.main.players.offscreen_distance.get() * 2.f;

		// get viewport origin.
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		// get direction to target.
		delta = (target_pos - view_origin).normalized();

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = pos * (g_cl.m_height / 480.f);

		// get the data we need for rendering.
		get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[0] = { offscreen_pos.x + (1 * size) , offscreen_pos.y + (1 * size) };        // 0,  0
		verts[1] = { offscreen_pos.x - (12.f * size), offscreen_pos.y + (24.f * size) }; // -1, 1
		verts[2] = { offscreen_pos.x + (12.f * size), offscreen_pos.y + (24.f * size) }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[0] = { verts[0].m_pos.x - 1.f, verts[0].m_pos.y - 1.f };
		verts_outline[1] = { verts[1].m_pos.x - 1.f, verts[1].m_pos.y + 1.f };
		verts_outline[2] = { verts[2].m_pos.x + 1.f, verts[2].m_pos.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[0] = render::RotateVertex(offscreen_pos, verts[0], offscreen_rotation);
		verts[1] = render::RotateVertex(offscreen_pos, verts[1], offscreen_rotation);
		verts[2] = render::RotateVertex(offscreen_pos, verts[2], offscreen_rotation);
		// verts_outline[ 0 ] = render::RotateVertex( offscreen_pos, verts_outline[ 0 ], offscreen_rotation );
		// verts_outline[ 1 ] = render::RotateVertex( offscreen_pos, verts_outline[ 1 ], offscreen_rotation );
		// verts_outline[ 2 ] = render::RotateVertex( offscreen_pos, verts_outline[ 2 ], offscreen_rotation );

		// render!
		int alpha1337 = sin(abs(fmod(-math::pi + (g_csgo.m_globals->m_curtime * (2 / .75)), (math::pi * 2)))) * 255;

		if (alpha1337 < 0)
			alpha1337 = alpha1337 * (-1);

		color = g_menu.main.players.offscreen_color.get(); // damage_data.m_color;
		color.a() = (alpha == 255) ? alpha1337 : alpha / 2;
		g_csgo.m_surface->DrawSetColor(color);
		g_csgo.m_surface->DrawTexturedPolygon(3, verts);

		// g_csgo.m_surface->DrawSetColor( colors::black );
		// g_csgo.m_surface->DrawTexturedPolyLine( 3, verts_outline );
	}
}

std::string Visuals::GetWeaponIcon(const int id) {
	auto search = m_weapon_icons.find(id);
	if (search != m_weapon_icons.end())
		return std::string(&search->second, 1);

	return XOR("");
}

void Visuals::DrawPlayer( Player* player ) {
	Rect		  box;
	player_info_t info;

	// get player index.
	int index = player->index( );

	// get reference to array variable.
	bool& draw = m_draw[ index - 1 ];

	// first opacity should reach 1 in 250 ms second will reach 1 in 500 ms.
	constexpr int frequency = 1.f / 0.250f;
	constexpr int s_frequency = 1.f / 0.500f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;
	float slow_frequency = s_frequency * g_csgo.m_globals->m_frametime;

	// is player enemy.
	bool dormant = player->dormant( );
	bool enemy = player->enemy( g_cl.m_local );

	if( g_menu.main.visuals.enemy_radar.get( ) /* && !dormant*/ )
		player->m_bSpotted( ) = true;

	// we can draw this player again.
	if( !dormant )
		draw = true;

	if( !draw )
		return;

	// is dormant esp enabled for this player.
	bool dormant_esp = g_menu.main.players.dormant.get( );

	if( !dormant_esp && player->dormant( ) )
		return;

	// get player info.
	if( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	bool valid_dormant = false;

	// i think this is pandora fade when dead shit!
	if (!player->alive())
		m_opacities[index] -= slow_frequency;

	if (!player->dormant()) {
		g_sound.m_cSoundPlayers[player->index()].reset(true, player->GetAbsOrigin(), player->m_fFlags());
	}
	else
		valid_dormant = g_sound.AdjustPlayerBegin(player);

	if (player->dormant())
	{
		const vec3_t origin = player->GetAbsOrigin();

		if (origin.is_zero())
			m_opacities[index] = 0.0f;
		if (!valid_dormant && m_opacities[index] > 0.0f)
			m_opacities[index] -= step;
		else if (valid_dormant && m_opacities[index] < 1.f)
			m_opacities[index] += step;
	}
	else if (m_opacities[index] < 1.f && player->alive() && !player->dormant())
		m_opacities[index] += step;

	// get color based on enemy or not.
	Color color = g_menu.main.players.box_enemy.get();

	m_opacities[index] = std::clamp(m_opacities[index], 0.f, 1.f);

	const int alpha = 255.f * m_opacities[index];
	const int low_alpha = int(alpha / /*1.41*/ 1.7);
	const int weapon_alpha = int(alpha / /*1.41*/ 1.41);
	const int box_alpha = int(alpha / 1.21);

	if (dormant && dormant_esp) {
		// override color.
		color = Color(220, 220, 220);
	}
	
	// get player info.
	if( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	// run offscreen ESP.
	OffScreen( player, alpha );

	// attempt to get player box.
	if( !GetPlayerBoxRect( player, box ) ) {
		// OffScreen( player );
		return;
	}

	// DebugAimbotPoints( player );

	const bool bone_esp = ( enemy && g_menu.main.players.skeleton.get(  ) ) || ( !enemy && g_menu.main.players.teammates.get(  ) );
	if( bone_esp )
		DrawSkeleton( player, alpha );

	const bool hist_skelet = (enemy && g_menu.main.players.history_skeleton.get()) || (!enemy && g_menu.main.players.teammates.get());
	if (hist_skelet)
		DrawHistorySkeleton(player, alpha);

	// is box esp enabled for this player.
	const bool box_esp = ( enemy && g_menu.main.players.box.get( ) ) || ( !enemy && g_menu.main.players.teammates.get( ) );

	// render box if specified.
	if (box_esp) {
		if (dormant)
			render::rect_outlined(box.x, box.y, box.w, box.h, Color(210, 210, 210, alpha), { 0,0,0, low_alpha });
		else
			render::rect_outlined(box.x, box.y, box.w, box.h, Color(color.r(), color.g(), color.b(), color.a()).alpha(alpha), Color(0, 0, 0, color.a()).alpha( low_alpha ) );
	}

	// is name esp enabled for this player.
	const bool name_esp = ( enemy && g_menu.main.players.name.get(  ) ) || ( !enemy && g_menu.main.players.teammates.get(  ) );


	// draw name esp
	if (name_esp) {
		// fix retards with their namechange meme 
		// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat
		std::string name{ std::string(info.m_name).substr(0, 24) };

		// smallfonts needs upper case.
		//if (g_menu.main.players.name_style.get() == 0)
		//	std::transform(std::execution::par, name.begin(), name.end(), name.begin(), ::toupper);

		Color clr = g_menu.main.players.name_color.get();
		// override alpha.
		clr.a() = low_alpha;

		//std::for_each(name.begin(), name.end(), [](char& c) {
			//c = ::tolower(c);
		//});

		if (dormant)
			render::esp.string(box.x + box.w / 2, box.y - render::esp.m_size.m_height, Color(210, 210, 210, low_alpha), name, render::ALIGN_CENTER);
		else
			render::esp.string(box.x + box.w / 2, box.y - render::esp.m_size.m_height, Color(clr.r(), clr.g(), clr.b(), clr.a()).alpha(low_alpha), name, render::ALIGN_CENTER);
		
	}

	const // is health esp enabled for this player.
	bool health_esp = ( enemy && g_menu.main.players.health.get(  ) ) || ( !enemy && g_menu.main.players.teammates.get(  ) );

	if( health_esp ) {
		int y = box.y + 1;
		int h = box.h - 2;


		int hp = std::min(100, player->m_iHealth());
		static float player_hp[64];

		if (player_hp[player->index()] > hp)
			player_hp[player->index()] -= 270 * g_csgo.m_globals->m_frametime;
		else
			player_hp[player->index()] = hp;

		hp = player_hp[player->index()];

		// calculate hp bar color.
		int r = std::min( ( 510 * ( 100 - hp ) ) / 100, 255 );
		int g = std::min( ( 510 * hp ) / 100, 255 );

		// get hp bar height.
		int fill = ( int ) std::round( hp * h / 100.f );

		// render background.
		render::rect_filled( box.x - 6, y - 2, 4, h + 4, { 0, 0, 0, low_alpha } );

		// render actual bar.
		if (dormant)
			render::rect( box.x - 5, y - 1 + h - fill, 2, fill + 2, Color(210,210,210,alpha));
		else
			render::rect(box.x - 5, y - 1 + h - fill, 2, fill + 2, { r, g, 0, alpha });

		// if hp is below max, draw a string.
		if (player->m_iHealth() <= 92) {
			render::esp_small.string(box.x - 5, y + (h - fill) - 5, { 255, 255, 255, low_alpha }, std::to_string(hp), render::ALIGN_CENTER);
		}
	}

	// draw flags.
	{
		AimPlayer* data = &g_aimbot.m_players[player->index() - 1];
		std::vector< std::pair< std::string, Color > > flags;

		auto items = enemy ? g_menu.main.players.flags_enemy.GetActiveIndices( ) : g_menu.main.players.flags_friendly.GetActiveIndices( );

		// NOTE FROM NITRO TO DEX -> stop removing my iterator loops, i do it so i dont have to check the size of the vector
		// with range loops u do that to do that.
		for( auto it = items.begin( ); it != items.end( ); ++it ) {

			if (g_csgo.m_resource != nullptr) {
				auto player_resource = *(g_csgo.m_resource);
				int ping = round(player_resource->get_ping(player->index()));
				Color ping_flag;

				if (ping >= 150) {
					if (ping <= 200)
						ping_flag = Color(155, 210, 100, low_alpha);
					else
						ping_flag = Color(255, 0, 0, low_alpha);

					flags.push_back({ std::to_string(ping) + "MS", ping_flag });
				}
			}

			// money.
			if( *it == 0 )
				if (dormant)
				flags.push_back( { tfm::format( XOR( "$%i" ), player->m_iAccount( ) ), { 210, 210, 210, low_alpha } } );
				else
				flags.push_back({ tfm::format(XOR("$%i"), player->m_iAccount()), { 110, 165, 23, 210 } });


			// armor.
			if (*it == 1) {
				if (player->m_ArmorValue() > 0) {
					if (player->m_bHasHelmet())
						if (dormant)
						flags.push_back({ XOR("HK"), {  210, 210, 210, low_alpha } });
						else
						flags.push_back({ XOR("HK"), {  255, 255, 255, low_alpha } });
					else
				if (dormant)
					flags.push_back({ XOR("K"), {  210, 210, 210, low_alpha } });
				else
					flags.push_back({ XOR("K"), {  255, 255, 255, low_alpha } });
				}
			}

			// scoped.
			if( *it == 2 && player->m_bIsScoped( ) )
				if (dormant)
				flags.push_back( { XOR( "ZOOM" ), { 210, 210, 210, low_alpha } } );
				else
				flags.push_back({ XOR("ZOOM"), {  60, 180, 225, low_alpha } });

			// flashed.
			if( *it == 3 && player->m_flFlashBangTime( ) > 0.f )
				if (dormant)
				flags.push_back( { XOR( "BLIND" ), { 210, 210, 210, low_alpha } } );
				else
				flags.push_back({ XOR("BLIND"), {  60, 180, 225, low_alpha } });

			// reload.
			if( *it == 4 ) {
				// get ptr to layer 1.
				C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

				// check if reload animation is going on.
				if( layer1->m_weight != 0.f && player->GetSequenceActivity( layer1->m_sequence ) == 967 /* ACT_CSGO_RELOAD */ )
					if (dormant)
					flags.push_back( { XOR( "RELOAD" ), { 210, 210, 210, low_alpha } } );
					else
					flags.push_back({ XOR("RELOAD"), {  60, 180, 225, low_alpha } });
			}

			// bomb.
			if( *it == 5 && player->HasC4( ) )
				if (dormant) 
				flags.push_back( { XOR( "BOMB" ), { 180, 180, 180, low_alpha } } );
				else
				flags.push_back({ XOR("BOMB"), { 255, 0, 0, low_alpha } });

			if (*it == 6 && enemy && data->m_records.size() > 0) {

				LagRecord* current = data->m_records.front().get();

				Color clr = Color(255, 255, 255, low_alpha);
				if (current->m_mode == Resolver::Modes::RESOLVE_WALK || current->m_mode == Resolver::Modes::RESOLVE_LBY_PRED) {
					clr = Color(155, 210, 100, low_alpha);
				}

				if (dormant)
					flags.push_back({ XOR("FAKE"), { 130,130,130, low_alpha } });
				else
					flags.push_back({ XOR("FAKE"), { clr } });
			}

			if (*it == 7) {
				auto m_weapon = g_cl.m_local->GetActiveWeapon();
				if (m_weapon && !m_weapon->IsKnife())
				{
					auto data = m_weapon->GetWpnData();

					if (data->m_damage >= (int)std::round(player->m_iHealth()))
						flags.push_back({ XOR("LETHAL"), { 255, 0, 0, low_alpha } });
				}
			}


			if (*it == 8 && data->m_records.size() >= 1 && data->m_shift) {	
				LagRecord* current = data->m_records.front().get();
				std::string text = "X";

				Color col = Color( 255, 255, 255, low_alpha );

				if (current->m_sim_time <= current->m_old_sim_time)
					col = Color( 220, 0, 0, low_alpha );

				if (dormant)
					flags.push_back({ text, { 130, 130, 130, low_alpha } });
				else 
					flags.push_back({ text, { 255, 255, 255, low_alpha } });
			}

			if (*it == 9 && data && data->m_records.size() > 0 && enemy) {

				if (!dormant && data->m_hit && g_cl.m_local->alive())
					flags.push_back({ "HIT", { 220, 220, 220, low_alpha} });

			}
		}

		if (g_menu.main.aimbot.debugaim.get()) {
			if (data && data->m_records.size() && enemy && g_menu.main.aimbot.correct.get()) {
				LagRecord* current = data->m_records.front().get();
				if (current->m_mode == Resolver::Modes::RESOLVE_LBY)
					flags.push_back({ XOR("LBY"), { 255,255,255, low_alpha } });
				else if (current->m_mode == Resolver::Modes::RESOLVE_STAND)
					flags.push_back({ XOR("STATIC"), { 255,255,255, low_alpha } });
				else if (current->m_mode == Resolver::Modes::RESOLVE_LBY_PRED)
					flags.push_back({ XOR("PREDICT"), { 255,255,255, low_alpha } });
				else if (current->m_mode == Resolver::Modes::RESOLVE_STOPPED_MOVING)
					flags.push_back({ XOR("STATIC"), { 255,255,255, low_alpha } });
				else if (current->m_mode == Resolver::Modes::RESOLVE_AIR)
					flags.push_back({ XOR("AIR"), { 255,255,255, low_alpha } });
				else if (current->m_mode == Resolver::Modes::RESOLVE_OVERRIDE)
					flags.push_back({ current->m_resolver_mode, { 255,255,255, low_alpha } });
				else
					flags.push_back({ XOR("MOVING"), { 255,255,255, low_alpha } });
			}
		}

		// iterate flags.
		for (size_t i{ }; i < flags.size(); ++i) {
			const auto& f = flags[i];
			int offset = i * (render::menu.m_size.m_height) + 1;
			render::esp_small.string(box.x - 1 + box.w + 3, box.y + 1 + offset - 2, f.second, f.first);
		}
	}

	// draw bottom bars.
	{
		int  offset1{ 0 };
		int  offset{ 0 };
		int  offset3{ 0 };;
		int  distance1337{ 0 };

		// draw lby update bar.
		if( enemy && g_menu.main.players.lby_update.get( ) ) {
			AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];

			// make sure everything is valid.
			if( data && data->m_moved &&data->m_records.size( ) ) {
				// grab lag record.
				LagRecord* current = data->m_records.front( ).get( );

				if( current ) {
					if( !( current->m_velocity.length_2d( ) > 0.1 && !current->m_fake_walk ) && data->m_body_idx <= 3 ) {
						// calculate box width
						float cycle = std::clamp<float>( data->m_body_timer - current->m_anim_time, 0.f, 1.125f );
						float width = ( box.w * cycle ) / 1.125f;

						if( width > 0.f ) {
							// draw.
							render::rect_filled( box.x - 1, box.y + box.h + 2, box.w + 2, 4, { 10, 10, 10, low_alpha } );

							Color clr = g_menu.main.players.lby_update_color.get( );
							clr.a( ) = alpha;
							render::rect( box.x + 1, box.y + box.h + 3, width, 2, clr );

							// move down the offset to make room for the next bar.
							offset += 5;
							offset3 += 1;
						}
					}
				}
			}
		}

		// draw weapon.
		if( ( enemy )) {
			Weapon* weapon = player->GetActiveWeapon( );
			if( weapon ) {
				WeaponInfo* data = weapon->GetWpnData( );
				if( data ) {
					int bar;
					float scale;

					// the maxclip1 in the weaponinfo
					int max = data->m_max_clip1;
					int current = weapon->m_iClip1( );

					C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

					// set reload state.
					bool reload = ( layer1->m_weight != 0.f ) && ( player->GetSequenceActivity( layer1->m_sequence ) == 967 );

					// ammo bar.
					if (max != -1 && g_menu.main.players.ammo.get()) {
						// check for reload.
						if (reload)
							scale = layer1->m_cycle;

						// not reloading.
						// make the division of 2 ints produce a float instead of another int.
						else
							scale = (float)current / max;

						// relative to bar.
						bar = (int)std::round((box.w - 2) * scale);

						// draw.
						render::rect_filled(box.x - 1, box.y + box.h + 2 + offset, box.w + 2, 4, { 0, 0, 0, low_alpha });

						Color clr = g_menu.main.players.ammo_color.get();
						clr.a() = alpha;
						if (dormant)
							render::rect(box.x, box.y + box.h + 3 + offset, bar + 2, 2, Color(210, 210, 210, alpha));
						else
							render::rect(box.x, box.y + box.h + 3 + offset, bar + 2, 2, clr);

						// less then a 5th of the bullets left.
						if (current > 0 && current <= int(std::round(max / 5)) && !reload)
							if (dormant)
								render::esp_small.string(box.x + bar, box.y + box.h + offset, { 255, 255, 255, low_alpha }, std::to_string(current), render::ALIGN_CENTER);
							else
								render::esp_small.string(box.x + bar, box.y + box.h + offset, { 255, 255, 255, 210 }, std::to_string(current), render::ALIGN_CENTER);

						offset += 6;
					}

					if (g_menu.main.players.distance.get()) {
						std::string distance;
						int dist = (((player->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;

						if (dist < 0)
							distance1337 = 0;

						if (dist > 0) {
							distance1337 = 9 + offset3;
							if (dist > 5) {
								while (!(dist % 5 == 0)) {
									dist = dist - 1;
								}

								if (dist % 5 == 0)
									distance = tfm::format(XOR("%i FT"), dist);
							}
							else
								distance = tfm::format(XOR("%i FT"), dist);

							if (dormant)
								render::esp_small.string(box.x + box.w / 2, box.y + box.h + offset + offset3, { 255, 255, 255, low_alpha }, distance, render::ALIGN_CENTER);
							else
								render::esp_small.string(box.x + box.w / 2, box.y + box.h + offset + offset3, { 255, 255, 255, 210 }, distance, render::ALIGN_CENTER);
						}
						offset += render::esp_small.m_size.m_height;
					}

					Color wpn_clr = g_menu.main.players.weaponcolor.get();

					// text.
					if (g_menu.main.players.weapontext.get()) {
						offset -= 9;
						// construct std::string instance of localized weapon name.

						std::string name{ weapon->GetLocalizedName() };

						// smallfonts needs upper case.
						std::transform( std::execution::par, name.begin(), name.end(), name.begin(), ::toupper);

						int move;
						if (!g_menu.main.players.distance.get())
							move = 9;
						else
							move = 0;

						if (dormant)
							render::esp_small.string(box.x + box.w / 2, box.y + box.h + move + offset + distance1337, { 255, 255, 255, low_alpha }, name, render::ALIGN_CENTER);
						else
							render::esp_small.string(box.x + box.w / 2, box.y + box.h + move + offset + distance1337, { wpn_clr.r(), wpn_clr.g(), wpn_clr.b(), 210 }, name, render::ALIGN_CENTER);
						
						offset += render::esp_small.m_size.m_height;
					}

					// icons.
					if (g_menu.main.players.weaponicon.get()) {
						offset -= 4;
						// icons are super fat..
						// move them back up.

						int move;
						if (!g_menu.main.players.distance.get())
							move = 9;
						else
							move = 0;

						if (dormant)
							render::icon.string(box.x + box.w / 2, box.y + box.h + move + offset - offset1 + distance1337 + 2, Color(210, 210, 210, low_alpha), GetWeaponIcon(weapon->m_iItemDefinitionIndex()), render::ALIGN_CENTER);
						else
							render::icon.string(box.x + box.w / 2, box.y + box.h + move + offset - offset1 + distance1337 + 2, Color(wpn_clr.r(), wpn_clr.g(), wpn_clr.b(), 210), GetWeaponIcon(weapon->m_iItemDefinitionIndex()), render::ALIGN_CENTER);
					
						offset += render::icon.m_size.m_height;
					}
				}
			}
		}
	}
}

void Visuals::DrawHitboxMatrix(LagRecord* record, Color col, float time, int mode) {
	if (!g_menu.main.aimbot.debugaim.get())
		return;

	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel();
	if (!model)
		return;

	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	set = hdr->GetHitboxSet(record->m_player->m_nHitboxSet());
	if (!set)
		return;

	switch (mode) {
	case 0:
		goto HITBOX_CAPSULE_MODE;
		break;
	case 1:
		goto SKELETON_MODE;
		break;
	case 2:
		goto HITBOX_ONLY;
		break;
		break;
	case 3:
		goto HITBOX_CAPSULE_MODE;
		break;
	default:
		return;
	}


HITBOX_CAPSULE_MODE: {
	Color color_capsules;
	Color color_bbox;
	for (int i{ }; i < set->m_hitboxes; ++i) {
		bbox = set->GetHitbox(i);
		if (!bbox)
			continue;

		color_capsules = mode == 3 ? Color(150, 150, 150, 100) : col;
		color_bbox = mode == 3 ? Color(150, 150, 150, 100) : col;

		if (mode == 3 && i == g_aimbot.m_hitbox)
			color_capsules = col;

		// bbox.
		if (bbox->m_radius <= 0.f) {
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms(record->m_bones[bbox->m_bone], rot_matrix, matrix);

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles(matrix, bbox_angle);

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin();

			// draw box.
			g_csgo.m_debug_overlay->AddBoxOverlay(origin, bbox->m_mins, bbox->m_maxs, bbox_angle, color_bbox.r(), color_bbox.g(), color_bbox.b(), 0, time);
		}

		// capsule.
		else {
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix(bbox->m_angle, matrix);

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms(record->m_bones[bbox->m_bone], matrix, matrix);

			// get world positions from new matrix.
			math::VectorTransform(bbox->m_mins, matrix, mins);
			math::VectorTransform(bbox->m_maxs, matrix, maxs);

			g_csgo.m_debug_overlay->AddCapsuleOverlay(mins, maxs, bbox->m_radius, color_capsules.r(), color_capsules.g(), color_capsules.b(), color_capsules.a(), time, 0, 1);
		}
	}
	return; // return at the end of this created statement
	}

SKELETON_MODE: {
vec3_t        bone_pos, parent_pos;
for (int i{ }; i < hdr->m_num_bones; ++i) {
	// get bone.
	auto bone = hdr->GetBone(i);
	if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
		continue;

	// get parent bone.
	auto parent = bone->m_parent;
	if (parent == -1)
		continue;

	// resolve main bone and parent bone positions.
	record->m_bones->get_bone(bone_pos, i);
	record->m_bones->get_bone(parent_pos, parent);

	g_csgo.m_debug_overlay->AddLineOverlay(bone_pos, parent_pos, col.r(), col.g(), col.b(), true, time);
}
return; // return at the end of this created statement
}

HITBOX_ONLY: {

matrix3x4_t matrix;
int hitbox = g_aimbot.m_target ? g_aimbot.m_hitbox : 0;
bbox = set->GetHitbox(hitbox);
if (!bbox)
return;

g_csgo.AngleMatrix(bbox->m_angle, matrix);

// apply the rotation matrix to the entity output space (world).
math::ConcatTransforms(record->m_bones[bbox->m_bone], matrix, matrix);

// get world positions from new matrix.
math::VectorTransform(bbox->m_mins, matrix, mins);
math::VectorTransform(bbox->m_maxs, matrix, maxs);

g_csgo.m_debug_overlay->AddCapsuleOverlay(mins, maxs, bbox->m_radius, col.r(), col.g(), col.b(), col.a(), time, 0, 1);
return;
}
}

void Visuals::DrawPlantedC4() {
	std::string time_str, damage_str, new_site;
	Color       damage_color;
	Color		timer_color;
	Color		defuse_color;

	if (!g_csgo.m_engine->IsConnected() || !g_csgo.m_engine->IsInGame())
		return;

	// bomb not currently active, do nothing.
	if (!m_c4_planted || !m_planted_c4)
		return;

	if (!g_menu.main.visuals.planted_c4.get())
		return;

	// setup variables

	auto time_to_explode = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;
	auto defused_at = m_planted_c4->m_flDefuseCountDown();
	auto defuse_lenght = m_planted_c4->m_flDefuseLength();
	auto site = m_planted_c4->m_nBombSite();
	auto defuser = m_planted_c4->m_hBombDefuser();

	if (time_to_explode <= 0.1f)
		return;

	int width, height;
	g_csgo.m_engine->GetScreenSize(width, height);

	static auto c4_timer = g_csgo.m_cvar->FindVar(HASH("mp_c4timer"))->GetInt();

	auto value = (time_to_explode * height) / c4_timer;

	if (value <= 0.1f)
		return;

	const auto percentage = 1.0f;
	auto dmg = 500.f;
	auto radius = dmg * 3.5f;
	auto distance = (m_planted_c4->EyePos() - g_cl.m_local->EyePos()).length();
	auto sigma = radius / 3.0f;
	auto fall_off = exp(-distance * distance / (2.0f * sigma * sigma));
	int final_dmg = dmg * fall_off * percentage;

	auto scale_damage = [&](float dmg, int armor_value) {
		float ratio = 0.5f, bonus = 0.5f;
		if (armor_value > 0) {
			float _new = dmg * ratio;
			float armor = (dmg - _new) * bonus;

			if (armor > static_cast<float>(armor_value)) {
				armor = static_cast<float>(armor_value) * (1.f / bonus);
				_new = dmg - armor;
			}

			dmg = _new;
		}
		return std::max(0, (int)std::floor(dmg));
		};

	final_dmg = scale_damage(final_dmg, g_cl.m_local->m_ArmorValue());

	if (site == 0)
		new_site = 'A';
	else
		new_site = 'B';

	auto ss = g_visuals.m_last_bombsite.c_str();

	time_str = tfm::format(XOR("%i - %.1fs"), new_site, time_to_explode);
	damage_str = tfm::format(XOR("-%i HP"), final_dmg);

	// get timer color.
	if (time_to_explode > 10.f) {
		timer_color = Color(150, 200, 60);
	}
	else if (time_to_explode < 10.f && time_to_explode > 5.0f) {
		timer_color = Color{ 255, 255, 140 };
	}
	else if (time_to_explode < 5.f) {
		timer_color = Color{ 255, 12, 0 };
	}

	// get damage color.
	damage_color = (final_dmg < g_cl.m_local->m_iHealth()) ? Color{ 255, 255, 140 } : Color{ 255, 12, 0 };


	// draw defuse overlay
	auto defuse_percentage = (g_csgo.m_globals->m_curtime - (defused_at - defuse_lenght)) / defuse_lenght;
	float value_defuse = 0.f;
	if (defuse_lenght == 10.f)
		value_defuse = ((defuse_percentage * 10.f) * height) / defuse_lenght;
	else
		value_defuse = ((defuse_percentage * 5.f) * height) / defuse_lenght;


	bool has_time = time_to_explode > (defused_at - g_csgo.m_globals->m_curtime);

	if (has_time)
		defuse_color = Color(150, 255, 60, 120);
	else
		defuse_color = Color(255, 12, 0, 120);

	if (defuser != (-1)) {
		if (defuse_percentage >= 0.f && defuse_percentage <= 1.f) {
			render::rect_filled(0, 0, 22, height, Color(0, 0, 0, 120));
			render::rect_filled(1, 0, 20, value_defuse, defuse_color);
		}
	}

	// draw overlay
	if (time_to_explode > 0.f)
		render::indicator.string(2, 5, timer_color, time_str, render::ALIGN_LEFT);

	if (g_cl.m_local->alive())
		if (final_dmg >= g_cl.m_local->m_iHealth())
			render::indicator.string(2, render::indicator.m_size.m_height, damage_color, "FATAL", render::ALIGN_LEFT);
		else if (final_dmg > 0)
			render::indicator.string(2, render::indicator.m_size.m_height, damage_color, damage_str, render::ALIGN_LEFT);
}


bool Visuals::GetPlayerBoxRect(Player* player, Rect& box) {
	vec3_t pos{ player->GetAbsOrigin() };
	vec3_t top = pos + vec3_t(0, 0, player->GetCollideable()->OBBMaxs().z);

	vec2_t pos_screen, top_screen;

	if (!render::WorldToScreen(pos, pos_screen) ||
		!render::WorldToScreen(top, top_screen))
		return false;

	box.x = int(top_screen.x - ((pos_screen.y - top_screen.y) / 2) / 2);
	box.y = int(top_screen.y);

	box.w = int(((pos_screen.y - top_screen.y)) / 2);
	box.h = int((pos_screen.y - top_screen.y));

	const bool out_of_fov = pos_screen.x + box.w + 20 < 0 || pos_screen.x - box.w - 20 > g_cl.m_width || pos_screen.y + 20 < 0 || pos_screen.y - box.h - 20 > g_cl.m_height;

	return !out_of_fov;
}

void Visuals::AddMatrix(Player* player, matrix3x4_t* bones) {
	auto& hit = m_hit_matrix.emplace_back();

	std::memcpy(hit.pBoneToWorld, bones, player->bone_cache().count() * sizeof(matrix3x4_t));

	float time = g_menu.main.players.chams_shot_fadetime.get();

	hit.time = g_csgo.m_globals->m_realtime + time;

	static int m_nSkin = 0xA1C;
	static int m_nBody = 0xA20;

	hit.info.m_origin = player->GetAbsOrigin();
	hit.info.m_angles = player->GetAbsAngles();

	auto renderable = player->renderable();
	if (!renderable)
		return;

	auto model = player->GetModel();
	if (!model)
		return;

	auto hdr = *(studiohdr_t**)(player->GetModelPtr());
	if (!hdr)
		return;

	hit.state.m_pStudioHdr = hdr;
	hit.state.m_pStudioHWData = g_csgo.m_model_cache->GetHardwareData(model->m_studio);
	hit.state.m_pRenderable = renderable;
	hit.state.m_drawFlags = 0;

	hit.info.m_renderable = renderable;
	hit.info.m_model = model;
	hit.info.m_lighting_offset = nullptr;
	hit.info.m_lighting_origin = nullptr;
	hit.info.m_hitboxset = player->m_nHitboxSet();
	hit.info.m_skin = (int)(uintptr_t(player) + m_nSkin);
	hit.info.m_body = (int)(uintptr_t(player) + m_nBody);
	hit.info.m_index = player->index();
	hit.info.m_instance = util::get_method<ModelInstanceHandle_t(__thiscall*)(void*) >(renderable, 30u)(renderable);
	hit.info.m_flags = 0x1;

	hit.info.m_model_to_world = &hit.model_to_world;
	hit.state.m_pModelToWorld = &hit.model_to_world;

	math::angle_matrix(hit.info.m_angles, hit.info.m_origin, hit.model_to_world);
}

void Visuals::override_material(bool ignoreZ, bool use_env, Color& color, IMaterial* material) {
	material->SetFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);
	material->IncrementReferenceCount();

	bool found;
	auto var = material->FindVar("$envmaptint", &found);

	if (found)
		var->set_vec_value(color.r(), color.g(), color.b());

	g_csgo.m_studio_render->ForcedMaterialOverride(material);
}

void Visuals::on_post_screen_effects() {
	if (!g_cl.m_processing)
		return;

	const auto local = g_cl.m_local;
	if (!local || !g_menu.main.players.chams_shot.get() || !g_csgo.m_engine->IsInGame())
		m_hit_matrix.clear();

	if (m_hit_matrix.empty() || !g_csgo.m_model_render)
		return;

	auto ctx = g_csgo.m_material_system->get_render_context();
	if (!ctx)
		return;

	auto it = m_hit_matrix.begin();

	while (it != m_hit_matrix.end()) {
		if (!it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			!it->info.m_renderable || !it->info.m_model_to_world || !it->info.m_model) {
			++it;
			continue;
		}

		auto alpha = 1.0f;
		auto delta = g_csgo.m_globals->m_realtime - it->time;

		if (delta > 0.0f) {
			alpha -= delta;

			if (delta > 1.0f) {
				it = m_hit_matrix.erase(it);
				continue;
			}
		}

		auto alpha_color = (float)g_menu.main.players.chams_shot_blend.get() / 255.f;

		Color ghost_color = g_menu.main.players.chams_shot_col.get();

		g_csgo.m_render_view->SetBlend(alpha_color * alpha);
		g_chams.SetupMaterial(g_chams.m_materials[g_menu.main.players.chams_shot_mat.get()], ghost_color, true);
		g_csgo.m_model_render->DrawModelExecute(ctx, it->state, it->info, it->pBoneToWorld);
		g_csgo.m_model_render->ForceMat(nullptr);

		++it;
	}
}

Color determine_clr(Player* player, Color clr, float alpha)
{
	if (player->dormant())
		return Color(clr.r(), clr.g(), clr.b(), alpha);
	else {
		return clr.malpha(alpha);
	}
}

void Visuals::DrawSkeleton(Player* player, int opacity) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	BoneArray     matrix[128];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	model = player->GetModel();
	if (!model)
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	// get bone matrix.
	if (!player->SetupBones(matrix, 128, BONE_USED_BY_ANYTHING, g_csgo.m_globals->m_curtime))
		return;

	for (int i{ }; i < hdr->m_num_bones; ++i) {
		// get bone.
		bone = hdr->GetBone(i);
		if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if (parent == -1)
			continue;

		// resolve main bone and parent bone positions.
		matrix->get_bone(bone_pos, i);
		matrix->get_bone(parent_pos, parent);

		Color clr = player->enemy(g_cl.m_local) ? g_menu.main.players.skeleton_enemy.get() : g_menu.main.players.skeleton_enemy.get();

		// world to screen both the bone parent bone then draw.
		if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen))
			render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, determine_clr(player, clr, opacity).malpha(g_menu.main.players.skeleton_enemy.get().a()));
	}
}

bool is_grenade(const int id)
{
	return id == 9 || id == 98 || id == 134;
}

bool isc4(const int id)
{
	return id == 29 || id == 108;
}

void Visuals::RenderGlow() {
	Color   color;
	Player* player;
	if (!g_cl.m_local)
		return;

	if (!g_csgo.m_glow->m_object_definitions.Count())
		return;

	float blend = g_menu.main.players.glow_blend.get() / 100.f;

	for (int i{ }; i < g_csgo.m_glow->m_object_definitions.Count(); ++i) {
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[i];
		if (obj->IsUnused() || !obj->m_entity)
			continue;

		const auto classid = obj->m_entity->GetClientClass()->m_ClassID;

		if (is_grenade(classid) && g_menu.main.visuals.proj.get()) {
			color = g_menu.main.visuals.proj_color.get();

			float blend = g_menu.main.visuals.proj_col_slider.get() / 100.f;

			obj->m_render_occluded = true;
			obj->m_render_unoccluded = false;
			obj->m_render_full_bloom = false;
			obj->m_color = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
			obj->m_alpha = blend;
			obj->m_bloom_amount = 1.f;
		}

		if (isc4(classid) && g_menu.main.visuals.planted_c4.get()) {
			color = g_menu.main.visuals.c4_col.get();
			float blend = g_menu.main.visuals.c4_blend.get() / 100.f;
			obj->m_render_occluded = true;
			obj->m_render_unoccluded = false;
			obj->m_render_full_bloom = false;
			obj->m_color = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
			obj->m_alpha = blend;
			obj->m_bloom_amount = 1.f;
		}

		if (!isc4(classid) && obj->m_entity->IsBaseCombatWeapon() && g_menu.main.visuals.dropped_weapons.get(3)) {
			color = g_menu.main.visuals.item_color.get();
			float blend = g_menu.main.visuals.glow_color_alpha.get() / 100.f;

			obj->m_render_occluded = true;
			obj->m_render_unoccluded = false;
			obj->m_render_full_bloom = false;
			obj->m_color = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
			obj->m_alpha = blend;
			obj->m_bloom_amount = 1.f;
		}

		if (obj->m_entity->IsPlayer()) {

			// skip non-players.
			if (!obj->m_entity || !obj->m_entity->IsPlayer())
				continue;

			// get player ptr.
			player = obj->m_entity->as< Player* >();

			if (player->m_bIsLocalPlayer())
				continue;

			// get reference to array variable.
			float& opacity = m_opacities[player->index()];

			bool enemy = player->enemy(g_cl.m_local);

			if (enemy && !g_menu.main.players.glow.get())
				continue;

			if (!enemy && !g_menu.main.players.teammates.get())
				continue;

			// enemy color
			if (enemy)
				color = g_menu.main.players.glow_enemy.get();

			// friendly color
			else
				color = g_menu.main.players.glow_enemy.get();

			obj->m_render_occluded = true;
			obj->m_render_unoccluded = false;
			obj->m_render_full_bloom = false;
			obj->m_color = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
			obj->m_alpha = opacity * blend;
		}
	}
}

void Visuals::DrawBeams() {
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beamInfo;
	Beam_t* beam;

	if (!g_cl.m_local)
		return;

	if (!g_menu.main.visuals.impact_beams2.get())
		return;

	auto vis_impacts = &g_shots.m_vis_impacts;

	// the local player is dead, clear impacts.
	if (!g_cl.m_processing) {
		if (!vis_impacts->empty())
			vis_impacts->clear();
	}

	else {
		impact_count = vis_impacts->size();
		if (!impact_count)
			return;

		curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

		for (size_t i{ impact_count }; i-- > 0; ) {
			auto impact = &vis_impacts->operator[ ](i);
			if (!impact)
				continue;

			// impact is too old, erase it.
			if (std::abs(curtime - game::TICKS_TO_TIME(impact->m_tickbase)) > g_menu.main.visuals.impact_beams_time.get()) {
				vis_impacts->erase(vis_impacts->begin() + i);

				continue;
			}

			// already rendering this impact, skip over it.
			if (impact->m_ignore)
				continue;

			// is this the final impact?
			// last impact in the vector, it's the final impact.
			if (i == (impact_count - 1))
				is_final_impact = true;

			// the current impact's tickbase is different than the next, it's the final impact.
			else if ((i + 1) < impact_count && impact->m_tickbase != vis_impacts->operator[ ](i + 1).m_tickbase)
				is_final_impact = true;

			else
				is_final_impact = false;

			// is this the final impact?
			// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

			if (is_final_impact) {
				// calculate start and end position for beam.
				start = impact->m_shoot_pos;

				dir = (impact->m_impact_pos - start).normalized();
				dist = (impact->m_impact_pos - start).length();

				end = start + (dir * dist);

				// setup beam info.
				// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
				if (g_menu.main.visuals.local_material_type.get() == 0) {
					beamInfo.m_vecStart = start;
					beamInfo.m_vecEnd = end;
					beamInfo.m_nType = 0;
					beamInfo.m_pszModelName = XOR("sprites/purplelaser1.vmt");
					beamInfo.m_nModelIndex = g_csgo.m_model_info->GetModelIndex(XOR("sprites/purplelaser1.vmt"));
					beamInfo.m_flHaloScale = 0.0f;
					beamInfo.m_flLife = g_menu.main.visuals.impact_beams_time.get();
					beamInfo.m_flWidth = 4.0f;
					beamInfo.m_flEndWidth = 4.0f;
					beamInfo.m_flFadeLength = 0.0f;
					beamInfo.m_flAmplitude = 2.0f;
					beamInfo.m_flBrightness = g_menu.main.visuals.impact_beams_color.get().a();
					beamInfo.m_flSpeed = 0.2f;
					beamInfo.m_nStartFrame = 0;
					beamInfo.m_flFrameRate = 0.f;
					beamInfo.m_flRed = g_menu.main.visuals.impact_beams_color.get().r();
					beamInfo.m_flGreen = g_menu.main.visuals.impact_beams_color.get().g();
					beamInfo.m_flBlue = g_menu.main.visuals.impact_beams_color.get().b();
					beamInfo.m_nSegments = 2;
					beamInfo.m_bRenderable = true;
					beamInfo.m_nFlags = 0x100 | 0x200 | 0x8000;
				}
				else if (g_menu.main.visuals.local_material_type.get() == 1) {
					beamInfo.m_vecStart = start;
					beamInfo.m_vecEnd = end;
					beamInfo.m_nType = 0;
					beamInfo.m_pszModelName = XOR("sprites/physbeam.vmt");
					beamInfo.m_nModelIndex = g_csgo.m_model_info->GetModelIndex(XOR("sprites/physbeam.vmt"));
					beamInfo.m_flHaloScale = 0.0f;
					beamInfo.m_flLife = g_menu.main.visuals.impact_beams_time.get();
					beamInfo.m_flWidth = 1.0f;
					beamInfo.m_flEndWidth = 1.f;
					beamInfo.m_flFadeLength = 0.0f;
					beamInfo.m_flAmplitude = 0.f;
					beamInfo.m_flBrightness = g_menu.main.visuals.impact_beams_color.get().a();
					beamInfo.m_flSpeed = g_menu.main.visuals.impact_beams_speed.get();
					beamInfo.m_nStartFrame = 0;
					beamInfo.m_flFrameRate = 0.f;
					beamInfo.m_flRed = g_menu.main.visuals.impact_beams_color.get().r();
					beamInfo.m_flGreen = g_menu.main.visuals.impact_beams_color.get().g();
					beamInfo.m_flBlue = g_menu.main.visuals.impact_beams_color.get().b();
					beamInfo.m_nSegments = 2;
					beamInfo.m_bRenderable = true;
				}

				// attempt to render the beam.
				beam = game::CreateGenericBeam(beamInfo);
				if (beam) {
					g_csgo.m_beams->DrawBeam(beam);

					// we only want to render a beam for this impact once.
					impact->m_ignore = true;
				}
			}
		}
	}
}

void Visuals::ImpactData()
{
	if (!g_cl.m_processing) return;

	if (!g_menu.main.visuals.bullet_impacts.get()) return;

	//call this in fsn or whatever
	static auto last_count = 0;
	auto& client_impact_list = *(CUtlVector< client_hit_verify_t >*)((uintptr_t)g_cl.m_local + 0xBA84);

	for (auto i = client_impact_list.Count(); i > last_count; i--) {
		g_csgo.m_debug_overlay->AddBoxOverlay(client_impact_list[i - 1].pos, vec3_t(-2, -2, -2), vec3_t(2, 2, 2), ang_t(0, 0, 0), 255, 0, 0, 127, g_menu.main.visuals.impact_duration.get());
	}

	if (client_impact_list.Count() != last_count)
		last_count = client_impact_list.Count();
}

void Visuals::Add(BulletImpactInfo beamEffect) {
	bulletImpactInfo.push_back(beamEffect);
}	

void Visuals::DrawHistorySkeleton(Player* player, int opacity) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	AimPlayer* data;
	LagRecord* record;
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	if (!g_menu.main.misc.fake_latency.get())
		return;

	// get player's model.
	model = player->GetModel();
	if (!model)
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	data = &g_aimbot.m_players[player->index() - 1];
	if (!data)
		return;

	record = g_resolver.FindLastRecord(data);
	if (!record)
		return;

	for (int i{ }; i < hdr->m_num_bones; ++i) {
		// get bone.
		bone = hdr->GetBone(i);
		if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if (parent == -1)
			continue;

		// resolve main bone and parent bone positions.
		record->m_bones->get_bone(bone_pos, i);
		record->m_bones->get_bone(parent_pos, parent);

		Color clr = player->enemy(g_cl.m_local) ? g_menu.main.players.skeleton_enemy.get() : g_menu.main.players.skeleton_enemy.get();

		// world to screen both the bone parent bone then draw.
		if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen))
			render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, determine_clr(player, clr, opacity).malpha(g_menu.main.players.skeleton_enemy.get().a()));
	}
}

void Visuals::IndicateAngles() {
	if (!g_csgo.m_engine->IsInGame() && !g_csgo.m_engine->IsConnected())
		return;

	if (!g_menu.main.visuals.antiaim_arrows.get())
		return;

	if (!g_csgo.m_input->CAM_IsThirdPerson())
		return;

	if (!g_cl.m_local || g_cl.m_local->m_iHealth() < 1)
		return;

	const auto& pos = g_cl.m_local->GetRenderOrigin();
	vec2_t tmp;

	if (render::WorldToScreen(pos, tmp))
	{
		vec2_t draw_tmp;
		const vec3_t real_pos(50.f * std::cos(math::deg_to_rad(g_cl.m_radar.y)) + pos.x, 50.f * sin(math::deg_to_rad(g_cl.m_radar.y)) + pos.y, pos.z);

		if (render::WorldToScreen(real_pos, draw_tmp))
		{
			render::line(tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, colors::transparent_red );
			render::esp_small.string(draw_tmp.x, draw_tmp.y, { 222, 44, 44 }, "FAKE", render::ALIGN_LEFT);
		}

		if (g_menu.main.antiaim.fake_yaw.get())
		{
			const vec3_t fake_pos(50.f * cos(math::deg_to_rad(g_cl.m_angle.y)) + pos.x, 50.f * sin(math::deg_to_rad(g_cl.m_angle.y)) + pos.y, pos.z);

			if (render::WorldToScreen(fake_pos, draw_tmp))
			{
				render::line(tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { colors::transparent_green });
				render::esp_small.string(draw_tmp.x, draw_tmp.y, { 55, 215, 90, 255 }, "REAL", render::ALIGN_LEFT);
			}
		}

		if (g_menu.main.antiaim.body_yaw.get() > 0) // why do this == i ????
		{
			float lby = g_cl.m_local->m_flLowerBodyYawTarget();
			const vec3_t lby_pos(50.f * cos(math::deg_to_rad(lby)) + pos.x,
				50.f * sin(math::deg_to_rad(lby)) + pos.y, pos.z);

			if (render::WorldToScreen(lby_pos, draw_tmp))
			{
				render::line(tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { colors::light_blue });
				render::esp_small.string(draw_tmp.x, draw_tmp.y, { 72, 100, 232, 255 }, "LBY", render::ALIGN_LEFT);
			}
		}
	}
}