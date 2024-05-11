#include "includes.h"

const std::string keynames[] = {
	"", XOR("[M1]"), XOR("[M2]"), XOR("[CN]"), XOR("[M3]"), XOR("[M4]"), XOR("[M5]"), XOR("[-]"),
	XOR("[BAC]"), XOR("[TAB]"), XOR("[-]"), XOR("[-]"), XOR("[CLR]"), XOR("[RET]"), XOR("[-]"), XOR("[-]"), XOR("[SHI]"),
	XOR( "[CTL]" ), XOR( "[ALT]" ), XOR( "[PAU]" ), XOR( "[CAP]" ), XOR( "[KAN]" ), XOR( "[-]" ), XOR( "[JUN]" ), XOR( "[FIN]" ), XOR( "[KAN]" ),
	XOR( "[-]" ), XOR( "[ESC]" ), XOR( "[CON]" ), XOR( "[NCO]" ), XOR( "[ACC]" ), XOR( "[MAD]" ), XOR( "[SPA]" ), XOR( "[PGU]" ), XOR( "[PGD]" ), XOR( "[END]" ),
	XOR( "[HOM]" ), XOR( "[LEF]" ), XOR( "[UP]" ), XOR( "[RIG]" ), XOR( "[DOW]" ), XOR( "[SEL]" ), XOR( "[PRI]" ), XOR( "[EXE]" ), XOR( "[PRI]" ), XOR( "[INS]" ),
	XOR("[DEL]"), XOR("[HEL]"), XOR("[0]"), XOR("[1]"), XOR("[2]"), XOR("[3]"), XOR("[4]"), XOR("[5]"), XOR("[6]"), XOR("[7]"), XOR("[8]"), XOR("[9]"),
	XOR( "[-]" ),  XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[A]" ), XOR( "[B]" ), XOR( "[C]" ), XOR( "[D]" ),
	XOR("[E]"),  XOR("[F]"), XOR("[G]"), XOR("[H]"), XOR("[I]"), XOR("[J]"), XOR("[K]"), XOR("[L]"), XOR("[M]"), XOR("[N]"), XOR("[O]"), XOR("[P]"), XOR("[Q]"),
	XOR( "[R]" ), XOR( "[S]" ), XOR( "[T]" ), XOR( "[U]" ), XOR( "[V]" ), XOR( "[W]" ), XOR( "[X]" ), XOR( "[Y]" ), XOR( "[Z]" ), XOR( "[WIN]" ), XOR( "[WIN]" ),
	XOR( "[APP]" ), XOR( "[-]" ),  XOR( "[SLE]" ), XOR( "[NUM]" ), XOR( "[NUM]" ), XOR( "[NUM]" ), XOR( "[NUM]" ), XOR( "[NUM]" ), XOR( "[NUM]" ),
	XOR( "[NUM]"), XOR("[NUM]"), XOR("[NUM]"),  XOR("[NUM]"), XOR("[MUL]"), XOR("[ADD]"), XOR("[SEP]"), XOR("[MIN]"), XOR("[DEC]"),
	XOR( "[DIV]" ), XOR( "[F1]" ), XOR( "[F2]" ),  XOR( "[F3]" ), XOR( "[F4]" ), XOR( "[F5]" ), XOR( "[F6]" ), XOR( "[F7]" ), XOR( "[F8]" ), XOR( "[F9]" ), XOR( "[F10]" ),
	XOR("[F11]"), XOR("[F12]"), XOR("[F13]"), XOR("[F14]"), XOR("[F15]"), XOR("[F16]"), XOR("[F17]"),  XOR("[F18]"), XOR("[F19]"), XOR("[F20]"), XOR("[F21]"),
	XOR( "[F22]" ), XOR( "[F23]" ), XOR( "[F24]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ),
	XOR( "[-]" ), XOR( "[NUM]" ), XOR( "[SCR]" ), XOR( "[EQU]" ), XOR( "[MAS]" ), XOR( "[TOY]" ), XOR( "[OYA]" ), XOR( "[OYA]" ), XOR( "[-]" ), XOR( "[-]" ),
	XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ),  XOR( "[-]" ), XOR( "[-]" ), XOR( "[-]" ), XOR( "[SHI]" ), XOR( "[SHI]" ), XOR( "[CTR]" ),
	XOR("[CTR]"), XOR("[ALT]"), XOR("[ALT]"), XOR("[-]"), XOR("[-]"), XOR("[-]"), XOR("[-]"), XOR("[-]"), XOR("[-]"), XOR("[M]"), XOR("[D]"),
	XOR("[C]"), XOR("[B]"),  XOR("[P]"), XOR("[Q]"), XOR("[J]"), XOR("[G]"), XOR("[180]"), XOR("[181]"), XOR("[182]"), XOR("f"), XOR("[184]"), XOR("[185]"),
	XOR( "[;]" ), XOR( "[=]" ), XOR( " )," ), XOR( "-" ), XOR( "[.]" ), XOR( "[/]" ), XOR( "grave" ), XOR( "[193]" ), XOR( "[194]" ), XOR( "[195]" ), XOR( "[196]" ),
	XOR( "[197]" ), XOR( "[198]" ),  XOR( "[199]" ), XOR( "[200]" ), XOR( "[201]" ), XOR( "[202]" ), XOR( "[203]" ), XOR( "[204]" ), XOR( "[205]" ), XOR( "[206]" ),
	XOR( "[207]" ), XOR( "[208]" ), XOR( "[209]" ), XOR( "[210]" ), XOR( "[211]" ), XOR( "[212]" ), XOR( "[213]" ), XOR( "[214]" ), XOR( "[215]" ), XOR( "[216]" ),
	XOR( "[217]" ),  XOR( "[218]" ), XOR( "[" ), XOR( "\\" ), XOR( "]" ), XOR( "[222]" ), XOR( "[223]" ), XOR( "[224]" ), XOR( "[225]" ), XOR( "\\" ), XOR( "[227]" ),
	XOR( "[228]" ), XOR( "[229]" ), XOR( "[230]" ), XOR( "[231]" ), XOR( "[232]" ), XOR( "[233]" ), XOR( "right win" ), XOR( "[235]" ), XOR( "[236]" ), XOR( "[237]" ),
	XOR( "[238]" ), XOR( "[239]" ), XOR( "[240]" ), XOR( "left win" ), XOR( "[242]" ), XOR( "[243]" ), XOR( "[244]" ), XOR( "[245]" ), XOR( "[246]" ), XOR( "[247]" ),
	XOR( "[248]" ), XOR( "application" ), XOR( "[250]" ), XOR( "[251]" ), XOR( "[252]" ), XOR( "[253]" )
};;

void Keybind::draw( ) {
	Rect  area{ m_parent->GetElementsRect() };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// get gui color.
	Color color = g_gui.m_color;
	color.a() = m_parent->m_alpha;

	// draw label.
	if (m_safe)
		render::menu.string(p.x + KEYBIND_X_OFFSET, p.y - 2, { 205, 205, 205, m_parent->m_alpha }, m_label);
	else
		render::menu.string(p.x + KEYBIND_X_OFFSET, p.y - 2, { 205, 205, 103, m_parent->m_alpha }, m_label);

	if (!m_set && !(m_key >= 0 && m_key <= 0xFE))
		render::esp_small.string(p.x + KEYBIND_X_OFFSET + KEYBIND_ITEM_X_OFFSET + 130, p.y + 15 + 4 - 19, { 90, 90, 90, m_parent->m_alpha }, XOR("[-]"));

	if (!m_set && m_key >= 0 && m_key <= 0xFE)
		render::esp_small.string(p.x + KEYBIND_X_OFFSET + KEYBIND_ITEM_X_OFFSET + 130, p.y + 15 + 4 - 19, { 90, 90, 90, m_parent->m_alpha }, XOR(""));

	// todo; animate with '...'
	if (m_set)
		render::esp_small.string(p.x + KEYBIND_X_OFFSET + KEYBIND_ITEM_X_OFFSET + 130, p.y + 15 + 4 - 19, { 255, 16, 16, m_parent->m_alpha }, XOR("[-]"));

	// we have a key assigned.
	else if (m_key >= 0 && m_key <= 0xFE)
		render::esp_small.string(p.x + KEYBIND_X_OFFSET + KEYBIND_ITEM_X_OFFSET + 130, p.y + 15 + 4 - 19, { 90, 90, 90, m_parent->m_alpha }, keynames[m_key]);
}

void Keybind::think( ) {
	// we want to set a key.
	if (m_set) {

		// iterate all keystates.
		for (int i{}; i <= 0xFE; ++i) {

			// find pressed key.
			if (g_input.GetKeyPress(i)) {
				// escape is our wildcard.
				if (i == VK_ESCAPE) {
					m_key = -1;
					m_set = false;

					if (m_callback)
						m_callback();

					return;
				}

				m_key = i;
				m_set = false;
				m_old_set = true;

				if (m_callback)
					m_callback();

				return;
			}
		}
	}

	else m_old_set = false;
}

void Keybind::click( ) {
	Rect  area{ m_parent->GetElementsRect() };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// area where user has to click.
	Rect bind = { p.x + KEYBIND_X_OFFSET + KEYBIND_ITEM_X_OFFSET + 130, p.y + 15 + 4 - 19, m_w - KEYBIND_X_OFFSET + 10, KEYBIND_BOX_HEIGHT };

	if (!m_set && !m_old_set && g_input.IsCursorInRect(bind))
		m_set = true;
}