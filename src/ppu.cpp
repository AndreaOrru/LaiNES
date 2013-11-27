#include "cartridge.hpp"
#include "ppu.hpp"

namespace PPU {


u8 ciRam[0x800];
u8 palette[0x20];
u8 oam[0x100];

Addr vAddr, tAddr;
u8 fX;
u8 oamAddr;
Ctrl ctrl;
Mask mask;
Status status;

static u8 rd(u16 addr)
{
    if      (addr < 0x2000) return Cartridge::chr_access<0>(addr);
    else if (addr < 0x2800) return ciRam[addr - 0x2000];
    else if (addr < 0x3000) return 0x00;
    else if (addr < 0x3F00) return ciRam[addr - 0x3000];
    else                    return palette[addr % 0x20];
}

static u8 wr(u16 addr, u8 v)
{
    if      (addr < 0x2000) return Cartridge::chr_access<1>(addr, v);
    else if (addr < 0x2800) return ciRam[addr - 0x2000] = v;
    else if (addr < 0x3000) return 0x00;
    else if (addr < 0x3F00) return ciRam[addr - 0x3000] = v;
    else                    return palette[addr % 0x20] = v;
}

template <bool write> u8 access(u16 index, u8 v)
{
    static u8 res, buffer;
    static bool latch;

    if (write)
    {
        res = v;

        switch (index)
        {
            case 0:  ctrl.r = v; tAddr.nt = ctrl.nt; break;
            case 1:  mask.r = v; break;
            case 3:  oamAddr = v; break;
            case 4:  oam[oamAddr++] = v; break;
            case 5:
                if (!latch) { fX = v & 7; tAddr.cX = v >> 3;   }
                else  { tAddr.fY = v & 7; tAddr.cY = v & 0x1F; }
                latch = !latch;
            case 6:
                if (!latch) { tAddr.h = v & 0x3F; }
                else        { tAddr.l = v; vAddr.r = tAddr.r; }
                latch = !latch;
            case 7:  wr(vAddr.addr, v); vAddr.addr += ctrl.incr; break;
        }
    }

    else
        switch (index)
        {
            case 2:  res = (res & 0x1F) | status.r; status.vBlank = 0; latch = 0; break;
            case 4:  res = oam[oamAddr]; break;
            case 7:
                if (vAddr.addr <= 0x3EFF)
                {
                    res = buffer;
                    buffer = rd(vAddr.addr);
                }
                else
                    res = buffer = rd(vAddr.addr);
                vAddr.addr += ctrl.incr;
                break;
        }

    return res;
}
template u8 access<0>(u16, u8); template u8 access<1>(u16, u8);


}
