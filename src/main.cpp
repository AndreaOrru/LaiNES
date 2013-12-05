#include "cartridge.hpp"
#include "cpu.hpp"
#include "io.hpp"
#include "ppu.hpp"

int main(int argc, char *argv[])
{
    IO::init();
    Cartridge::load(argv[1]);
    CPU::power();

    CPU::run();

    return 0;
}
