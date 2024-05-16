#include "includes.h"
#include <codecvt>
#include <locale>
#define MINUSHEIGHT -1
#define PLUSHEIGHT  -2

std::wstring string_to_wstring(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

void Slider::draw( ) {
	Rect  area{ m_parent->GetElementsRect( ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };
	Rect slider{ p.x + SLIDER_X_OFFSET, p.y + m_offset, m_w - SLIDER_X_OFFSET, SLIDER_HEIGHT };

	// get gui color.
	Color color = g_gui.m_color;
	color.a( ) = m_parent->m_alpha;

	// draw label.
	if( m_use_label )
		render::menu.string( p.x + LABEL_OFFSET, p.y - 2, { 205, 205, 205, m_parent->m_alpha }, m_label );

	// outline.
	render::rect( p.x + SLIDER_X_OFFSET, p.y + m_offset, m_w - SLIDER_X_OFFSET, SLIDER_HEIGHT, { 15,15,15, m_parent->m_alpha } );

	// background.
	render::gradient( p.x + SLIDER_X_OFFSET + 1, p.y + m_offset + 1, m_w - SLIDER_X_OFFSET - 2, SLIDER_HEIGHT - 2, { 55, 55, 55, m_parent->m_alpha }, { 70, 70, 70, m_parent->m_alpha } );

	// bar.
	render::rect_filled( p.x + SLIDER_X_OFFSET + 1, p.y + m_offset + 1, m_fill - 2, SLIDER_HEIGHT - 2, color );
	render::rect_filled_fade(p.x + SLIDER_X_OFFSET + 1, p.y + m_offset + 1, m_fill - 2, SLIDER_HEIGHT - 2, { 50, 50, 35, m_parent->m_alpha }, 0, 150);

	// to stringstream.
	std::wstringstream ss;

	// this one is for generic values (so we dont need to create for each)
	std::map<float, std::wstring> generic_values = {
	{0.f, L"Off"},
	};

	// values for HP+x
	std::map<float, std::wstring> special_texts = {
	{0.f,   L"Auto"},
	{101.f, L"HP+1"},
	{102.f, L"HP+2"},
	{103.f, L"HP+3"},
	{104.f, L"HP+4"},
	{105.f, L"HP+5"},
	{106.f, L"HP+6"},
	{107.f, L"HP+7"},
	{108.f, L"HP+8"},
	{109.f, L"HP+9"},
	{110.f, L"HP+10"},
	{111.f, L"HP+11"},
	{112.f, L"HP+12"},
	{113.f, L"HP+13"},
	{114.f, L"HP+14"},
	{115.f, L"HP+15"},
	{116.f, L"HP+16"},
	{117.f, L"HP+17"},
	{118.f, L"HP+18"},
	{119.f, L"HP+19"},
	{120.f, L"HP+20"},
	{121.f, L"HP+21"},
	{122.f, L"HP+22"},
	{123.f, L"HP+23"},
	{124.f, L"HP+24"},
	{125.f, L"HP+25"},
	{126.f, L"HP+26"},
	};

	// aspect ratio values
	std::map<float, std::wstring> aspect_ratio = {
	{1.25f, L"5:4"},
	{1.33f, L"4:3 "},
	{1.50f, L"3:2"},
	{1.60f, L"16:10"},
	{1.78f, L"16:9"},
	};

	if (m_file_id == "minimal_damage" || m_file_id == "dmg_override") {
		auto it = special_texts.find(m_value);
		if (it != special_texts.end()) {
			// if we have a special value, override the numeric one
			ss << it->second;
		}
		else {
			// if not, show the numeric value
			ss << std::fixed << std::setprecision(m_precision) << m_value << m_suffix;
		}
	}
	else if (m_file_id == "hitchance_amount") {
		auto it = generic_values.find(m_value);
		if (it != generic_values.end()) {
			ss << it->second;
		}
		else {
			ss << std::fixed << std::setprecision(m_precision) << m_value << m_suffix;
		}
	}
	else if (m_file_id == "aspect") {
		auto it = aspect_ratio.find(m_value);
		if (it != aspect_ratio.end()) {
			ss << it->second;
		}
		else {
			ss << std::fixed << std::setprecision(m_precision) << m_value << m_suffix;
		}
	}
	else {
		// slider doesnt have a special value, show normal stringstream
		ss << std::fixed << std::setprecision(m_precision) << m_value << m_suffix;
	}

	// get size.
	render::FontSize_t size = render::slider.wsize( ss.str( ) );

	// outline
	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2) + 1, (p.y - 1 + m_offset + 1) + 1, { 17, 17, 17, m_parent->m_alpha }, ss.str());
	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2) - 1, (p.y - 1 + m_offset + 1) - 1, { 17, 17, 17, m_parent->m_alpha }, ss.str());
	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2) + 1, (p.y - 1 + m_offset + 1) - 1, { 17, 17, 17, m_parent->m_alpha }, ss.str());
	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2) - 1, (p.y - 1 + m_offset + 1) + 1, { 17, 17, 17, m_parent->m_alpha }, ss.str());

	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2) + 1, (p.y - 1 + m_offset + 1), { 17, 17, 17, m_parent->m_alpha }, ss.str());
	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2) - 1, (p.y - 1 + m_offset + 1), { 17, 17, 17, m_parent->m_alpha }, ss.str());
	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2), (p.y - 1 + m_offset + 1) - 1, { 17, 17, 17, m_parent->m_alpha }, ss.str());
	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2), (p.y - 1 + m_offset + 1) + 1, { 17, 17, 17, m_parent->m_alpha }, ss.str());

	// draw value.
	render::slider.wstring(p.x + SLIDER_X_OFFSET + m_fill - (size.m_width / 2), p.y - 1 + m_offset + 1, { 191, 191, 191, m_parent->m_alpha }, ss.str( ) );

	// - and + buttons
	int buttonSize = 5;
	int buttonSpacingminus = -10;
	int buttonSpacingplus = 5;

	// draw "-" button
	if (m_value != m_min)
		render::menu.string(p.x - buttonSpacingminus, p.y + m_offset + (MINUSHEIGHT - buttonSize) / 2, { 205, 205, 205, m_parent->m_alpha }, "-");

	// draw "+" button
	if (m_value != m_max)
		render::menu.string(p.x + m_w + buttonSpacingplus, p.y + m_offset + (PLUSHEIGHT - buttonSize) / 2, { 205, 205, 205, m_parent->m_alpha }, "+");
}

void Slider::think( ) {
	Rect  area{ m_parent->GetElementsRect( ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// how many steps do we have?
	float steps = ( m_max - m_min ) / m_step;

	// compute the amount of pixels for one step.
	float pixels = ( m_w - SLIDER_X_OFFSET ) / steps;

	// clamp the current value.
	math::clamp( m_value, m_min, m_max );

	// compute the fill ratio.
	m_fill = ( int )std::floor( std::max( ( ( m_value - m_min ) / m_step ) * pixels, 0.f ) );

	// check for button clicks
	int buttonSize = 8;
	int buttonSpacingminus = -15;
	int buttonSpacingplus = 5;

	// - button
	// adjust value depending on precision
	if (g_input.IsCursorInRect(Rect{ p.x - buttonSpacingminus - buttonSize, p.y + m_offset, buttonSize, buttonSize })) {
		if (g_input.GetKeyPress(VK_LBUTTON)) {
			/*if (m_precision == 2)
				m_value -= 0.01f;
			else if (m_precision == 1)
				m_value -= 0.1f;
			else
				m_value -= 1;*/

			m_value -= m_step;

			math::clamp(m_value, m_min, m_max);
			if (m_callback)
				m_callback();
		}
	}

	// + button
	else if (g_input.IsCursorInRect(Rect{ p.x + m_w + buttonSpacingplus, p.y + m_offset, buttonSize, buttonSize })) {
		if (g_input.GetKeyPress(VK_LBUTTON)) {
			/*if (m_precision == 2)
				m_value += 0.01f;
			else if (m_precision == 1)
				m_value += 0.1f;
			else
				m_value += 1;*/

			m_value += m_step;

			math::clamp(m_value, m_min, m_max);
			if (m_callback)
				m_callback();
		}
	}

	// we are draggin this mofo!
	if( m_drag ) {
		// left mouse is still down.
		if( g_input.GetKeyState( VK_LBUTTON ) ) {

			// compute the new value.
			float updated = m_min + ( ( ( g_input.m_mouse.x - SLIDER_X_OFFSET ) - p.x ) / pixels * m_step );

			// set updated value to closest step.
			float remainder = std::fmod( updated, m_step );

			if( remainder > ( m_step / 2 ) )
				updated += m_step - remainder;

			else
				updated -= remainder;

			m_value = updated;

			// clamp the value.
			math::clamp( m_value, m_min, m_max );

			if( m_callback )
				m_callback( );
		}

		// left mouse has been released.
		else 
			m_drag = false;
	}
}

void Slider::click( ) {
	Rect  area{ m_parent->GetElementsRect( ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// get slider area.
	Rect slider{ p.x + SLIDER_X_OFFSET, p.y + m_offset, m_w - SLIDER_X_OFFSET, SLIDER_HEIGHT };

	// detect dragging.
	if( g_input.IsCursorInRect( slider ) )
		m_drag = true;

	// get incrementor area.
	//Rect increment{ l.x + m_w - SLIDER_OFFSET + 1, l.y + SLIDER_OFFSET_Y + 1, 6, 6 };

	// get decrementor area.
	//Rect decrement{ l.x - 6, l.y + SLIDER_OFFSET_Y + 1, 6, 6 };

	// increment value.
	//else if( g_input.is_mouse_in_area( increment ) )
	//	m_value += m_step;

	// decrement value.
	//else if( g_input.is_mouse_in_area( decrement ) )
	//	m_value -= m_step;

	// clamp the updated value.
	math::clamp( m_value, m_min, m_max );
}