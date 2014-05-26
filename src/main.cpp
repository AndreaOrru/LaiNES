#include "cartridge.hpp"
#include "cpu.hpp"
#include "gui.hpp"
#include "ppu.hpp"


int main(int argc, char *argv[])
{
    GUI::init();
    Cartridge::load(argv[1]);
    CPU::power();

    while(true)
        CPU::run_frame();

    return 0;
}
