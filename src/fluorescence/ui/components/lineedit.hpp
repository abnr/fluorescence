/*
 * fluorescence is a free, customizable Ultima Online client.
 * Copyright (C) 2011-2012, http://fluorescence-client.org

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef FLUO_UI_COMPONENTS_LINEEDIT_HPP
#define FLUO_UI_COMPONENTS_LINEEDIT_HPP

#include <ui/gumpcomponent.hpp>

#include <ClanLib/Display/Font/font.h>
#include <ClanLib/Core/System/timer.h>

#include <misc/string.hpp>

namespace fluo {
namespace ui {
namespace components {

class LineEdit : public GumpComponent {
public:
    LineEdit(CL_GUIComponent* parent);
    ~LineEdit();

    void setFont(const UnicodeString& fontName, unsigned int fontHeight);

    void setText(const UnicodeString& string);
    UnicodeString getText();
    int getTextInt() const;

    void setAction(const UnicodeString& action);
    void onEnterPressed();

    void setEntryId(unsigned int entryId);
    unsigned int getEntryId() const;

    void setPasswordMode(bool enable = true);
	void setNumericMode(bool enable = true);
	void setMaxLength(int length);

private:
    UnicodeString action_;

    unsigned int entryId_;

    CL_Font font_;


    // clanlib stuff
    bool is_password_mode() const;
	int get_max_length() const;
	CL_StringRef get_text() const;
	CL_String get_selection() const;
	int get_selection_start() const;
	int get_selection_length() const;
	int get_cursor_pos() const;
	CL_Size get_text_size();
	CL_Size get_text_size(const CL_String &str);

	void select_all();
	void set_read_only(bool enable = true);
	void set_text(const CL_StringRef &text);
	void set_text(int number);
	void set_text(float number, int num_decimal_places = 6);
	void set_selection(int pos, int length);
	void clear_selection();
	void set_cursor_pos(int pos);
	void delete_selected_text();
	void set_input_mask(const CL_StringRef &mask);
	void set_decimal_character(const CL_StringRef &decimal_char);
	void set_cursor_drawing_enabled(bool enable);

    void on_process_message(CL_GUIMessage &msg);
	void on_render(CL_GraphicContext &gc, const CL_Rect &update_rect);
	void on_timer_expired();
	void on_scroll_timer_expired();
	void on_enable_changed();
	void update_text_clipping();

	CL_Timer timer;
	CL_String text;
	int cursor_pos;
	int max_length;
	bool mouse_selecting;
	bool password_mode;
	bool numeric_mode;
	int selection_start;
	int selection_length;
	CL_Colorf text_color;
	CL_String input_mask;
	CL_String decimal_char;
	bool cursor_drawing_enabled_when_parent_focused;

	static CL_String break_characters;

	void move(int steps, CL_InputEvent &e);
	bool insert_text(int pos, const CL_StringRef &str);
	void backspace();
	void del();
	int get_character_index(int mouse_x_wincoords);
	int find_next_break_character(int pos);
	int find_previous_break_character(int pos);
	CL_String get_visible_text_before_selection();
	CL_String get_visible_text_after_selection();
	CL_String get_visible_selected_text();
	CL_String create_password(CL_String::size_type num_letters) const;
	CL_Size get_visual_text_size(CL_GraphicContext &gc, CL_Font &font, int pos, int npos) const;
	CL_Size get_visual_text_size(CL_GraphicContext &gc, CL_Font &font) const;
	CL_Rect get_cursor_rect();
	CL_Rect get_selection_rect();
	bool input_mask_accepts_input(int cursor_pos, const CL_StringRef &str);
	void set_selection_start(int start);
	void set_selection_length(int length);
	void set_text_selection(int start, int length);

	CL_Timer scroll_timer;

	bool mouse_moves_left;
	bool cursor_blink_visible;
	unsigned int blink_timer;
	int clip_start_offset;
	int clip_end_offset;
	bool ignore_mouse_events;

	struct UndoInfo
	{
		/* set undo text when:
  		   - added char after moving
		   - destructive block operation (del, cut etc)
		   - beginning erase
		*/

		UndoInfo() : first_erase(0), first_text_insert(0) {}
		CL_String undo_text;
		bool first_erase;
		bool first_text_insert;
	} undo_info;

	bool select_all_on_focus_gain;

	static const CL_StringRef numeric_mode_characters;

};

}
}
}

#endif
