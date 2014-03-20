#include "cpu.hpp"
#include "ppu.hpp"
#include "mappers/mapper4.hpp"


void Mapper4::apply()
{
    map_prg<8>(1, regs[7]);

    // PRG Mode 0:
    if (!(reg8000 & (1 << 6)))
    {
        map_prg<8>(0, regs[6]);
        map_prg<8>(2, -2);
    }
    // PRG Mode 1:
    else
    {
        map_prg<8>(0, -2);
        map_prg<8>(2, regs[6]);
    }

    // CHR Mode 0:
    if (!(reg8000 & (1 << 7)))
    {
        map_chr<2>(0, regs[0] >> 1);
        map_chr<2>(1, regs[1] >> 1);
        for (int i = 0; i < 4; i++)
            map_chr<1>(4 + i, regs[2 + i]);
    }
    // CHR Mode 1:
    else
    {
        for (int i = 0; i < 4; i++)
            map_chr<1>(i, regs[2 + i]);
        map_chr<2>(2, regs[0] >> 1);
        map_chr<2>(3, regs[1] >> 1);
    }

    set_mirroring(horizMirroring ? PPU::HORIZONTAL : PPU::VERTICAL);
}

u8 Mapper4::write(u16 addr, u8 v)
{
    if (addr < 0x8000)
        prgRam[addr - 0x6000] = v;
    else if (addr & 0x8000)
    {
        switch (addr & 0xE001)
        {
            case 0x8000:  reg8000 = v;                      break;
            case 0x8001:  regs[reg8000 & 0b111] = v;        break;
            case 0xA000:  horizMirroring = v & 1;           break;
            case 0xC000:  irqPeriod = v;                    break;
            case 0xC001:  irqCounter = 0;                   break;
            case 0xE000:  CPU::set_irq(irqEnabled = false); break;
            case 0xE001:  irqEnabled = true;                break;
        }
        apply();
    }
    return v;
}

u8 Mapper4::chr_write(u16 addr, u8 v)
{
    return chr[addr] = v;
}

void Mapper4::signal_scanline()
{
    if (irqCounter == 0)
        irqCounter = irqPeriod;
    else
        irqCounter--;

    if (irqEnabled and irqCounter == 0)
        CPU::set_irq();
}
