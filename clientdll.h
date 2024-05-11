#pragma once



enum Stage_t {
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

enum Hitgroups_t {
	HITGROUP_GENERIC  = 0,
	HITGROUP_HEAD     = 1,
	HITGROUP_CHEST    = 2,
	HITGROUP_STOMACH  = 3,
	HITGROUP_LEFTARM  = 4,
	HITGROUP_RIGHTARM = 5,
	HITGROUP_LEFTLEG  = 6,
	HITGROUP_RIGHTLEG = 7,
	HITGROUP_NECK     = 8,
	HITGROUP_GEAR     = 10
};

class CHLClient {
public:
	enum indices : size_t {
		INIT                = 0,
		LEVELINITPREENTITY  = 5,
		LEVELINITPOSTENTITY = 6,
		LEVELSHUTDOWN		= 7,
		GETALLCLASSES       = 8,
		HUDPROCESSINPUT     = 10,
		INACTIVATEMOUSE     = 15,
		INKEYEVENT          = 20,
		CREATEMOVE          = 21,
		RENDERVIEW          = 27,
		FRAMESTAGENOTIFY    = 36,
		DISPATCHUSERMESSAGE = 37,
	};

public:
	__forceinline ClientClass* GetAllClasses( ) {
		return util::get_method< ClientClass*( __thiscall* )( decltype( this ) )>( this, GETALLCLASSES )( this );
	}
};

#define Assert( _exp ) ( ( void )0 )
#define OFFSET( funcname, type, offset ) type& funcname( vec3_t vec )
#define OFFSETRS( funcname, type, offset ) type& funcname( int index ) \
{ \
	static uint16_t _offset = offset; \
	Assert( _offset ); \
	return *reinterpret_cast<type*>( uintptr_t( this ) + _offset + index * 4 ); \
}

class CSPlayerResource
{
public:
	// helper methods.
	template< typename t >
	__forceinline t& get(size_t offset) {
		return *(t*)((uintptr_t)this + offset);
	}

	template< typename t >
	__forceinline void set(size_t offset, const t& val) {
		*(t*)((uintptr_t)this + offset) = val;
	}

	template< typename t >
	__forceinline t as() {
		return (t)this;
	}

public:
	OFFSET(bombsiteA, vec3_t, g_netvars.get(HASH("DT_CSPlayerResource"), HASH("m_bombsiteCenterA")));
	OFFSET(bombsiteB, vec3_t, g_netvars.get(HASH("DT_CSPlayerResource"), HASH("m_bombsiteCenterB")));
	OFFSETRS(get_ping, int, 0x0AE4);
};