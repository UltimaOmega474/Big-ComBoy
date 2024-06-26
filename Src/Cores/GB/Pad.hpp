/*
    Big ComBoy
    Copyright (C) 2023-2024 UltimaOmega474

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <cinttypes>

namespace GB {
    enum class PadButton { Left, Right, Up, Down, A, B, Select, Start };

    class Gamepad {
    public:
        void reset();
        void clear_buttons();
        void set_pad_state(PadButton btn, bool pressed);
        void select_button_mode(uint8_t value);
        uint8_t get_pad_state();

    private:
        uint8_t dpad = 0xFF, action = 0xFF, mode = 0;
    };
}