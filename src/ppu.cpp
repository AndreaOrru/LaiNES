#include "cartridge.hpp"
#include "cpu.hpp"
#include "io.hpp"
#include "ppu.hpp"

namespace PPU {
#include "palette.inc"


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
            case 0:  ctrl.r = v; tAddr.nt = ctrl.nt; break;       // PPUCTRL   ($2000).
            case 1:  mask.r = v; break;                           // PPUMASK   ($2001).
            case 3:  oamAddr = v; break;                          // OAMADDR   ($2003).
            case 4:  oam[oamAddr++] = v; break;                   // OAMDATA   ($2004).
            case 5:                                               // PPUSCROLL ($2005).
                if (!latch) { fX = v & 7; tAddr.cX = v >> 3; }      // First write.
                else  { tAddr.fY = v & 7; tAddr.cY = v >> 3; }      // Second write.
                latch = !latch; break;
            case 6:                                               // PPUADDR   ($2006).
                if (!latch) { tAddr.h = v & 0x3F; }                 // First write.
                else        { tAddr.l = v; vAddr.r = tAddr.r; }     // Second write.
                latch = !latch; break;
            case 7:  wr(vAddr.addr, v); vAddr.addr += ctrl.incr ? 32 : 1;  // PPUDATA ($2007).
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
/* Put new data into the shift registers */
inline void reload_shift()
{
    bgShiftL = (bgShiftL & 0xFF00) | bgL;
    bgShiftH = (bgShiftH & 0xFF00) | bgH;
    if (!(vAddr.cX % 2))  // Attributes change every 16 pixel.
    {                     // FIXME: I think this breaks fineX scrolling.
        atShiftL = (atShiftL << 1) |  (at & 1);
        atShiftH = (atShiftH << 1) | ((at & 2) >> 1);
    }
}

/* Process a pixel, draw it if it's on screen */
void pixel()
{
    u8 bgBits = (NTH_BIT(bgShiftH, 15 - fX) << 1) |
                 NTH_BIT(bgShiftL, 15 - fX);
    u8 atBits = ((atShiftH & 1) << 1) |
                 (atShiftL & 1);
    u8 palInd = rendering() ? ((atBits << 2) | bgBits) : 0;

    if (scanline < 240 and cycle >= 1 and cycle <= 256)
        IO::draw_pixel(cycle - 1, scanline, nes_rgb[rd(0x3F00 | palInd)]);

    bgShiftL <<= 1; bgShiftH <<= 1;
}

/* Execute a cycle of a scanline */
template<Scanline s> void scanline_cycle()
{
    static u16 addr;

    if (s == NMI and cycle == 1) { status.vBlank = true; if (ctrl.nmi) CPU::set_nmi(); }
    else if (s == POST and cycle == 0) IO::flush_screen();
    else if (s == VISIBLE or s == PRE)
        switch (cycle)
        {
            case 2 ... 255: case 322 ... 337:
                switch (cycle % 8)
                {
                    // Nametable:
                    case 1:  addr  = nt_addr(); reload_shift(); break;
                    case 2:  nt    = rd(addr);  break;
                    // Attribute:
                    case 3:  addr  = at_addr(); break;
                    case 4:  at    = rd(addr);  if (vAddr.cY & 2) at >>= 4;
                                                if (vAddr.cX & 2) at >>= 2; break;
                    // Background (low bits):
                    case 5:  addr  = bg_addr(); break;
                    case 6:  bgL   = rd(addr);  break;
                    // Background (high bits):
                    case 7:  addr += 8;         break;
                    case 0:  bgH   = rd(addr); h_scroll();
                }
                pixel(); break;
            case         256:  bgH = rd(addr); v_scroll(); pixel(); break;  // Vertical bump.
            case         257:  reload_shift(); h_update(); break;  // Update horizontal position.
            case 280 ... 304:  if (s == PRE)   v_update(); break;  // Update vertical position.

            // No shift reloading:
            case             1:  addr = nt_addr(); if (s == PRE) status.vBlank = false; break;
            case 321: case 339:  addr = nt_addr(); break;
            // Nametable fetch instead of attribute:
            case           338:  nt = rd(addr); break;
            case           340:  nt = rd(addr); if (s == PRE && rendering() && frameOdd) cycle++;
        }
}

/* Execute a PPU cycle. */
void step()
{
    switch (scanline)
    {
        case 0 ... 239:  scanline_cycle<VISIBLE>(); break;
        case       240:  scanline_cycle<POST>();    break;
        case       241:  scanline_cycle<NMI>();     break;
        case       261:  scanline_cycle<PRE>();     break;
    }
    // Update cycle and scanline counters:
    if (++cycle > 340)
    {
        cycle %= 341;
        if (++scanline > 261)
        {
            scanline = 0;
            frameOdd ^= 1;
        }
    }
}


}
