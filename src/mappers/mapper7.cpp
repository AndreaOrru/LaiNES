#include "ppu.hpp"
#include "mappers/mapper7.hpp"

/* Based off of https://wiki.nesdev.com/w/index.php/AxROM */

/* Apply the registers state */
void Mapper7::apply()
{
   /*
    * 32 kb PRG ROM Banks
    * 0x8000 - 0xFFFF swappable
    */
    map_prg<32>(0, regs[0] & 0b00001111);

    /* 8k of CHR (ram) */
    map_chr<8>(0, 0);

    /* Mirroring based on bit 5 */
    set_mirroring((regs[0] & 0b00010000) ? PPU::ONE_SCREEN_HI : PPU::ONE_SCREEN_LO);
}

u8 Mapper7::write(u16 addr, u8 v)
{
    /* check for bus contingency? (addr & 0x8000 == v?)
     * Seems not neccesary */

    /* bank switching */
    if (addr & 0x8000)
    {
        regs[0] = v;
        apply();
    }
    return v;
}

u8 Mapper7::chr_write(u16 addr, u8 v)
{
    return chr[addr] = v;
}
