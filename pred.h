#pragma once

class InputPrediction {
public:
	float m_curtime;
	float m_frametime;
	int m_predicted_flags{ };
	bool m_first_time_predicted;
	bool m_in_prediction;


	struct {
		CMoveData data{ };
		int m_tickbase{ };
		int m_flags{ };
		int m_move_type{ };
		vec3_t m_origin{ }, m_view_offset{ }, m_velocity{ };
		ang_t  m_view_punch{ }, m_aim_punch{ }, m_aim_punch_vel{ };

		// note; these are just as predicted as the ones above and they all feed
		// back into movement / spread. leaving them out made every re-prediction
		// start from a state the previous one had already mutated.
		float  m_stamina{ }, m_velocity_modifier{ }, m_duck_amount{ }, m_fall_velocity{ }, m_surface_friction{ };
	} m_data;

	// note; by reference. these used to take CMoveData by value, which made the
	// memcpy that was supposed to restore the move data write into a copy that
	// got thrown away on return.
	void ApplyPredictedNetvars(CMoveData& data);
	void StorePredictedNetvars(const CMoveData& data);


public:
	void Initialize();

	// function: puts the engines choked command clamp back the way we found it.
	void Restore();

	void UpdateGamePrediction(CUserCmd* cmd);
	void RunGamePrediction(CUserCmd* cmd);
	void PredictGamePrediction(CMoveData& data, CUserCmd* cmd);
	void RestoreGamePrediction(CUserCmd* cmd);

private:
	bool m_init_attempted{ false };
	bool m_unlocked_fakelag{ false };

	// where we patched the clamp and what used to be there.
	std::uint32_t* m_move_clamp{ nullptr };
	std::uint32_t  m_move_clamp_original{ };
};

extern InputPrediction g_inputpred;