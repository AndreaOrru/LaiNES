#include "ppu.hpp"

namespace PPU {


void step()
{
    if (y < 240)
    {
        if (cyc > 0 && cyc <= 256)
        {
            switch (cyc % 8)
            {
                case 1: addr = ntAddr();   break;  case 2: tileNum = rd(addr);         break;
                case 3: addr = attrAddr(); break;  case 4: attr = rd(addr);            break;
                case 5: addr = dataAddr(); break;  case 6: tileData.l = rd(addr);      break;
                case 7: addr += 8;         break;  case 0: tileData.h = rd(addr); v++; break;
            }
            render_pixel();
        }
            
        cyc++;
}


}
