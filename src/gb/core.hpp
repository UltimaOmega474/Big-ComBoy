#pragma once
#include "sm83.hpp"
#include "cartridge.hpp"
#include "timer.hpp"
#include "ppu.hpp"
#include "pad.hpp"
#include "apu.hpp"

#include <memory>
#include <ostream>
namespace GameBoy
{

	struct CoreSettings
	{
		std::string boot_rom_path;
		bool skip_boot_rom;
	};

	class Core
	{
		bool boot_rom_enabled = true;
		uint32_t cycle_count = 0;

	public:
		SM83 cpu;
		Timer timer;
		PPU ppu;
		APU apu{};
		Gamepad pad{};
		CoreSettings settings{};
		std::array<uint8_t, 8192> wram{}; // c000h - cfffh
		std::array<uint8_t, 127> hram{};  // ff80h - fffeh
		std::array<uint8_t, 256> boot_rom{};

		std::unique_ptr<Cartridge> cart{};

		Core();

		void start(std::unique_ptr<Cartridge> cart);

		void run_for_frames(uint32_t frames);
		void run_for_cycles(uint32_t cycles, std::ostream &log_stream);
		void tick_subcomponents(uint8_t cycles);
		void request_interrupt(uint8_t interrupt);
		void load_boot_rom_from_file();

		// bus functions
		uint8_t read_no_tick(uint16_t address);
		void write_no_tick(uint16_t address, uint8_t value);

		uint8_t read(uint16_t address);
		void write(uint16_t address, uint8_t value);
		uint16_t read_uint16(uint16_t address);
		void write_uint16(uint16_t address, uint16_t value);
	};

	constexpr bool within_range(uint16_t address, uint16_t start, uint16_t end)
	{
		return ((address >= start) && (address <= end));
	}

}