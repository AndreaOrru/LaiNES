#include "ppu.hpp"

namespace PPU {


int scanline;
int cycle;

void step()
{
    switch (cycle++)
    {
        TILE_PHASE_1:  addr = ntAddr();       break;
        TILE_PHASE_2:  tileNum = rd(addr);    break;
        TILE_PHASE_3:  addr = attrAddr();     break;
        TILE_PHASE_4:  attr = rd(addr);       break;
        TILE_PHASE_5:  addr = dataAddr();     break;
        TILE_PHASE_6:  tileData.l = rd(addr); break;
        TILE_PHASE_7:  addr += 8;             break;
        TILE_PHASE_8:  tileData.h = rd(addr); h_scroll(); break;

        case 256:  tileData.h = rd(addr); h_scroll(); v_scroll(); break;
        case 257:  h_update(); break;

        SPRITE_PHASE_5:  addr = sprAddr(); break;
        SPRITE_PHASE_6:  sprData.l = rd(addr); break;
        SPRITE_PHASE_7:  addr += 8; break;
        SPRITE_PHASE_8:  sprData.h = rd(addr); break;
    }
    render_pixel();
}


}
