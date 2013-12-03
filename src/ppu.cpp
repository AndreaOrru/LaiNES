#include <csignal>
#include <SDL/SDL.h>
#include "cartridge.hpp"
#include "cpu.hpp"
#include "ppu_case.hpp"
#include "ppu.hpp"

namespace PPU {


SDL_Surface* s;
u8 ciRam[0x800];    // VRAM for nametables.
u8 palette[0x20];   // VRAM for palettes.
u8 oam[0x100];      // VRAM for sprites.

Addr vAddr, tAddr;  // Loopy V, T.
u8 fX;              // Fine X.
u8 oamAddr;         // OAM address.

Ctrl ctrl;          // PPUCTRL   ($2000) register.
Mask mask;          // PPUMASK   ($2001) register.
Status status;      // PPUSTATUS ($2002) register.

// Background latches:
u8 nt, at, bgL, bgH;
// Background shift registers:
u8 atShiftL, atShiftH; u16 bgShiftL, bgShiftH;

// Rendering counters:
int scanline, cycle;
bool frameOdd;

u32 nes_rgb[] =
{ 0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400,
  0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
  0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10,
  0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
  0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044,
  0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
  0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
  0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000 };

inline bool rendering() { return mask.bg || mask.spr; }

/* Access PPU memory */
template <bool wr> u8 mem_access(u16 addr, u8 v = 0)
{
    u8* ref;

    if      (addr < 0x2000) return Cartridge::chr_access<wr>(addr, v);     // CHR-ROM/RAM.
    else if (addr < 0x2800) ref = &ciRam[addr - 0x2000];                   // Nametables.
    else if (addr < 0x3000) return 0x00;
    else if (addr < 0x3F00) ref = &ciRam[addr - 0x3000];                   // Nametables (mirror).
    else                    ref = &palette[(addr % 4) ? addr % 0x20 : 0];  // Palettes.

    if (wr) return *ref = v;
    else    return *ref;
}
inline u8 rd(u16 addr)       { return mem_access<0>(addr); }
inline u8 wr(u16 addr, u8 v) { return mem_access<1>(addr, v); }

/* Access PPU through registers. */
template <bool write> u8 access(u16 index, u8 v)
{
    static u8 res;      // Result of the operation.
    static u8 buffer;   // VRAM read buffer.
    static bool latch;  // Detect second reading.

    /* Write into register */
    if (write)
    {
        res = v;

        switch (index)
        {
            case 0:  ctrl.r = v; tAddr.nt = ctrl.nt; break;                // PPUCTRL   ($2000).
            case 1:  mask.r = v; break;                                    // PPUMASK   ($2001).
            case 3:  oamAddr = v; break;                                   // OAMADDR   ($2003).
            case 4:  oam[oamAddr++] = v; break;                            // OAMDATA   ($2004).
            case 5:                                                        // PPUSCROLL ($2005).
                if (!latch) { fX = v & 7; tAddr.cX = v >> 3; }               // First write.
                else  { tAddr.fY = v & 7; tAddr.cY = v >> 3; }               // Second write.
                latch = !latch; break;
            case 6:                                                        // PPUADDR   ($2006).
                if (!latch) { tAddr.h = v & 0x3F; }                           // First write.
                else        { tAddr.l = v; vAddr.r = tAddr.r; }               // Second write.
                latch = !latch; break;
            case 7:  wr(vAddr.addr, v); vAddr.addr += ctrl.incr ? 32 : 1;  // PPUDATA   ($2007).
        }
    }
    /* Read from register */
    else
        switch (index)
        {
            // PPUSTATUS ($2002):
            case 2:  res = (res & 0x1F) | status.r; status.vBlank = 0; latch = 0; break;
            case 4:  res = oam[oamAddr]; break;  // OAMDATA ($2004).
            case 7:                              // PPUDATA ($2007).
                if (vAddr.addr <= 0x3EFF)
                {
                    res = buffer;
                    buffer = rd(vAddr.addr);
                }
                else
                    res = buffer = rd(vAddr.addr);
                vAddr.addr += ctrl.incr ? 32 : 1;
        }
    return res;
}
template u8 access<0>(u16, u8); template u8 access<1>(u16, u8);

/* Calculate graphics addresses */
inline u16 nt_addr() { return 0x2000 | (vAddr.r & 0xFFF); }
inline u16 at_addr() { return 0x23C0 | (vAddr.nt << 10) | ((vAddr.cY / 4) << 3) | (vAddr.cX / 4); }
inline u16 bg_addr() { return (ctrl.bgTbl * 0x1000) + (nt * 16) + vAddr.fY; }
/* Increment the scroll by one pixel */
inline void h_scroll() { if (!rendering()) return; if (vAddr.cX == 31) vAddr.r ^= 0x41F; else vAddr.cX++; }
inline void v_scroll()
{
    if (!rendering()) return;
    if (vAddr.fY < 7) vAddr.fY++;
    else
    {
        vAddr.fY = 0;
        if      (vAddr.cY == 31)   vAddr.cY = 0;
        else if (vAddr.cY == 29) { vAddr.cY = 0; vAddr.nt ^= 0b10; }
        else                       vAddr.cY++;
    }
}
/* Copy scrolling data from loopy T to loopy V */
inline void h_update() { if (!rendering()) return; vAddr.r = (vAddr.r & ~0x041F) | (tAddr.r & 0x041F); }
inline void v_update() { if (!rendering()) return; vAddr.r = (vAddr.r & ~0x7BE0) | (tAddr.r & 0x7BE0); }
// Put new data into the shift registers:
inline void reload_shift()
{
    bgShiftL = (bgShiftL & 0xFF00) | bgL;
    bgShiftH = (bgShiftH & 0xFF00) | bgH;
    if (!(vAddr.cX % 2))
    {
        atShiftL = (atShiftL << 1) |  (at & 1);
        atShiftH = (atShiftH << 1) | ((at & 2) >> 1);
    }
}

void pixel()
{
    u8 bgBits = (NTH_BIT(bgShiftH, 15 - fX) << 1) |
                 NTH_BIT(bgShiftL, 15 - fX);
    u8 atBits = ((atShiftH & 1) << 1) |
                 (atShiftL & 1);
    u8 palInd = (atBits << 2) | bgBits;

    if (scanline < 240 and cycle >= 1 and cycle <= 256)
        ((u32*) s->pixels) [scanline * 256 + (cycle - 1)] = rendering() ? nes_rgb[rd(0x3F00 | palInd)] : 0;

    bgShiftL <<= 1; bgShiftH <<= 1;
}

/* Execute a cycle of a scanline (prerender or visible) */
template <bool pre> void scanline_()
{
    static u16 addr;

    switch (cycle)
    {
        // Nametable fetching:
        TILE_PHASE_1:  if (pre && cycle == 1) status.vBlank = false;  // Disable VBlank in prerender.
                       addr = nt_addr(); pixel(); break;
        TILE_PHASE_2:  nt   = rd(addr);  pixel(); break;
        // Attribute fetching:
        TILE_PHASE_3:  addr = at_addr(); pixel(); break;
        TILE_PHASE_4:  at   = rd(addr);  if (vAddr.cY & 2) at >>= 4;
                                         if (vAddr.cX & 2) at >>= 2;
                                         pixel(); break;
        // Background fetching (low bits):
        TILE_PHASE_5:  addr = bg_addr(); pixel(); break;
        TILE_PHASE_6:  bgL  = rd(addr);  pixel(); break;
        // Attribute fetching (high bits):
        TILE_PHASE_7:  addr += 8;        pixel(); break;
        TILE_PHASE_8:  bgH  = rd(addr); h_scroll(); pixel(); reload_shift(); break;

        case 256:      bgH  = rd(addr); h_scroll(); v_scroll(); pixel(); reload_shift(); break;  // End of line.
        case 257:      h_update(); break;  // Update horizontal scrolling info.

        case 280 ... 304:  // Vertical scrolling update in prerender scanline.
            if (pre) v_update();
    }
}
inline void scanline_visible()   { scanline_<0>(); }
inline void scanline_prerender() { scanline_<1>(); }

/* Execute a PPU cycle. */
void step()
{
    switch (scanline)
    {
        case 0 ... 239:  scanline_visible(); break;  // Visible scanline.
        case 240:
            if (cycle == 0) SDL_Flip(s); break;

        case 241:  // Signal VBlank.
            if (cycle == 1) { status.vBlank = true; if (ctrl.nmi) CPU::set_nmi(); }
            break;

        case 261:  // Pre-render scanline.
            scanline_prerender();
            if (cycle == 340)
            {
                // Restart from the first pixel on the screen:
                cycle = (rendering() && frameOdd) ? 1 : 0;   // Skip one cicle on odd frames.
                scanline = 0; frameOdd ^= 1;
                return;  // Don't update scanline and cycle.
            }
    }
    // Update current cycle and scanline:
    cycle = (cycle + 1) % 341;
    scanline = cycle ? scanline : (scanline + 1);
}

void power()
{
    SDL_Init(SDL_INIT_VIDEO);
    s = SDL_SetVideoMode(256, 240, 32, 0);
    signal(SIGINT, SIG_DFL);
}


}
