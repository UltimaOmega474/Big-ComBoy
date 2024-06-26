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

#include "Bus.hpp"
#include "Core.hpp"
#include <stdexcept>

namespace GB {
    MainBus::MainBus(Core *core) : core(core) {
        if (!core) {
            throw std::invalid_argument("Core cannot be null.");
        }
    }

    bool MainBus::bootstrap_mapped() const { return bootstrap_mapped_; }

    bool MainBus::is_compatibility_mode() const { return (KEY0 & DISABLE_CGB_FUNCTIONS); }

    void MainBus::reset(Cartridge *new_cart) {
        KEY0 = 0;
        bootstrap_mapped_ = true;
        wram.fill(0);
        hram.fill(0);
        cart = new_cart;
    }

    uint8_t MainBus::read(uint16_t address) {
        auto page = address >> 12;

        switch (page) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: {
            if (bootstrap_mapped_) {
                if ((address < 0x100) || (address > 0x1FF)) {
                    return core->read_bootstrap(address);
                } else if (cart) {
                    return cart->read(address);
                }
            } else {
                if (cart) {
                    return cart->read(address);
                }
            }
            return 0xFF;
        }

        case 0x8:
        case 0x9: {
            return core->ppu.read_vram(address & 0x1FFF);
        }
        case 0xA:
        case 0xB: {
            if (cart) {
                return cart->read_ram(address & 0x1FFF);
            }
            return 0xFF;
        }
        case 0xC: {
            return wram[address & (address & 0xFFF)];
        }
        case 0xD: {
            return wram[(wram_bank_num * 0x1000) + (address & 0xFFF)];
        }
        case 0xE: {
            return wram[address & (address & 0xFFF)];
        }
        case 0xF: {
            auto hram_page = address >> 8;

            switch (hram_page) {
            case 0xFD: {
                return wram[address & 0x1FFF];
            }
            case 0xFE: {
                return core->ppu.read_oam(address & 0xFF);
            }
            case 0xFF: {
                auto io_address = address & 0xFF;

                switch (io_address) {
                // Input
                case 0x00: {
                    return core->pad.get_pad_state();
                }

                // Serial Port
                case 0x01:
                case 0x02: {
                    return 0xFF;
                }

                // Timer
                case 0x04:
                case 0x05:
                case 0x06:
                case 0x07: {
                    return core->timer.read_register(io_address);
                }

                case 0x0F: {
                    return core->cpu.interrupt_flag;
                }

                // APU
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x18:
                case 0x19:
                case 0x1A:
                case 0x1B:
                case 0x1C:
                case 0x1D:
                case 0x1E:
                case 0x1F:
                case 0x20:
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x25:
                case 0x26:
                case 0x27: {
                    return core->apu.read_register(io_address);
                }

                // APU Wave RAM
                case 0x30:
                case 0x31:
                case 0x32:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x36:
                case 0x37:
                case 0x38:
                case 0x39:
                case 0x3A:
                case 0x3B:
                case 0x3C:
                case 0x3D:
                case 0x3E:
                case 0x3F: {
                    return core->apu.read_wave_ram(io_address - 0x30);
                }

                // PPU Registers
                case 0x40:
                case 0x41:
                case 0x42:
                case 0x43:
                case 0x44:
                case 0x45:
                case 0x46:
                case 0x47:
                case 0x48:
                case 0x49:
                case 0x4A:
                case 0x4B:
                case 0x4F:
                case 0x68:
                case 0x69:
                case 0x6A:
                case 0x6B:
                case 0x6C: {
                    return core->ppu.read_register(io_address);
                }

                case 0x4C: {
                    return KEY0;
                }
                case 0x4D: {
                    return is_compatibility_mode() ? 0xFF : core->cpu.KEY1;
                }
                case 0x50: {
                    return bootstrap_mapped_;
                }
                case 0x55: {
                    return core->dma.get_dma_status();
                }
                case 0x70: {
                    return wram_bank_num;
                }
                }

                if ((address >= 0xFF80) && (address <= 0xFFFE)) {
                    return hram[address - 0xFF80]; // High Ram
                } else if (address == 0xFFFF) {
                    return core->cpu.interrupt_enable;
                }

                return 0xFF; // IO Registers
            }
            }

            return 0;
        }
        }

        return 0;
    }

    void MainBus::write(uint16_t address, uint8_t value) {
        auto page = address >> 12;

        switch (page) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: {
            if (bootstrap_mapped_) {
                if ((address < 0x100) || (address > 0x1FF)) {
                    return;
                } else if (cart) {
                    cart->write(address, value);
                }
            } else {
                if (cart) {
                    cart->write(address, value);
                }
            }

            return;
        }

        case 0x8:
        case 0x9: {
            core->ppu.write_vram(address & 0x1FFF, value);
            return;
        }

        case 0xA:
        case 0xB: {
            if (cart) {
                cart->write_ram(address & 0x1FFF, value);
            }
            return;
        }

        case 0xC: {
            wram[address & (address & 0xFFF)] = value;
            return;
        }
        case 0xD: {
            wram[(wram_bank_num * 0x1000) + (address & 0xFFF)] = value;
            return;
        }
        case 0xE: {
            wram[address & (address & 0xFFF)] = value;
            return;
        }

        case 0xF: {
            auto hram_page = address >> 8;

            switch (hram_page) {
            case 0xFD: {
                wram[address & 0x1FFF] = value;
                return;
            }
            case 0xFE: {
                core->ppu.write_oam(address & 0xFF, value);
                return;
            }
            case 0xFF: {
                auto io_address = address & 0xFF;

                switch (io_address) {
                // Input
                case 0x00: {
                    core->pad.select_button_mode(value);
                    return;
                }

                // Timer
                case 0x04:
                case 0x05:
                case 0x06:
                case 0x07: {
                    core->timer.write_register(io_address, value);
                    return;
                }

                case 0x0F: {
                    core->cpu.interrupt_flag = value;
                    return;
                }

                // APU
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                case 0x18:
                case 0x19:
                case 0x1A:
                case 0x1B:
                case 0x1C:
                case 0x1D:
                case 0x1E:
                case 0x1F:
                case 0x20:
                case 0x21:
                case 0x22:
                case 0x23:
                case 0x24:
                case 0x25:
                case 0x26:
                case 0x27: {
                    core->apu.write_register(io_address, value);
                    return;
                }

                // APU Wave RAM
                case 0x30:
                case 0x31:
                case 0x32:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x36:
                case 0x37:
                case 0x38:
                case 0x39:
                case 0x3A:
                case 0x3B:
                case 0x3C:
                case 0x3D:
                case 0x3E:
                case 0x3F: {
                    core->apu.write_wave_ram(io_address - 0x30, value);
                    return;
                }

                // PPU Registers
                case 0x40:
                case 0x41:
                case 0x42:
                case 0x43:
                case 0x45:
                case 0x46:
                case 0x47:
                case 0x48:
                case 0x49:
                case 0x4A:
                case 0x4B:
                case 0x4F:
                case 0x68:
                case 0x69:
                case 0x6A:
                case 0x6B:
                case 0x6C: {
                    core->ppu.write_register(io_address, value);
                    return;
                }

                case 0x4C: {
                    if (bootstrap_mapped_) {
                        KEY0 = value;
                    }
                    return;
                }

                case 0x4D: {
                    core->cpu.KEY1 &= ~0x1;
                    core->cpu.KEY1 |= value & 0x1;
                    return;
                }

                case 0x50: {
                    bootstrap_mapped_ = false;
                    return;
                }
                case 0x51: {
                    core->dma.set_hdma1(value);
                    return;
                }
                case 0x52: {
                    core->dma.set_hdma2(value);
                    return;
                }
                case 0x53: {
                    core->dma.set_hdma3(value);
                    return;
                }
                case 0x54: {
                    core->dma.set_hdma4(value);
                    return;
                }
                case 0x55: {
                    core->dma.set_dma_control(value);
                    return;
                }

                case 0x70: {
                    value &= 0x7;
                    wram_bank_num = value ? value : 1;
                    return;
                }
                }

                if ((address >= 0xFF80) && (address <= 0xFFFE)) {
                    hram[address - 0xFF80] = value; // High Ram
                    return;
                } else if (address == 0xFFFF) {
                    core->cpu.interrupt_enable = value;
                    return;
                }

                return;
            }
            }
        }
        }
    }
}