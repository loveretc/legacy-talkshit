#pragma once
#include "includes.h"

class Groupbox
{
	friend class GUI;
	friend class Element;
	friend class Dropdown;
	friend class MultiDropdown;
	friend class Colorpicker;
	friend class Edit;
	friend class Config;


	const char* m_text{ };

public:
	void start(Form* data, const char* text)
	{
		m_text = text;
		m_start = vec2(data->m_x - 10.f, data->m_y + 5.f);

		// setup coord for next item
		data->m_y += 15.f;
	}

	void end(Form* data)
	{
		m_end = vec2(data->m_x - 10.f, data->m_y + 5.f);

		// setup coord for next item
		data->m_y += 15.f;

		draw(data);
	}

	void draw(Form* data)
	{
		UIcolor_t text_color(206, 206, 206, data->m_alpha);
		UIcolor_t shadow_color(0, 0, 0, data->m_alpha);
		UIcolor_t edge_color(44, 44, 44, data->m_alpha);

		float width = 211.f;

		float text_width = UI::text_width(m_text);

		// top
		render::line(edge_color, m_start.x, m_start.y, m_start.x + 8.f, m_start.y);
		UI::line(edge_color, m_start.x + 12.f + text_width, m_start.y, m_start.x + width, m_start.y);
		// left
		UI::line(edge_color, m_start.x, m_start.y, m_end.x, m_end.y);
		// right
		UI::line(edge_color, m_start.x + width, m_start.y, m_end.x + width, m_end.y + 1.f);
		UI::line(shadow_color, m_start.x + width + 1.f, m_start.y, m_end.x + width + 1.f, m_end.y + 1.f);
		// bottom
		UI::line(edge_color, m_end.x, m_end.y, m_end.x + width + 1.f, m_end.y);
		UI::line(shadow_color, m_end.x, m_end.y + 1.f, m_end.x + width + 2.f, m_end.y + 1.f);

		if (m_text)
		{
			UI::string(shadow_color, m_start.x + 10.f + 1.f, m_start.y + 1.f - 6.f, false, m_text);
			UI::string(text_color, m_start.x + 10.f, m_start.y - 6.f, false, m_text);
		}
	}
};

class c_label
{
	const char* m_text{ };
public:
	c_label()
	{

	}

	void draw(Form* data)
	{
		Color text_color(206, 206, 206, data->m_alpha);
		Color text_shadow(0, 0, 0, data->m_alpha);

		if (m_text)
		{
			render::menu.string(text_shadow, m_start.x + 20.f + 1.f, m_start.y + 1.f - 3.f, false, m_text);
			render::menu.string(text_color, m_start.x + 20.f, m_start.y - 3.f, false, m_text);
		}
	}

	void handle(c_window_data* data, const char* text)
	{
		m_text = text;
		m_start = vec2(data->m_x, data->m_y);

		// setup coord for next item
		data->m_y += 15.f;

		draw(data);
	}
};
