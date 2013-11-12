#include "ppu.hpp"

namespace PPU {


void step()
{
    if (y < 240)
        switch (x % 8)
        {
            case 1:
                addr = 0x2000 | (v & 0x0FFF);
                cNameTable = (cNameTable << 8)
                break;
            case 2:
                nameTable = rd(addr);
                break;
            case 3:
                addr = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07);
                break;
            case 4:
                attribute = rd(addr);
                break;
            case 5:
                addr  = (0x1000 * bgPos) + (nametable << 4) + (v.fineY);
                break;
            case 6:
                tileData.l = rd(addr);
                break;
            case 7:
                addr++;
                break;
            case 0:
                tileData.h = rd(addr);
                
                break;
        }
}


}
