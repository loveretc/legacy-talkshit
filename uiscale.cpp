#include "includes.h"

namespace ui {
	// active multiplier, and every font set we have built so far keyed by
	// lround( factor * 100 ). vgui has no way to release a font, so caching
	// keeps the handle count bounded to the factors the user actually visits.
	static float                                m_factor{ 1.f };
	static std::unordered_map< int, FontSet_t >	m_sets{ };
	static FontSet_t*                           m_active{ nullptr };
	static bool                                 m_changed{ false };

	// derive a factor from the game's render resolution.
	// csgo never opts into per-monitor dpi awareness, so asking windows for the
	// monitor dpi would just return 96 and tell us nothing. the backbuffer
	// height is the only signal that actually tracks how small the menu looks.
	static float AutomaticFactor( ) {
		if( g_cl.m_height <= 0 )
			return 1.f;

		float f = ( float )g_cl.m_height / DESIGN_HEIGHT;

		// snap to quarter steps, glyphs stay crisper on round-ish pixel sizes.
		f = std::round( f * 4.f ) / 4.f;

		math::clamp( f, MIN_FACTOR, MAX_FACTOR );

		return f;
	}

	static float WantedFactor( ) {
		// note - this runs before g_menu.init during startup. a freshly
		//        constructed Dropdown reports 0 ( automatic ), which is what
		//        we want anyway.
		switch( g_menu.main.misc.scaling.get( ) ) {
		case SCALE_100: return 1.00f;
		case SCALE_125: return 1.25f;
		case SCALE_150: return 1.50f;
		case SCALE_175: return 1.75f;
		case SCALE_200: return 2.00f;
		default:        return AutomaticFactor( );
		}
	}

	// gets ( or builds ) the font set matching the current factor.
	static FontSet_t* EnsureSet( ) {
		const int key = ( int )std::lround( m_factor * 100.f );

		auto it = m_sets.find( key );
		if( it == m_sets.end( ) ) {
			FontSet_t set{ };

			// note - 'small' is a windows macro ( RpcNdr.h ), don't name a local that.
			const int base_size  = std::max( 1, ( int )std::lround( ( float )FONT_SIZE * m_factor ) );
			const int small_size = std::max( 1, ( int )std::lround( ( float )FONT_SMALL_SIZE * m_factor ) );
			const int ind_size   = std::max( 1, ( int )std::lround( ( float )FONT_INDICATOR_SIZE * m_factor ) );
			const int esp_size   = std::max( 1, ( int )std::lround( ( float )FONT_ESP_SIZE * m_factor ) );
			const int icon_size  = std::max( 1, ( int )std::lround( ( float )FONT_ICON_SIZE * m_factor ) );
			const int und_size   = std::max( 1, ( int )std::lround( ( float )FONT_UNDEFEATED_SIZE * m_factor ) );
			const int hud_size   = std::max( 1, ( int )std::lround( ( float )FONT_HUD_SIZE * m_factor ) );
			const int con_size   = std::max( 1, ( int )std::lround( ( float )FONT_CONSOLE_SIZE * m_factor ) );

			set.m_font      = render::Font( XOR( "Verdana" ), base_size, FW_NORMAL, FONTFLAG_ANTIALIAS );
			set.m_bold      = render::Font( XOR( "Verdana" ), base_size, FW_BOLD, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
			set.m_value     = render::Font( XOR( "Verdana" ), base_size, FW_BOLD, FONTFLAG_ANTIALIAS );
			set.m_small     = render::Font( XOR( "Small Fonts" ), small_size, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE );
			set.m_indicator = render::Font( XOR( "Verdana" ), ind_size, FW_BOLD, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
			set.m_esp       = render::Font( XOR( "Verdana" ), esp_size, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
			set.m_icon       = render::Font( XOR( "undefeated" ), icon_size, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
			set.m_undefeated = render::Font( XOR( "undefeated" ), und_size, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
			set.m_hud        = render::Font( XOR( "Verdana" ), hud_size, FW_NORMAL, FONTFLAG_DROPSHADOW );
			set.m_console    = render::Font( XOR( "Lucida Console" ), con_size, FW_NORMAL, FONTFLAG_DROPSHADOW );

			// unordered_map is node based, so this reference stays valid even
			// if a later insert rehashes the table.
			it = m_sets.emplace( key, set ).first;
		}

		m_active = &it->second;

		return m_active;
	}

	float factor( ) {
		return m_factor;
	}

	bool think( ) {
		const float wanted = WantedFactor( );

		// nothing moved, but we might still be missing our fonts after an
		// invalidate.
		if( wanted == m_factor ) {
			if( !m_active )
				EnsureSet( );

			m_changed = false;
			return false;
		}

		m_factor = wanted;
		EnsureSet( );

		m_changed = true;
		return true;
	}

	bool scale_changed( ) {
		return m_changed;
	}

	void invalidate( ) {
		m_sets.clear( );
		m_active = nullptr;
	}

	int logical_w( ) {
		return unpx( g_cl.m_width );
	}

	int logical_h( ) {
		return unpx( g_cl.m_height );
	}

	render::Font& font( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_font;
	}

	render::Font& font_bold( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_bold;
	}

	render::Font& font_value( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_value;
	}

	render::Font& font_small( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_small;
	}

	render::Font& font_indicator( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_indicator;
	}

	render::Font& font_esp( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_esp;
	}

	render::Font& font_icon( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_icon;
	}

	render::Font& font_undefeated( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_undefeated;
	}

	render::Font& font_hud( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_hud;
	}

	render::Font& font_console( ) {
		if( !m_active )
			EnsureSet( );

		return m_active->m_console;
	}

	void line( int x0, int y0, int x1, int y1, Color color ) {
		const int t = std::max( 1, px( 1 ) );

		if( t <= 1 ) {
			render::line( x0, y0, x1, y1, color );
			return;
		}

		// walk the perpendicular of the segment and lay down t parallel 1px
		// lines, which gives us a stroke t px wide in any direction.
		const float dx = ( float )( x1 - x0 );
		const float dy = ( float )( y1 - y0 );
		const float len = std::sqrt( dx * dx + dy * dy );

		if( len <= 0.f ) {
			render::line( x0, y0, x1, y1, color );
			return;
		}

		const float nx = -dy / len;
		const float ny = dx / len;

		// centre the stroke on the original segment.
		const float base = -( float )( t - 1 ) * 0.5f;

		for( int i{ }; i < t; ++i ) {
			const int ox = ( int )std::lround( nx * ( base + ( float )i ) );
			const int oy = ( int )std::lround( ny * ( base + ( float )i ) );

			render::line( x0 + ox, y0 + oy, x1 + ox, y1 + oy, color );
		}
	}

	void frame( int x, int y, int w, int h, Color color ) {
		const int t = std::max( 1, px( 1 ) );
		const int inner_h = std::max( 0, h - t - t );

		render::rect_filled( x, y, w, t, color );
		render::rect_filled( x, y + h - t, w, t, color );
		render::rect_filled( x, y + t, t, inner_h, color );
		render::rect_filled( x + w - t, y + t, t, inner_h, color );
	}

	void frame_outlined( int x, int y, int w, int h, Color color, Color outline ) {
		const int t = std::max( 1, px( 1 ) );

		frame( x, y, w, h, color );
		frame( x - t, y - t, w + t + t, h + t + t, outline );
	}

	void rect( int x, int y, int w, int h, Color color ) {
		const int x0 = px( x ), y0 = px( y );
		const int x1 = px( x + w ), y1 = px( y + h );

		// the thickness has to scale as well. the form frame is six stacked
		// 1px outlines, drawing those as hairlines at a high factor would turn
		// a solid border into a set of gappy lines.
		const int t = std::max( 1, px( 1 ) );

		const int inner_y = y0 + t;
		const int inner_h = std::max( 0, ( y1 - t ) - inner_y );

		render::rect_filled( x0, y0, x1 - x0, t, color );
		render::rect_filled( x0, y1 - t, x1 - x0, t, color );
		render::rect_filled( x0, inner_y, t, inner_h, color );
		render::rect_filled( x1 - t, inner_y, t, inner_h, color );
	}

	void rect_filled( int x, int y, int w, int h, Color color ) {
		// scale the edges rather than the size, that way neighbouring rects
		// keep sharing an exact edge and we get no seams / overlaps on
		// fractional factors.
		const int x0 = px( x ), y0 = px( y );
		const int x1 = px( x + w ), y1 = px( y + h );

		render::rect_filled( x0, y0, x1 - x0, y1 - y0, color );
	}

	void rect_filled_fade( int x, int y, int w, int h, Color color, int a1, int a2 ) {
		const int x0 = px( x ), y0 = px( y );
		const int x1 = px( x + w ), y1 = px( y + h );

		render::rect_filled_fade( x0, y0, x1 - x0, y1 - y0, color, a1, a2 );
	}

	void gradient( int x, int y, int w, int h, Color color1, Color color2 ) {
		const int x0 = px( x ), y0 = px( y );
		const int x1 = px( x + w ), y1 = px( y + h );

		render::gradient( x0, y0, x1 - x0, y1 - y0, color1, color2 );
	}

	void gradient2( int x, int y, int w, int h, Color color1, Color color2 ) {
		const int x0 = px( x ), y0 = px( y );
		const int x1 = px( x + w ), y1 = px( y + h );

		render::gradient2( x0, y0, x1 - x0, y1 - y0, color1, color2 );
	}

	void draw_arc( int x, int y, int radius, int start_angle, int percent, int thickness, Color color ) {
		// draw_arc walks radius down to radius - thickness drawing 1px rings, so
		// scaling both keeps the ring contiguous and the band proportional.
		render::draw_arc( px( x ), px( y ), px( radius ), start_angle, percent, std::max( 1, px( thickness ) ), color );
	}

	void string( render::Font& f, int x, int y, Color color, const std::string& text, render::StringFlags_t flags ) {
		f.string( px( x ), px( y ), color, text, flags );
	}

	void wstring( render::Font& f, int x, int y, Color color, const std::wstring& text, render::StringFlags_t flags ) {
		f.wstring( px( x ), px( y ), color, text, flags );
	}

	render::FontSize_t size( render::Font& f, const std::string& text ) {
		// the font is already at the scaled size, so this measures real px.
		// hand back design px, callers feed it straight into layout math.
		const render::FontSize_t s = f.size( text );

		return { unpx( s.m_width ), unpx( s.m_height ) };
	}

	render::FontSize_t wsize( render::Font& f, const std::wstring& text ) {
		const render::FontSize_t s = f.wsize( text );

		return { unpx( s.m_width ), unpx( s.m_height ) };
	}
}
