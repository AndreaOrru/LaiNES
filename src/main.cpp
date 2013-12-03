#include "cartridge.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

int main(int argc, char *argv[])
{
    Cartridge::load(argv[1]);
    PPU::init();
    CPU::power();

    CPU::run();

    return 0;
}
