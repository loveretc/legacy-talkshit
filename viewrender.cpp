#include "includes.h"

void Hooks::OnRenderStart() {
	// call og.
	g_hooks.m_view_render.GetOldMethod< OnRenderStart_t >(CViewRender::ONRENDERSTART)(this);

	if (g_cl.m_local && g_cl.m_local->alive() && g_cl.m_weapon) {

		// zoom FOV
		if (g_cl.m_local->m_bIsScoped()) {

			// variables
			float zoomFactor = (100.f - g_menu.main.misc.zoom_amt.get()) / 100.f;
			float zoom1 = 0.49f + zoomFactor * (1.0f - 0.49f);
			float zoom2 = 0.19f + zoomFactor * (1.0f - 0.49f);
			static auto zoom_sensitivity_ratio_mouse = g_csgo.m_cvar->FindVar(HASH("zoom_sensitivity_ratio_mouse"));

			// fix sensitivity when using 3rd person
			if (g_csgo.m_input->CAM_IsThirdPerson()) {
				if (g_cl.m_local->GetActiveWeapon()->m_zoomLevel() == 1)
					zoom_sensitivity_ratio_mouse->SetValue(2.f);
				else if (g_cl.m_local->GetActiveWeapon()->m_zoomLevel() == 2)
					zoom_sensitivity_ratio_mouse->SetValue(6.f);
				else
					zoom_sensitivity_ratio_mouse->SetValue(1.f);
			}
			else
				zoom_sensitivity_ratio_mouse->SetValue(1.f);

			// apply
			g_csgo.m_view_render->m_view.m_fov = std::clamp(g_menu.main.misc.fov_amt.get() - (g_menu.main.misc.fov_amt.get() * zoomFactor), 20.f, 150.f); // 86.7 -- 
		}
		else g_csgo.m_view_render->m_view.m_fov = g_menu.main.misc.fov_amt.get();
	}

	g_csgo.m_view_render->m_view.m_viewmodel_fov = g_menu.main.misc.viewmodel_amt.get();
}

void Hooks::RenderView(const CViewSetup& view, const CViewSetup& hud_view, int clear_flags, int what_to_draw) {
	// ...

	g_hooks.m_view_render.GetOldMethod< RenderView_t >(CViewRender::RENDERVIEW)(this, view, hud_view, clear_flags, what_to_draw);
}

void Hooks::Render2DEffectsPostHUD(const CViewSetup& setup) {
	if (!g_menu.main.visuals.removals.get(3))
		g_hooks.m_view_render.GetOldMethod< Render2DEffectsPostHUD_t >(CViewRender::RENDER2DEFFECTSPOSTHUD)(this, setup);
}

void Hooks::RenderSmokeOverlay(bool unk) {
	// do not render smoke overlay.
	if (!g_menu.main.visuals.removals.get(1))
		g_hooks.m_view_render.GetOldMethod< RenderSmokeOverlay_t >(CViewRender::RENDERSMOKEOVERLAY)(this, unk);
}
