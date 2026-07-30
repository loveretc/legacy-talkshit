#pragma once
#include <cmath>

// the entire gui is laid out in 'design pixels' - a virtual 1080p canvas.
// this namespace is the only place where design pixels turn into real screen
// pixels, so no widget ever has to know about the current scale:
//
//   drawing -> design px in, scaled px out.
//   input   -> the real cursor position is divided back down into design px
//              once, in GUI::think, so every hit-test rect stays in design px.
//   fonts   -> the gui owns its own set ( render::menu / esp_small are shared
//              with the esp, scaling those would resize player flags ).
//              created at base_size * factor and cached per factor.
//
// note - the layout constants in element.h / tab.h / the widget headers are
//        design px and deliberately stay untouched.

namespace ui {
	// order must match the items of MiscTab::scaling.
	enum ScaleMode : size_t {
		SCALE_AUTOMATIC = 0,
		SCALE_100,
		SCALE_125,
		SCALE_150,
		SCALE_175,
		SCALE_200
	};

	// the canvas the menu was originally designed against.
	constexpr float DESIGN_HEIGHT = 1080.f;

	// bounds for the automatic mode.
	constexpr float MIN_FACTOR = 1.f;
	constexpr float MAX_FACTOR = 3.f;

	// base ( unscaled ) font sizes, taken from render::init.
	constexpr int FONT_SIZE           = 12;
	constexpr int FONT_SMALL_SIZE     = 8;
	constexpr int FONT_INDICATOR_SIZE = 26;
	constexpr int FONT_ESP_SIZE        = 12;
	constexpr int FONT_ICON_SIZE       = 16;
	constexpr int FONT_UNDEFEATED_SIZE = 25;
	constexpr int FONT_HUD_SIZE        = 12;
	constexpr int FONT_CONSOLE_SIZE    = 10;

	struct FontSet_t {
		render::Font m_font;
		render::Font m_bold;
		render::Font m_value;
		render::Font m_small;
		render::Font m_indicator;
		render::Font m_esp;
		render::Font m_icon;
		render::Font m_undefeated;
		render::Font m_hud;
		render::Font m_console;
	};

	// current design px -> real px multiplier.
	float factor( );

	// recomputes the factor and swaps in the matching font set.
	// called once per frame from Client::OnPaint, before anything measures or
	// draws, so the esp indicators and the menu always agree on the factor.
	// returns true on the frame the factor changed.
	bool think( );

	// whether think( ) changed the factor this frame.
	bool scale_changed( );

	// drops every cached font set. must be called whenever the game recreates
	// its fonts ( render::init ), otherwise we hand out stale handles.
	void invalidate( );

	// screen bounds, in design px.
	int logical_w( );
	int logical_h( );

	// design px -> real px.
	__forceinline int px( int v ) {
		return ( int )std::lround( ( float )v * factor( ) );
	}

	// real px -> design px.
	__forceinline int unpx( int v ) {
		return ( int )std::lround( ( float )v / factor( ) );
	}

	// fonts of the active set.
	render::Font& font( );
	render::Font& font_bold( );
	render::Font& font_value( );
	render::Font& font_small( );
	render::Font& font_indicator( );

	// the esp draws in real px ( its anchors come out of WorldToScreen, which
	// already scales with the resolution ). these fonts exist so the *text* on
	// top of those anchors follows the interface scale too - call .string( ) on
	// them directly and wrap the fixed pixel offsets in px( ).
	render::Font& font_esp( );
	render::Font& font_icon( );
	render::Font& font_undefeated( );
	render::Font& font_hud( );
	render::Font& font_console( );

	// real px endpoints, stroke width scaled. render::line is always 1px, which
	// leaves skeletons hair-thin at high factors.
	void line( int x0, int y0, int x1, int y1, Color color );

	// real px bounds with a scaled stroke, for esp boxes - their bounds already
	// come out of WorldToScreen in real px, only the outline needs scaling.
	void frame( int x, int y, int w, int h, Color color );
	void frame_outlined( int x, int y, int w, int h, Color color, Color outline );

	// primitives, all coordinates in design px.
	void rect( int x, int y, int w, int h, Color color );
	void rect_filled( int x, int y, int w, int h, Color color );
	void rect_filled_fade( int x, int y, int w, int h, Color color, int a1, int a2 );
	void gradient( int x, int y, int w, int h, Color color1, Color color2 );
	void gradient2( int x, int y, int w, int h, Color color1, Color color2 );

	// angles stay angles, only the geometry scales.
	void draw_arc( int x, int y, int radius, int start_angle, int percent, int thickness, Color color );

	void string( render::Font& f, int x, int y, Color color, const std::string& text, render::StringFlags_t flags = render::ALIGN_LEFT );
	void wstring( render::Font& f, int x, int y, Color color, const std::wstring& text, render::StringFlags_t flags = render::ALIGN_LEFT );

	// measurement, handed back in design px so it can be fed straight into layout.
	render::FontSize_t size( render::Font& f, const std::string& text );
	render::FontSize_t wsize( render::Font& f, const std::wstring& text );
}
