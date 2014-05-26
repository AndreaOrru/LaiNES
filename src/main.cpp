#include "cartridge.hpp"
#include "cpu.hpp"
#include "gui.hpp"
#include "ppu.hpp"


int main(int argc, char *argv[])
{
    GUI::init();
    Cartridge::load(argv[1]);
    CPU::power();

    GUI::run();

    return 0;
}
