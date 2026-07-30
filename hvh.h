#pragma once

class AdaptiveAngle {
public:
	float m_yaw;
	float m_dist;

public:
	// ctor.
	// note; this used to take a 'penalty' that nothing ever passed a value for.
	__forceinline AdaptiveAngle( float yaw ) {
		// set yaw.
		m_yaw = math::NormalizedAngle( yaw );

		// init distance.
		m_dist = 0.f;
	}
};

enum AntiAimMode : size_t {
	STAND = 0,
	WALK,
	AIR,
};

class HVH {
public:
	size_t m_mode;
	int    m_pitch;
	int    m_yaw;
	float  m_jitter_range;
	float  m_rot_range;
	float  m_rot_speed;
	float  m_rand_update;
	int    m_dir;
	float  m_dir_custom;
	size_t m_base_angle;
	float  m_auto_time;

	bool   m_step_switch;
	int    m_random_lag;
	float  m_next_random_update;
	float  m_random_angle;
	float  m_direction;
	float  m_auto;
	float m_last_real, m_last_fake;
	bool m_swap;
	float  m_auto_dist;
	float  m_auto_last;
	float  m_view;

	bool   m_left, m_right, m_back, m_forward;

	// note; the final packet of a move can never be choked, so we lose the ability to
	//       shoot a couple of ticks before we hit the actual choke limit.
	//       the engine clamp itself is patched open in InputPrediction::Initialize, but
	//       the server still only processes sv_maxusrcmdprocessticks ( 16 ) per update,
	//       which is what Client::m_max_lag reflects.
	static constexpr int k_max_shootable_lag = 14;

public:
	// function: is the user holding a manual anti-aim direction right now?
	// note; defined in the cpp, this header is included before the menu exists.
	bool IsManualActive( );

	void IdealPitch( );
	void AntiAimPitch( );
	void AutoDirection( );
	void GetAntiAimDirection( );
    bool DoEdgeAntiAim( Player *player, ang_t &out );
	void DoRealAntiAim( );
	void DoFakeAntiAim( );
	void AntiAim( );
	void SendPacket( );
};

extern HVH g_hvh;