#include "ppu.hpp"
#include "mappers/mapper2.hpp"

/* Based off of https://wiki.nesdev.com/w/index.php/UxROM */

/* Apply the registers state */
void Mapper2::apply()
{
   /*
    * 16 kb PRG ROM Banks
    * 0x8000 - 0xBFFF swappable
    * 0xC000 - 0xFFFF fixed
    */
    map_prg<16>(0, regs[0] & 0xF);
    map_prg<16>(1,           0xF);

    /* 8k of CHR */
    map_chr<8>(0, 0);

    /* mirroring is based on the header (soldered) */
    set_mirroring(vertical_mirroring?PPU::VERTICAL:PPU::HORIZONTAL);
}

u8 Mapper2::write(u16 addr, u8 v)
{
    /* check for bus contingency? (addr & 0x8000 == v?) nah */

    /* bank switching */
    if (addr & 0x8000)
    {
        regs[0] = v;
        apply();
    }
    return v;
}

u8 Mapper2::chr_write(u16 addr, u8 v)
{
    return chr[addr] = v;
}
