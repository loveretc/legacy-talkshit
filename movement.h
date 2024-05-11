#pragma once

struct custom_cmd
{
	ang_t			viewAngles;
	float			forwardmove;
	float			sidemove;
	float			upmove;
};

class Movement {
public:
	float  m_speed;
	float  m_ideal;
	float  m_ideal2;
	vec3_t m_mins;
	vec3_t m_maxs;
	vec3_t m_origin;
	float  m_switch_value = 1.f;
	int    m_strafe_index;
	float m_max_player_speed;
	float m_max_weapon_speed;
	float  m_old_yaw;
	float  m_circle_yaw;
	bool   m_invert;
	vec3_t start_position = vec3_t(0.0f, 0.0f, 0.0f);
	bool   fired_shot = false;
	vec3_t quickpeekstartpos;
	std::vector<custom_cmd>usercmds_for_quickpeek;
	int qp_count;
	bool hasShot;

	struct {
		vec3_t m_source{};
		bool m_going_back{};
		float m_animation{};

		int m_last_ticks_going_back{};
		float m_last_shot_time{};
		bool m_can_recharge{};

		bool m_working{};
		bool m_should_stop{};

		__forceinline void reset() {
			m_source = vec3_t{};
			m_going_back = false;
			m_last_ticks_going_back = -1;
			m_last_shot_time = -1.f;
			m_can_recharge = true;
			m_should_stop = false;
		}
	} m_peek_state;

	bool m_is_any_move_pressed{};

public:
	void JumpRelated( );
	void Strafe( );
	void DoPrespeed( );
	bool GetClosestPlane( vec3_t& plane );
	bool WillCollide( float time, float step );
	void FixMove( CUserCmd* cmd, const ang_t& old_angles );
	void AutoPeek(CUserCmd* cmd, float wish_yaw);
	void AutoPeek();
	void NullVelocity();
	void AutoStop();
	void accelerate(const CUserCmd& cmd, const vec3_t& wishdir, const float wishspeed, vec3_t& velocity, float acceleration);
	void walk_move(const CUserCmd& cmd, vec3_t& move, vec3_t& fwd, vec3_t& right, vec3_t& velocity);
	void full_walk_move(const CUserCmd cmd, vec3_t move, vec3_t fwd, vec3_t right, vec3_t velocity);
	void modify_move(CUserCmd cmd, vec3_t velocity);
	void predict_move(const CUserCmd cmd, vec3_t velocity);
	void QuickStop();
	void FakeWalk( );
	void FastStop();
	void ClampMovementSpeed(float speed);
	void EdgeJump(CUserCmd* cmd);
};

extern Movement g_movement;