#pragma once

// note; handle of this module, stored by DllMain in entry.cpp.
extern HMODULE g_module;

class Unloader {
public:
	// function: asks for an unload. the button that calls this runs from inside
	// PaintTraverse, so all we do here is raise a flag; the actual work happens at the
	// top of the next frame where we are not buried in our own call stack.
	__forceinline void Request( ) {
		m_pending = true;
	}

	__forceinline bool Done( ) const {
		return m_done;
	}

	// function: remembers a convars value so we can hand it back on unload.
	// call this right before changing one. only the first value seen is kept.
	void RememberConvar( ConVar* cvar );

	// function: runs a pending unload. safe to call every frame.
	void Think( );

private:
	bool RestoreGameState( );
	void RestoreHooks( );

	static ulong_t __stdcall UnloadThread( void* module );

private:
	struct convar_backup_t {
		ConVar*     m_cvar;
		std::string m_value;
	};

	std::vector< convar_backup_t > m_convars;

	bool m_pending{ false };
	bool m_done{ false };
};

extern Unloader g_unloader;
