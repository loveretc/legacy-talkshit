#include "includes.h"

Unloader g_unloader{ };
HMODULE  g_module{ };

void Unloader::RememberConvar(ConVar* cvar) {
	if (!cvar || m_done)
		return;

	// already have the original.
	for (const auto& c : m_convars) {
		if (c.m_cvar == cvar)
			return;
	}

	const char* value = cvar->GetString();
	if (!value)
		return;

	m_convars.push_back({ cvar, std::string(value) });
}

bool Unloader::RestoreGameState() {

	// close the menu and hand input back. if we unloaded with a text field focused the
	// game would never get its input system re-enabled.
	g_gui.m_open = false;

	if (g_csgo.m_input_system)
		g_csgo.m_input_system->EnableInput(true);

	// put the camera back in first person. nothing left after us knows how to get out
	// of thirdperson, the user would be stuck looking at his own back.
	g_visuals.m_thirdperson = false;

	if (g_csgo.m_input && g_csgo.m_input->CAM_IsThirdPerson()) {
		g_csgo.m_input->CAM_ToFirstPerson();
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// stop forcing a material on the model renderer, chams could have been mid frame.
	if (g_csgo.m_studio_render)
		g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);

	// undo nightmode / transparent walls.
	if (g_csgo.m_material_system)
		Visuals::ResetWorldModulation();

	// give back the material references we took.
	g_chams.shutdown();

	// hand every convar we touched back to whatever it was.
	for (const auto& c : m_convars) {
		if (c.m_cvar)
			c.m_cvar->SetValue(c.m_value.data());
	}

	m_convars.clear();

	// hand the players back to the game.
	if (g_csgo.m_engine && g_csgo.m_engine->IsInGame() && g_csgo.m_entlist) {
		for (int i{ 1 }; i <= 64; ++i) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

			if (!player || !player->IsPlayer())
				continue;

			// we drive their animations ourselves and keep interpolation off,
			// both of which the game has to own again.
			player->m_bClientSideAnimation() = true;
			player->m_fEffects() &= ~EF_NOINTERP;
			player->m_iEFlags() &= ~(EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY);

			// our historical matrices are still sitting in the bone cache.
			player->InvalidateBoneCache();
		}

		// ask the server for a fresh copy of the world. this is what heals everything
		// we edited at the netvar level ( skins, models, angles ) without us having to
		// remember each one.
		if (g_csgo.cl_fullupdate && g_csgo.cl_fullupdate->m_callback)
			g_csgo.cl_fullupdate->m_callback();
	}

	// nobody is going to consume these anymore.
	std::memset(g_hooks.m_bUpdatingCSA, 0, sizeof(g_hooks.m_bUpdatingCSA));
	g_hooks.m_bUpdatingCSALP = false;

	return true;
}

void Unloader::RestoreHooks() {

	// game events; the manager holds a pointer to our listener.
	g_listener.unregister_events();

	// netvar proxies.
	g_netvars.RestoreProxy(HASH("DT_CSPlayer"), HASH("m_flLowerBodyYawTarget"), g_hooks.m_Body_original);
	g_netvars.RestoreProxy(HASH("DT_CSRagdoll"), HASH("m_vecForce"), g_hooks.m_Force_original);

	// the byte patch we put into the engines choked command clamp.
	g_inputpred.Restore();

	// detours. MinHook freezes every thread while it puts the original bytes back.
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();

	// vmts. every one of these points a class at a table we allocated inside this
	// module, so they all have to go back before we are free to unmap.
	g_hooks.m_panel.reset();
	g_hooks.m_client_mode.reset();
	g_hooks.m_client.reset();
	g_hooks.m_client_state.reset();
	g_hooks.m_engine.reset();
	g_hooks.m_engine_sound.reset();
	g_hooks.m_prediction.reset();
	g_hooks.m_surface.reset();
	g_hooks.m_render.reset();
	g_hooks.m_render_view.reset();
	g_hooks.m_model_render.reset();
	g_hooks.m_shadow_mgr.reset();
	g_hooks.m_view_render.reset();
	g_hooks.m_match_framework.reset();
	g_hooks.m_material_system.reset();
	g_hooks.m_fire_bullets.reset();
	g_hooks.m_net_show_fragments.reset();
	g_hooks.m_debug_spread.reset();

	// the net channel is the one interface here the game throws away and rebuilds. if
	// the one we hooked is gone, restoring its vtable pointer would corrupt whatever
	// took over that memory, and that kind of damage only shows up much later - which
	// is exactly what a crash on the next injection looks like.
	if (g_csgo.m_engine && g_hooks.m_net_channel.base().as< void* >() == g_csgo.m_engine->GetNetChannelInfo())
		g_hooks.m_net_channel.reset();
	else
		g_hooks.m_net_channel.abandon();

	// private fonts. these were built from a byte array that lives in this module, so
	// leaving them registered hands the process a font backed by memory that is about
	// to be gone - and the next injection would look that same font up by name.
	if (g_hooks.m_font_undefeated) {
		RemoveFontMemResourceEx(g_hooks.m_font_undefeated);
		g_hooks.m_font_undefeated = nullptr;
	}

	if (g_hooks.m_font_bold_verdana) {
		RemoveFontMemResourceEx(g_hooks.m_font_bold_verdana);
		g_hooks.m_font_bold_verdana = nullptr;
	}

	// window procedure.
	// note; if something else hooked the window after we did, this puts its procedure
	//       out of the chain. nothing we can do about that from here.
	if (g_hooks.m_old_wndproc && g_csgo.m_game && g_csgo.m_game->m_hWindow) {
		g_winapi.SetWindowLongA(g_csgo.m_game->m_hWindow, GWL_WNDPROC, util::force_cast< LONG >(g_hooks.m_old_wndproc));
		g_hooks.m_old_wndproc = nullptr;
	}
}

void Unloader::Think() {

	if (!m_pending || m_done)
		return;

	// one shot, whatever happens below.
	m_pending = false;

	// we never finished starting up, there is nothing to take apart.
	if (!g_csgo.m_done)
		return;

	// the entity listener is the one thing we cannot leave behind: the game keeps a raw
	// pointer to an object whose vtable lives in this module, so the next entity that
	// spawns after we unmap would call into freed memory.
	// if we can't get ourselves out of that list, unloading is a guaranteed crash and
	// staying loaded is not, so we stay.
	if (!g_custom_entity_listener.remove()) {
		g_notify.add(XOR("unload aborted: could not unregister the entity listener\n"), Color(255, 55, 55, 255));
		return;
	}

	// tell the user now, in a moment we can't draw anymore.
	g_cl.print(XOR("unloading...\n"));

	RestoreGameState();
	RestoreHooks();

	m_done = true;

	// note; this one goes to the game console, which still works with our hooks gone.
	//       if you ever see 'unloading...' without this line following it, the unload
	//       died somewhere in between and that is where to look.
	g_cl.print(XOR("detached, freeing module\n"));

	// no module handle means we were mapped by something the loader doesn't know about.
	// everything is detached at this point, we just can't free the pages.
	if (!g_module)
		return;

	// from here on nothing can enter our code through a hook, but a thread could still
	// be walking out of one, so the actual unmap happens from a thread that waits first.
	HANDLE thread = CreateThread(nullptr, 0, UnloadThread, g_module, 0, nullptr);
	if (thread)
		CloseHandle(thread);
}

ulong_t __stdcall Unloader::UnloadThread(void* module) {

	// give every game thread the time to walk out of whatever hook of ours it was in
	// when we pulled them. the render thread is the one that matters here, it can be a
	// frame behind the main thread.
	// note; there is no way to be certain, but nothing can enter our code anymore, so
	//       this only has to outlast the calls that were already in flight.
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// frees the module and ends this thread without ever returning into our code.
	FreeLibraryAndExitThread(static_cast< HMODULE >(module), 0);

	return 0;
}
