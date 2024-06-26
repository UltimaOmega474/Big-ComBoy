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

#include "Core.hpp"
#include "Constants.hpp"
#include "PPU.hpp"
#include <fstream>

namespace GB {
    Core::Core() : bus(this), ppu(this), timer(this), cpu(this), dma(this) {}

    void Core::initialize(Cartridge *cart) {
        ready_to_run = cart ? true : false;
        if (!cart) {
            return;
        }

        cart->reset();
        bootstrap.clear();
        apu.reset();
        ppu.reset();
        timer.reset();
        pad.reset();
        bus.reset(cart);
        dma.reset();

        if (ready_to_run) {

            if (cart->header().cgb_support == 0x80 || cart->header().cgb_support == 0xC0) {
                bus.KEY0 = cart->header().cgb_support;
                ppu.write_register(0x6C, 0);
            } else {
                bus.KEY0 = DISABLE_CGB_FUNCTIONS;
                ppu.write_register(0x6C, 1);

                ppu.set_compatibility_palette(PaletteID::BG, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ1, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ2, LCD_GRAY);
            }

            bus.bootstrap_mapped_ = false;

            cpu.reset(0x0100);
            ppu.set_post_boot_state();
        }
    }

    void Core::initialize_with_bootstrap(Cartridge *cart, ConsoleType console,
                                         std::filesystem::path bootstrap_path) {
        ready_to_run = cart ? true : false;

        if (!cart) {
            return;
        }

        cart->reset();
        bootstrap.clear();
        apu.reset();
        ppu.reset();
        timer.reset();
        pad.reset();
        bus.reset(cart);
        dma.reset();

        if (ready_to_run) {
            load_bootstrap(bootstrap_path);

            if (console == ConsoleType::DMG) {
                bus.KEY0 = DISABLE_CGB_FUNCTIONS;
                ppu.write_register(0x6C, 1);

                ppu.set_compatibility_palette(PaletteID::BG, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ1, LCD_GRAY);
                ppu.set_compatibility_palette(PaletteID::OBJ2, LCD_GRAY);
            }

            cpu.reset(0x0);
        }
    }

    void Core::run_for_frames(int32_t frames) {
        while (frames-- && ready_to_run) {
            while (cycle_count < CYCLES_PER_FRAME && !cpu.stopped()) {
                dma.tick();
                cpu.step();
            }

            if (cycle_count >= CYCLES_PER_FRAME) {
                cycle_count -= CYCLES_PER_FRAME;
            }
        }
    }

    void Core::tick_subcomponents(int32_t cycles) {
        int32_t adjusted_cycles = cpu.double_speed() ? 2 : 4;

        while (cycles > 0) {
            timer.update(4);
            ppu.step(adjusted_cycles);
            apu.step(adjusted_cycles);
            bus.cart->tick(adjusted_cycles);
            cycle_count += adjusted_cycles;
            cycles -= 4;
        }
    }

    void Core::load_bootstrap(std::filesystem::path path) {
        std::ifstream rom(path, std::ios::binary | std::ios::ate);

        if (rom) {
            auto len = rom.tellg();

            if (len == 0) {
                return;
            }

            bootstrap.clear();
            bootstrap.resize(len);

            rom.seekg(0);
            rom.read(reinterpret_cast<char *>(bootstrap.data()), len);
            rom.close();
        }
    }

    uint8_t Core::read_bootstrap(uint16_t address) { return bootstrap[address]; }
}