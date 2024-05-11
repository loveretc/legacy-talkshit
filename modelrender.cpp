#include "includes.h"

inline bool isWeapon(std::string& modelName) {
	if ((modelName.find("v_") != std::string::npos || modelName.find("uid") != std::string::npos || modelName.find("stattrack") != std::string::npos) && modelName.find("arms") == std::string::npos) {
		return true;
	}

	return false;
}

void Hooks::DrawModelExecute(uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone) {

	if (!info.m_model)
		return g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);

	if (isWeapon(std::string(info.m_model->m_name)) && g_menu.main.players.weapon_chams.get()) {
		auto alpha2 = (float)g_menu.main.players.weapon_chams_blend2.get() / 255.f;
		auto alpha1 = (float)g_menu.main.players.weapon_chams_blend.get() / 255.f;

		

		g_chams.SetAlpha(alpha2);
		g_chams.SetupMaterial(g_chams.m_materials[1], g_menu.main.players.weapon_chams_col2.get(), false);
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);

		g_chams.SetAlpha(alpha1);
		g_chams.SetupMaterial(g_chams.m_materials[g_menu.main.players.weapon_chams_mat.get()], g_menu.main.players.weapon_chams_col.get(), false);
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);
		g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
		g_csgo.m_render_view->SetColorModulation(colors::white);
		return;
	}

	if (g_csgo.m_engine->IsInGame()) {
		if (strstr(info.m_model->m_name, XOR("player/contactshadow")) != nullptr) {
			return;
		}
	}

	Entity* pModelEntity = (Entity*)g_csgo.m_entlist->GetClientEntity(info.m_index);

	// do chams.
	if (g_chams.DrawModel(ctx, state, info, bone)) {
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);
	}

	if (g_menu.main.antiaim.fakelag_chams.get() && g_cl.m_local->m_vecVelocity().length_2d() > 30.f && g_csgo.m_input->CAM_IsThirdPerson() && (pModelEntity == g_cl.m_local)) {
		IMaterial* fakelagcham = nullptr;
		fakelagcham = g_csgo.m_material_system->FindMaterial(XOR("models\\inventory_items\\dogtags\\dogtags_outline"), XOR("Model textures"));
		fakelagcham->AlphaModulate(0.12f);
		g_chams.SetupMaterial(fakelagcham, colors::burgundy, false);
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, g_chams.fakelagmatrix);
		g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
		g_csgo.m_render_view->SetColorModulation(colors::white);
		return;
	}

	// disable material force for next call.
	//g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
}